typedef struct {
  void *greater,*lesser;
  t_uuid dimensionid;
  long position;
  vs_list *vectors;
} __cdim_node;

typedef struct {
  __cdim_node *root;
} __cdim_index;

__cdim_index *__cdim_createcursorindex (vs_cursor *cursor);
void __cdim_destroyindex(__cdim_index **index);
long __cdim_insertvector (__cdim_index *index, vector *v);
long __cdim_deletevector (__cdim_index *index, vector *v);



vs_list *__cdim_query(vs_cursor *c, vector *v);


long __cdim_dimension_frequency (__cdim_index *index, t_uuid dimensionid);

