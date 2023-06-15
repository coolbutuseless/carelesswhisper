

SEXP df_create(int ncol, char **names, int *types);
void df_increase_size(SEXP df_);
void df_add_row(SEXP df_, ...);
void df_truncate_to_data_length(SEXP df_);
