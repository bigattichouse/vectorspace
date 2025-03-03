

typedef struct {
  int greater,lesser,data ;
  t_uuid keyid;
} __disk_node;


FILE *__disk_index_open(char *base_directory,char *filename);

void __disk_index_close(FILE *index);
int __disk_replace_key_value (FILE *index,t_uuid keyid, int filepos); 
int __disk_fetch_value (FILE *index,t_uuid keyid );
int __disk_key_exists (FILE *index,t_uuid keyid);



/*
int __disk_delete_vector (FILE *index, vector *v);

vs_list *__disk_index_query(vs_cursor *c, vector *v);
*/


