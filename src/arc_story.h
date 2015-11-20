

typedef struct {
  arc_moment *first,*last,*scroll ;
  int count;
  int readposition;
  float score; //used in the arc_memory to return score results
  float age; //for tracking how old a memory is.
  void *next;
  t_uuid storyid;
} arc_story;


arc_story *arc_create_story();
int arc_destroy_story(arc_story **story);
float arc_story_similarity(arc_story *source, arc_story *query);
int arc_story_push (arc_story *story, arc_moment *moment); //push to front of list
arc_moment *arc_story_pop (arc_story *story);  //pop off end of list
int arc_story_elapse (arc_story *story, arc_moment *moment, float clock, float threshold); //push if score < threshold, otherwise add clock to duration of first.
int arc_story_clone(arc_story *src, arc_story *dest);


void arc_story_reset(arc_story *story);
int arc_story_eof(arc_story *story);
arc_moment *arc_story_scroll(arc_story *story);

void arc_story_print(arc_story *src);
