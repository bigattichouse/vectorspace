int vp_save_cursor (vs_cursor *cursor, char *filename);
int vp_load_cursor (vs_cursor *cursor, char *filename);
int vp_load_text_cursor (vs_cursor *cursor, char *filename);


vector *vp_readvector (FILE *fp);
void vp_clearvector (FILE *fp,vector *v);
void vp_writevector (FILE *fp,vector *v);


