#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
 long readposition;
 long indexed;
 vs_list *list;
 vs_listnode *current,*prev;
 long count;                    
 long type;
 long rowset_reference;
 void *index;
} vs_cursor;


vs_cursor *vsc_createcursor();
long vsc_destroycursor(vs_cursor **cursor);
vs_cursor *vsc_clonecursor(vs_cursor *source);
vs_cursor *vsc_create_view(vs_cursor *original);
void vsc_sync_view(vs_cursor *original,vs_cursor *view);

long vsc_reset    (vs_cursor *cursor);
vector *vsc_readnext (vs_cursor *cursor);
long vsc_eof      (vs_cursor *cursor);

vector *vsc_findvector (vs_cursor *cursor,t_uuid vectorid);
long vsc_replacevector (vs_cursor *cursor,vector *v);
long vsc_deletevector(vs_cursor *cursor,t_uuid vectorid);

long vsc_indexcursor(vs_cursor *cursor);
vector *vsc_dimension_analysis (vs_cursor *cursor);

long vsc_rawloadvector (vs_cursor *cursor,vector *v); //creates a copy and inserts the copy... so you can free your vector
long vsc_rawinsertvector (vs_cursor *cursor,vector *v); //directly inserts, so don't free the vector

void vsc_print_cursor(vs_cursor *cursor);
