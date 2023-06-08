
// #define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

extern SEXP record_audio_(SEXP seconds_);
extern SEXP whisper_init_(SEXP path_);
extern SEXP whisper_(SEXP ctx_, SEXP snd_, SEXP params_);

static const R_CallMethodDef CEntries[] = {
  
  {"record_audio_"   , (DL_FUNC) &record_audio_   , 1},
  {"whisper_init_"   , (DL_FUNC) &whisper_init_   , 2},
  {"whisper_"        , (DL_FUNC) &whisper_        , 3},
  {NULL , NULL, 0}
};


void R_init_carelesswhisper(DllInfo *info) {
  R_registerRoutines(
    info,      // DllInfo
    NULL,      // .C
    CEntries,  // .Call
    NULL,      // Fortran
    NULL       // External
  );
  R_useDynamicSymbols(info, FALSE);
}



