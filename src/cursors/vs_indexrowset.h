#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
 int readposition;
 vs_list *vectors;
 vs_listnode *current;
 vs_cursor *cursor; 
} vs_indexrowset;

/*Rowset is designed to access the cursor by position, so we can have
multiple "clients" on a single large cursor without all that darn cloning going on!*/

vs_indexrowset *vsi_createrowset(vs_cursor *cursor, vector *query);
int vsi_destroyrowset (vs_indexrowset **rowset);
int vsi_reset    (vs_indexrowset *rowset);
vector *vsi_readnext (vs_indexrowset *rowset);
int vsi_eof  (vs_indexrowset *rowset);

