
typedef struct {
  void *greater,*lesser;
  t_uuid dimensionid;
  int position;
  vs_list *vectors;
} __ddim_node;

typedef struct {
  __ddim_node *root;
} __vdd_dimensions;


__vdd_dimensions *__vdd_create_disk_dimensions();
void __ddim_destroyindex(__vdd_dimensions **index);
int __ddim_insertvector (__vdd_dimensions *index, vector *v);
int __ddim_insertvectorid (__vdd_dimensions *index, t_uuid dimensionid, t_uuid vectorid);
int __ddim_deletevector (__vdd_dimensions *index, vector *v);

vs_list *__ddim_query(__vdd_dimensions *index, vector *v);







