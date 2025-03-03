typedef struct {
 char command; /*Q,V,R,etc.*/
 vector *current_vector;
 int vectorcount;
 int mode;
 float thesaurus;
 float threshold ;
 time_t touched;
 t_uuid id;
} interpreter_session;

int vsinterpreter_execute (vs_cursor *cursor,vs_cursor *thesaurus,interpreter_session *session, char command, t_uuid elementid,float value, vs_queue *output_buffer , FILE 	*logfile);
