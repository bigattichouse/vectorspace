#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "vs_core.h"
#include "vs_list.h"
#include "vs_cursor.h"
#include "vs_indexrowset.h"
#include "vs_cursorindex.h"

vs_indexrowset *vsi_createrowset(vs_cursor *cursor, vector *query){
  vs_indexrowset *result;
  result = malloc(sizeof(vs_indexrowset));
  result->readposition=0;
  result->cursor=cursor;
  result->current=NULL;
  result->vectors = __cdim_query(cursor,query);
  return(result);
}

int vsi_destroyrowset (vs_indexrowset **rowset){
  vs_list *vectors;
  vectors = (*rowset)->vectors;
  vs_clean_list(vectors);
  vs_destroy_list(&vectors);
  free(*rowset);
  return(1);
}

int vsi_reset (vs_indexrowset *rowset){
  rowset->readposition = 0 ;
  rowset->current=NULL;
  return(1);
}


vector *vsi_readnext (vs_indexrowset *rowset){
  vector *v;
  vs_listnode *nextnode,*current;
  v=NULL;
  current = rowset->current;
  if (current!=NULL){
    nextnode = current->next;
    rowset->current = nextnode;
    if (nextnode!=NULL){v = nextnode->item; }
  } else {
    nextnode = rowset->vectors->first;
    if (nextnode!=NULL){v = nextnode->item; }
    rowset->current = nextnode;
  }
  rowset->readposition += 1;
  return(v);
}

int vsi_eof  (vs_indexrowset *rowset){
 return((rowset->readposition >= (rowset->vectors->size)));
}


