#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
 int readposition;
 int indexed;
 vs_list *list;
 vs_listnode *current,*prev;
 int count;
 int type;
} arc_event;


arc_event *ae_createcursor();
int ae_destroycursor(arc_event **cursor);
arc_event *ae_clonecursor(arc_event *source);
int ae_clonecursor(arc_event *source, arc_event *dest);
arc_event *ae_create_view(arc_event *original);
void ae_sync_view(arc_event *original,arc_event *view);

int ae_reset    (arc_event *cursor);
vector *ae_readnext (arc_event *cursor);
int ae_eof      (arc_event *cursor);

vector *ae_findvector (arc_event *cursor,t_uuid vectorid);
int ae_replacevector (arc_event *cursor,vector *v);
int ae_deletevector(arc_event *cursor,t_uuid vectorid);


