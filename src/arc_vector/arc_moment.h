


typedef struct {
  vs_cursor *vectors;
  float duration;
  void *next;
  t_uuid momentid;
} arc_moment;


arc_moment *arc_create_moment();
int arc_destroy_moment(arc_moment **moment);
float arc_moment_similarity(arc_moment *source, arc_moment *query);
int arc_push(arc_moment *moment, vector *v);
arc_moment *arc_clone_moment (arc_moment *source);
arc_moment *arc_moment_vector_query(arc_moment *source, vector *query, float threshold);






