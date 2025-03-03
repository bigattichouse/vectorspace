#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
 int readposition;
 vs_list *vectors;
 vs_listnode *current;
 vs_cursor *cursor;
} vs_rowset;

/*Rowset is designed to access the cursor by position, so we can have
multiple "clients" on a single large cursor without all that darn cloning going on!*/

vs_rowset *vsr_createrowset(vs_cursor *cursor, vector *query);
int vsr_destroyrowset (vs_rowset **rowset);
int vsr_reset    (vs_rowset *rowset);
vector *vsr_readnext (vs_rowset *rowset);
int vsr_eof  (vs_rowset *rowset);

