//linked list list... read out it acheived via pop
// int = vs_list_pop (vslist *list,char **buff) ; int=0 if nothing there..
//            int reflects item size of something returned and buff points to data
// void vs_list_push (vslist *list,char *buff, int size);
// int = vs_list_size (vslist *list);

// vsdb will be using these lists to buffer output.

struct vs_listnode;



typedef struct {
  t_uuid id;
  float value;
  void *item;
  void *previous;
  void *next;
} vs_listnode;

typedef struct {
   vs_listnode *first,*last;
   int size;
   void *index;
} vs_list;


vs_list *vs_prepare_list();
vs_list *vs_prepare_unindexed_list();
void vs_clear_list(vs_list *list);
void vs_destroy_list(vs_list **list);
void vs_destroy_listnode(vs_listnode **node);
void vs_list_delete(vs_list *list,t_uuid id);
void vs_list_add (vs_list *list,void *item, t_uuid id);
void vs_list_add_value (vs_list *list,void *item, t_uuid id, float value);
int vs_list_size (vs_list *list);
vs_listnode *vs_list_find(vs_list *list,t_uuid id);
vs_listnode *vs_list_findbyindex(vs_list *list,int index);
int vs_list_copy(vs_list *src,vs_list *dst);
int vs_list_copy_ids(vs_list *src,vs_list *dst);

//non ownership "clean"
void vs_list_erase(vs_list *list,t_uuid id);
void vs_clean_list(vs_list *list);
