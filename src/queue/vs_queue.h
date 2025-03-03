//linked list queue... read out it acheived via pop
// int = vs_queue_pop (vsqueue *queue,char **buff) ; int=0 if nothing there..
//            int reflects item size of something returned and buff points to data
// void vs_queue_push (vsqueue *queue,char *buff, int size);
// int = vs_queue_size (vsqueue *queue);

// vsdb will be using these queues to buffer output.

typedef struct {
  int size;
  char *buffer;
  void *next;
  t_uuid channel;
} vs_queuenode;

typedef struct {
   vs_queuenode *first,*last;
   int size;
} vs_queue;

vs_queue *vs_prepare_queue();
void vs_clear_queue(vs_queue *queue);
void vs_destroy_queue(vs_queue **queue);
void vs_destroy_node(vs_queuenode **node);
vs_queuenode *vs_queue_pop (vs_queue *queue);
void vs_queue_putback (vs_queue *queue,vs_queuenode *node);
void vs_queue_push (vs_queue *queue,char *buffer, int size, t_uuid channel);
int vs_queue_size (vs_queue *queue);
