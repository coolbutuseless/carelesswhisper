


#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "data.frame.h"


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Create an R data.frame
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP df_create(int ncol, char **names, int *types) {
  
  unsigned int nprotect = 0;
  unsigned int init_length = 8;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Allocate a data.frame with the given number of columns "ncol"
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP df_ = PROTECT(allocVector(VECSXP, ncol)); nprotect++;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // - create vectors
  // - initialise (if needed)
  // -add as columns to the data.frame
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP data_;
  for (int i = 0; i < ncol; i++) {
    switch(types[i]) {
      case REALSXP:
        data_ = PROTECT(allocVector(REALSXP, init_length));
        memset(REAL(data_), 0, init_length * sizeof(double));
        SET_VECTOR_ELT(df_, i, data_);
        UNPROTECT(1);
        break;
      case INTSXP:
        data_ = PROTECT(allocVector(INTSXP, init_length));
        memset(INTEGER(data_), 0, init_length * sizeof(int));
        SET_VECTOR_ELT(df_, i, data_);
        UNPROTECT(1);
        break;
      case LGLSXP:
        data_ = PROTECT(allocVector(LGLSXP, init_length));
        memset(INTEGER(data_), 0, init_length * sizeof(int));
        SET_VECTOR_ELT(df_, i, data_);
        UNPROTECT(1);
        break;
      case STRSXP:
        SET_VECTOR_ELT(df_, i, allocVector(STRSXP , init_length));
        break;
      case VECSXP:
        SET_VECTOR_ELT(df_, i, allocVector(VECSXP, init_length));
        break;
      default:
        error("df_create: Unknown column type: %i", types[i]);
    }
  }

  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Treat the VECSXP as a data.frame by setting the 'class' attribute
  // Also set some classes so that it appears as a tibble.
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP class_ = PROTECT(allocVector(STRSXP, 3)); nprotect++;
  SET_STRING_ELT(class_, 0, mkChar("tbl_df"));
  SET_STRING_ELT(class_, 1, mkChar("tbl"));
  SET_STRING_ELT(class_, 2, mkChar("data.frame"));
  SET_CLASS(df_, class_);
  // SET_CLASS(df_, mkString("data.frame"));
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set the names on the list.
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP names_ = PROTECT(allocVector(STRSXP, ncol)); nprotect++;
  for (int i = 0; i < ncol; i++) {
    SET_STRING_ELT(names_, i, mkChar(names[i]) );
  }
  setAttrib(df_, R_NamesSymbol, names_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set my own attribute to keep track of actual length 
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  setAttrib(df_, mkString("data_length"), ScalarInteger(0));
  
  
  UNPROTECT(nprotect);
  return df_;
}





//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Double the memory allocated for each column of the data.frame
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void df_increase_size(SEXP df_) {
  
  unsigned int len = LENGTH(VECTOR_ELT(df_, 0));
  unsigned int new_len = 2 * len;
  
  unsigned int ncol = length(df_);
  for (int i = 0; i < ncol; i++) {
    SET_VECTOR_ELT(df_, i, lengthgets(VECTOR_ELT(df_, i) , new_len) );
  }
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Add a new row to a data.frame
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void df_add_row_core(SEXP df_, va_list args) {
  
  int nprotect = 0;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // After truncating data.frame, don't let this function grow it anymore
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (IS_GROWABLE( VECTOR_ELT(df_, 0) )) {
    error("df_add_row: data.frame is growable!");
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // - get current 'data'length'
  // - get allocated length
  // - check we haven't already set the GROWABLE bit
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP data_length_ = PROTECT(getAttrib(df_, mkString("data_length"))); nprotect++;
  if (isNull(data_length_)) {
    error("df_add_row: No  'data_length' attribute.");
  }
  int data_length = asInteger(data_length_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Get allocated length
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int allocated_length = length(VECTOR_ELT(df_, 0));
  // Rprintf("data_length: %i   allocated_length: %i\n", data_length, allocated_length);
  
  if (data_length >= allocated_length) {
    // Rprintf("df_add_row: Length exceeded. growing data.frame");
    df_increase_size(df_);
    
    allocated_length = length(VECTOR_ELT(df_, 0));
  }
  
  const unsigned int idx = data_length;
  
  
  int ncol = length(df_);
  
  double new_dbl;
  int    new_int;
  char  *new_chr;
  SEXP   new_sxp;
  
  for (int i = 0; i < ncol; i++) {
    SEXP data_ = PROTECT(VECTOR_ELT(df_, i));
    int type = TYPEOF(data_);
    switch(type) {
    case REALSXP:
      new_dbl = va_arg(args, double);
      REAL(data_)[idx] = new_dbl;
      break;
    case INTSXP:
    case LGLSXP:
      new_int = va_arg(args, int);
      INTEGER(data_)[idx] = new_int;
      break;
    case STRSXP:
      new_chr = va_arg(args, char*);
      SET_STRING_ELT(data_, idx, mkChar(new_chr));
      break;
    case VECSXP:
      new_sxp = PROTECT(va_arg(args, SEXP));
      SET_VECTOR_ELT(data_, idx, new_sxp);
      UNPROTECT(1);
      break;
    default:
      error("df_add_row: type not supported: %i %s", type, type2char(type));
    }
    
    UNPROTECT(1);
  }
  va_end(args);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // `data_length` has increased by 1
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  setAttrib(df_, mkString("data_length"), ScalarInteger(data_length + 1));
  
  UNPROTECT(nprotect);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// It seems really hard (if not impossoble) to export a C function from 
// an R package that accepts varags.
// Instead I'm going to export the "df_add_row_core" version, and
// recreate df_add_row(...) in the "deft.h" that the other package will 
// import.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void df_add_row(SEXP df_, ...) {
  va_list args;
  va_start(args, df_);
  df_add_row_core(df_, args);
  va_end(args);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Truncate a data.frame to the length specified in the 'data_length' 
// attribute
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void df_truncate_to_data_length(SEXP df_) {
  
  unsigned int nprotect = 0;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // You really shouldn't call this function twice on the same data.frame
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (IS_GROWABLE(VECTOR_ELT(df_, 0))) {
    error("df_truncate_to_data_length: data.frame is aleady growable.");
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // This function only works on my  "flexible" data.frames 
  // which have a 'data_length' attribute.
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP data_length_ = PROTECT(getAttrib(df_, mkString("data_length"))); nprotect++;
  if (isNull(data_length_)) {
    error("df_truncate_to_data_length: data.frame does not have a 'data_length' attribute.");
  }
  
  unsigned int data_length      = asInteger(data_length_);
  unsigned int allocated_length = LENGTH(VECTOR_ELT(df_, 0));
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Sanity checking: if this is screwy then just throw an error. HARD
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (data_length > allocated_length) {
    error("df_truncate_to_data_length: 'data_length' (%i) >= 'allocated_length' (%i).", data_length, allocated_length);
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set each member vector to be shorter
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  for (int i=0; i < length(df_); i++) {
    SETLENGTH(VECTOR_ELT(df_, i), data_length);
    SET_TRUELENGTH(VECTOR_ELT(df_, i), allocated_length);
    SET_GROWABLE_BIT(VECTOR_ELT(df_, i));
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Set the row.names on the list.
  // Use the shortcut as used in .set_row_names() in R
  // i.e. set rownames to c(NA_integer, -len) and it will
  // take care of the rest. This is equivalent to rownames(x) <- NULL
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP rownames = PROTECT(allocVector(INTSXP, 2));  nprotect++;
  SET_INTEGER_ELT(rownames, 0, NA_INTEGER);
  SET_INTEGER_ELT(rownames, 1, -data_length);
  setAttrib(df_, R_RowNamesSymbol, rownames);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Remove data_length attribute
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  setAttrib(df_, mkString("data_length"), R_NilValue);
  
  
  UNPROTECT(nprotect);
}




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// DEMO
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP deft_benchmark_(SEXP n_) {
  
  int nprotect = 0;
  int n;
  
  n = asInteger(n_);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Create dataframe
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  char *names[5] = {    "x",    "y",    "z",  "lgl", "list"};
  int   types[5] = {REALSXP, INTSXP, STRSXP, LGLSXP, VECSXP};
  SEXP df = PROTECT(df_create(5, names, types));  nprotect++;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Add values
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  for (int i = 0; i < n; i++) {
    // It is the caller's responsibility to ensure that the number and type
    // of the values passed to 'dv_add_row' are correct
    //
    //              double  , integer, char*, integer, SEXP
    df_add_row(df, (double)i, i, names[1], i % 2, ScalarInteger(i));
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Truncate to data length just prior to returning to R
  // This function *MUST* be called on any data.frame created by this code.
  // This function:
  //    * Trims the data to the actual data length
  //    * Sets the GROWABLE bit to indicate that it has been truncated
  //    * sets rownames (without which R will complain very very loudly 
  //        that the data.frame is corrupt)
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  df_truncate_to_data_length(df);
  
  
  UNPROTECT(nprotect);
  return df;
}









