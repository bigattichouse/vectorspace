

FILE *_vdd_open(char *base_directory,char *filename);
void _vdd_close(FILE *index);
vector *_vdd_retrieve (FILE *data,int filepos);
void _vdd_delete (FILE *data,int filepos);
int _vdd_append(FILE *data,vector *v);
int _vdd_replace (FILE *data,int filepos,vector *v);

