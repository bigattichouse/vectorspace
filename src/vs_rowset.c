#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "vs_core.h"
#include "vs_list.h"
#include "vs_cursor.h"
#include "vs_rowset.h"

vs_rowset *vsr_createrowset(vs_cursor *cursor, vector *query){
  vs_rowset *result;
  result = malloc(sizeof(vs_rowset));
  result->readposition=0;
  result->cursor=cursor;     
  result->current=NULL;
  return(result);
}

int vsr_destroyrowset (vs_rowset **rowset){
  free(*rowset);
  return(1);
}

int vsr_reset    (vs_rowset *rowset){
  rowset->readposition = 0 ;
  rowset->current=NULL;
  return(1);
}
                                                

vector *vsr_readnext (vs_rowset *rowset){
  vector *v;
  vs_listnode *nextnode,*current;
  v=NULL;
  current = rowset->current;
  if (current!=NULL){
    nextnode = current->next;
    rowset->current = nextnode;
    if (nextnode!=NULL){v = nextnode->item; }
  } else {
    nextnode = rowset->cursor->list->first;
    if (nextnode!=NULL){v = nextnode->item; }
    rowset->current = nextnode;
  }
  rowset->readposition += 1;
  return(v);
}

int vsr_eof  (vs_rowset *rowset){
 return((rowset->readposition >= (rowset->cursor->count)));
}
