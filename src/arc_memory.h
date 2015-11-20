

typedef struct {
  arc_story *first,*scroll,*last;
  int count;
  int readposition;
} arc_memory ;

arc_memory *arc_create_memory();
int arc_destroy_memory(arc_memory **memory, int destroy_stories); //set destroy_memories=0 if you are quering.
int arc_store(arc_memory *memory,arc_story *story); //put story into memory
arc_memory *arc_recall(arc_memory *memory,arc_story *query, float threshold); //may bring back more than one memory
int arc_forget(arc_memory *memory); //forgets the oldest item in the list.
//savetofile, etc..
void arc_memory_print(arc_memory *memory);


void arc_memory_reset(arc_memory *memory);
int arc_memory_eof(arc_memory *memory);
arc_story *arc_memory_scroll(arc_memory *memory);
