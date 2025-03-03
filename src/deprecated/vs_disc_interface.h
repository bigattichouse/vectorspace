
typedef struct {
  FILE *data;  //actual vector storage.
  FILE *Vindex; //where is that dimension?
  __vdd_dimensions *Dindex; //gimme a list of vectors containing dimension X
  char *base_directory;
} __vdi_interface;


__vdi_interface *__vdi_create_disk_interface(char *base_directory);
void __vdi_close(__vdi_interface **intf);

void  __vdi_replace_vector(__vdi_interface *intf,vector *v);
void  __vdi_delete_vector(__vdi_interface *intf,vector *v);
void  __vdi_delete_vectorbyid(__vdi_interface *intf,t_uuid vectorid);

int __vdi_query_load_cursor(__vdi_interface *intf, vector *query, vs_cursor *cursor);

void __vdi_rawappend_vector(__vdi_interface *intf,vector *v);






