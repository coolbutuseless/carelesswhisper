


#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "whisper.h"
#include "data.frame.h"


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Unpack an external pointer to a C 'whisper_context *'
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct whisper_context * external_ptr_to_whisper_context(SEXP ctx_) {
  if (!inherits(ctx_, "whisper_context")) error("Expecting 'ptr' to be an 'whisper_context' ExternalPtr");
  
  struct whisper_context *ctx = TYPEOF(ctx_) != EXTPTRSXP ? NULL : (struct whisper_context *)R_ExternalPtrAddr(ctx_);
  if (ctx == NULL) {
    error("whisper_context pointer is invalid/NULL.");
  }
      
  return ctx;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Finalizer for a 'whisper_context' object.
//
// This function will be called when whisper object gets 
// garbage collected.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void whisper_context_finalizer(SEXP ctx_) {

  // Rprintf("whisper_context_finalizer !!!!\n");
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Unpack the pointer 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  struct whisper_context *ctx = (struct whisper_context *) R_ExternalPtrAddr(ctx_);
  if (ctx == 0) {
    Rprintf("NULL whisper_context in finalizer");
    return;
  }
  
  whisper_free(ctx);
  R_ClearExternalPtr(ctx_);
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Parse the model file and create the whisper context
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP whisper_init_(SEXP path_, SEXP verbose_) {
  
  const char *path = CHAR(STRING_ELT(path_, 0));

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Init whisper context
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  struct whisper_context * ctx = whisper_init_from_file(path, asLogical(verbose_));
  
  if (ctx == 0) {
    error("Failed to create whisper context");
  }
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP ctx_ = R_MakeExternalPtr(ctx, R_NilValue, R_NilValue);
  PROTECT(ctx_);
  R_RegisterCFinalizer(ctx_, whisper_context_finalizer);
  Rf_setAttrib(ctx_, R_ClassSymbol, Rf_mkString("whisper_context"));
  UNPROTECT(1);
  
  return ctx_;
}





//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Main whisper routine
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP whisper_(SEXP ctx_, SEXP snd_, SEXP params_, SEXP details_) {
  
  unsigned int nprotect = 0;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Unpack the pointer to the whisper context
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  struct whisper_context *ctx = external_ptr_to_whisper_context(ctx_);
  if (ctx == 0) {
    error("NULL ctx in whisper_()");
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Convert 'double' to 'float' for whisper.cpp
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  double *snd = REAL(snd_);
  float *fsnd;
  
  fsnd = (float *)malloc(length(snd_) * sizeof(float));
  if (fsnd == NULL) {
    error("Could not allocate memory for 'fsnd'");
  }
  
  for (int i = 0; i < length(snd_); i++) {
    fsnd[i] = snd[i];
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Init whisper params
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  struct whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
  wparams.print_progress             = 0;
  wparams.suppress_blank             = 1;
  wparams.suppress_non_speech_tokens = 1;
  
  wparams.n_threads        = asInteger  (VECTOR_ELT(params_, 0));
  wparams.translate        = asLogical  (VECTOR_ELT(params_, 1));
  wparams.language         = CHAR(asChar(VECTOR_ELT(params_, 2)));
  wparams.max_len          = asInteger  (VECTOR_ELT(params_, 3));
  wparams.detect_language  = asLogical  (VECTOR_ELT(params_, 4));
  wparams.token_timestamps = true;
  
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Process audio
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (whisper_full(ctx, wparams, fsnd, length(snd_)) != 0) {
    free(fsnd);
    error("Whisper failed to process audio\n");
  }
  

  SEXP res;
  const int n_segments = whisper_full_n_segments(ctx);
  
  if (!asLogical(details_)) {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Extract text
    //
    // First figure out how much text we have.
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    unsigned int total_len = 1;
    for (unsigned int i = 0; i < n_segments; ++i) {
      total_len += strlen(whisper_full_get_segment_text(ctx, i));
    }
    
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Allocate just enough memory to hold this much text 
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    char *str;
    str = (char *)calloc(total_len, sizeof(char));
    if (str == NULL) {
      free(fsnd);
      error("Could not allocate %i bytes for 'str' output string", total_len);
    }  
    
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Extract the segment text and copy into final result 
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    for (unsigned int i = 0; i < n_segments; ++i) {
      const char * text = whisper_full_get_segment_text(ctx, i);
      strncat(str, text, strlen(text));
    }
    
    res = PROTECT(mkString(str)); nprotect++;
    free(str);
  } else {
    char *names[8] =  {
      "lang_id",
      "segment_idx",
      "start",
      "end",
      "token_idx", 
      "token_id", 
      "token", 
      "prob"
    };
    int types[8] = {
      INTSXP, INTSXP, INTSXP, INTSXP, INTSXP, INTSXP, STRSXP, REALSXP
    };
    res = PROTECT(df_create(8, names, types));

    for (unsigned int i = 0; i < n_segments; ++i) {
      unsigned int ntokens = whisper_full_n_tokens(ctx, i);
      for (unsigned int j = 0; j < ntokens; j++) {
        df_add_row(
          res, 
          whisper_full_lang_id(ctx),
          i,
          whisper_full_get_segment_t0(ctx, i),
          whisper_full_get_segment_t1(ctx, i),
          j,
          whisper_full_get_token_id(ctx, i, j),
          whisper_full_get_token_text(ctx, i, j),
          whisper_full_get_token_p(ctx, i, j)
        );
      }
    }
    
    df_truncate_to_data_length(res);
  }

  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Ensure we free 'str' after we make an R string from it.
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  free(fsnd);
  UNPROTECT(nprotect);
  
  return res;
}

