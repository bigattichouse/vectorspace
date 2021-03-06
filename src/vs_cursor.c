#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "vs_core.h"
#include "vs_dimensionindex.h"
#include "vs_list.h"
#include "vs_cursor.h"
#include "vs_cursorindex.h"


const long VSC_CURSOR = 1;
const long VSC_VIEW = 2;

//cursors are an inmemory "chunk" that helps speed up comparisons.. by loading - say 1000 vectors longo memory
//at a time... then allowing faster reads. reads are forward only, but can be reset back to origin.
//the vectors are stored as vector * in memory for fastest access.


void vsc_print_cursor(vs_cursor *cursor){
 vector *v;
 vsc_reset(cursor);
 printf("C\r\n");
 while (!vsc_eof(cursor)){
    v = vsc_readnext(cursor);
    vs_printvector(v);   
 }
}

long vsc_reset (vs_cursor *cursor){
  cursor->readposition = 0 ;
  cursor->current=NULL;
  //cursor->count=vs_list_size(cursor->list) ;
  return(1);
}

vs_cursor *vsc_createcursor(){
  vs_cursor *cursor;
  cursor = (vs_cursor *)malloc(sizeof(vs_cursor)+1);
  cursor->readposition=0;
  cursor->count=0;
  cursor->indexed =1;
  cursor->type = VSC_CURSOR;
  cursor->list = vs_prepare_unindexed_list();
  ((vs_list *)cursor->list)->index = NULL;
  cursor->rowset_reference=0;
  cursor->index=__cdim_createcursorindex(cursor);
  return(cursor);
}

long vsc_indexcursor(vs_cursor *cursor){

    return(-1);

}

void vsc_sync_view(vs_cursor *original,vs_cursor *view){
  view->count=original->count;
  view->indexed = original->indexed;
  view->list = original->list;
}

vs_cursor *vsc_create_view(vs_cursor *original){
  vs_cursor *cursor;
  cursor = (vs_cursor *)malloc(sizeof(vs_cursor)+1);
  cursor->readposition=0;
  cursor->current=NULL;
  cursor->indexed = original->indexed;
  cursor->type = VSC_VIEW;
  cursor->rowset_reference=0;
  vsc_sync_view(original,cursor);
  return(cursor);
}


long vsc_destroycursor(vs_cursor **cursor){
 vector *v;
 vs_cursor *c;
 __cdim_index *index;
 if((*cursor!=NULL) && (cursor!=NULL)){
  c = *cursor;
 if (c->type == VSC_CURSOR){ // handles both, as views only get freed, while cursors get cleared and freed.
 vsc_reset(c);
 while (!vsc_eof(c)){
    v = vsc_readnext(c);
    vs_destroyvector(&v);
    c->current->item = NULL;
 }
  vsc_reset(c);
  vs_destroy_list(&c->list);
  index = c->index;
  if (index!=NULL){__cdim_destroyindex(&index);}
}
  free(*cursor);
 }
 return(1);
}

vs_cursor *vsc_clonecursor(vs_cursor *source){
  vs_cursor *dest;
  vector *sourcevector ;
  dest = vsc_createcursor();
  dest->indexed=source->indexed;
  vsc_reset(source);
  while (!vsc_eof(source)){
   sourcevector = vsc_readnext(source);
   vsc_rawloadvector(dest,sourcevector); //clones vector to unique copy.
  }
  vsc_reset(dest); vsc_reset(source);
  return(dest);
}

vector *vsc_readnext (vs_cursor *cursor){
  vector *v;
  vs_listnode *nextnode,*current;
  v=NULL;
  current = cursor->current;
  if (current!=NULL){
    current->previous = current;
    nextnode = current->next;
    cursor->current = nextnode;
    if (nextnode!=NULL){v = nextnode->item; }
  } else {
    nextnode = cursor->list->first;
    if (nextnode!=NULL){v = nextnode->item; }
    cursor->current = nextnode;
  }
  cursor->readposition += 1;
  return(v);
}


/*
vector *vsc_readnext (vs_cursor *cursor){
  vector *v;
  vs_listnode *nextnode,*current;
  v=NULL;
  current = cursor->current;
  if (current!=NULL){
    nextnode = current->next;
    cursor->current = nextnode;
    if (nextnode!=NULL){v = nextnode->item; }
  } else {
    nextnode = cursor->list->first;
    if (nextnode!=NULL){v = nextnode->item; }
    cursor->current = nextnode;
  }
  cursor->readposition += 1;
  return(v);
}
*/


long vsc_eof (vs_cursor *cursor){
 return((cursor->readposition >= (cursor->count)));
}

vector *vsc_findvector (vs_cursor *cursor,t_uuid vectorid){
 vector *v;

 vs_listnode *node;
 node = vs_list_find(cursor->list,vectorid);
 if (node!=NULL){
    v=node->item;
    return(v);
 } else {return(NULL);}
}

long vsc_replacevector (vs_cursor *cursor,vector *v){
  vector *vold,*v2 ;
  vs_listnode *node;
  long result;
  __cdim_index *index;
index = cursor->index;
result=0;

if (cursor->type == VSC_CURSOR){   //updates and deletes only valid on cursors;
  node = vs_list_find(cursor->list,v->vectorid);
 if (node!=NULL){
    if (v->dimensioncount>0){
       vold = node->item;
       if (index!=NULL){__cdim_deletevector(cursor->index,vold); }
       vs_destroyvector(&vold);

       node->item = vs_createvector(v->vectorid,v->dimensioncount);
       v2 = (vector *)node->item;
       vs_clone(v,v2);
       vs_magnitude(v2);
       if(cursor->indexed==1){ __vdim_reindexvector(v2);}
       if (index!=NULL){__cdim_insertvector(cursor->index,v2);   }
    } else {
       if (index!=NULL){ __cdim_deletevector(cursor->index,v); }
        vsc_deletevector(cursor,v->vectorid);
    }
 } else {
   if (v->dimensioncount>0){
    v2 =vs_createvector(v->vectorid,v->dimensioncount);
    vs_clone(v,v2);
    vs_list_add (cursor->list,v2,v->vectorid);
    if(cursor->indexed==1){ __vdim_reindexvector(v2); }
    cursor->count+=1;
    if (index!=NULL){__cdim_insertvector(cursor->index,v2);  }
   }
  }
       result = cursor->count - 1;
}
  return(result);
}


long vsc_rawloadvector (vs_cursor *cursor,vector *v){
  vector *v2 ;
 // vs_listnode *node;
  long result;
  __cdim_index *index;
index = cursor->index;
result=0;
if (cursor->type == VSC_CURSOR){   //updates and deletes only valid on cursors;
    v2 =vs_createvector(v->vectorid,v->dimensioncount);
    vs_clone(v,v2);
    vs_reindex(v2);
    vs_list_add (cursor->list,v2,v->vectorid);
    if(cursor->indexed==1){ __vdim_reindexvector(v2); }
    if (index!=NULL){ __cdim_insertvector(cursor->index,v2);  }
    cursor->count+=1;

    result = cursor->count - 1;
}
  return(result);
}

long vsc_rawinsertvector (vs_cursor *cursor,vector *v){
  long result;
  __cdim_index *index;
index = cursor->index;
result=0;
if (cursor->type == VSC_CURSOR){   //updates and deletes only valid on cursors;
    vs_list_add (cursor->list,v,v->vectorid);
    if(cursor->indexed==1){ __vdim_reindexvector(v); }
    if (index!=NULL){__cdim_insertvector(cursor->index,v);}
    cursor->count+=1;
    result = cursor->count - 1;
}
  return(result);
}


long vsc_deletevector(vs_cursor *cursor,t_uuid vectorid){
  vs_listnode *node;
  vector *v;
  long result;
  __cdim_index *index;
index = cursor->index;
result=0;
if (cursor->type == VSC_CURSOR){   //updates and deletes only valid on cursors;
  node = vs_list_find(cursor->list,vectorid);
  if (node!=NULL){
   if (node->item!=NULL){
    v =(vector *)node->item;
    if (node==cursor->current){
       cursor->current = node->next;
     }
    if (index!=NULL){__cdim_deletevector(cursor->index,v); }
    vs_destroyvector(&v);
    node->item = NULL;
    vs_list_delete(cursor->list,vectorid);
    cursor->count=vs_list_size(cursor->list) ;
   }
  }
  result=1;
}
 return(result);
}


vector *vsc_dimension_analysis (vs_cursor *cursor) {
  vector *result;
  vector *check;
  t_uuid id;
  dimension *dim,*dims,*dimresult;
  vs_value vcheck,vresult;
  long max,i,val,j;
  id = StringToHash("11111111111111111111111111111111");
  result = vs_createvector(id,1);
  vs_clearindex(result);
  printf("%ld\r\n",cursor->count);
  j=0;
  vsc_reset(cursor);
  while ( !vsc_eof(cursor) ){
   check = vsc_readnext(cursor);
    //if(j % 100==0){ printf("%d: %d unique dimensions\r\n",j,result->dimensioncount);}
    j++; 
    max = check->dimensioncount;
    i=0;
    dims = check->dimensions;
    while (i<max){
       val = (long)dims + (long)(i * sizeof(dimension));
       dim = (dimension *)(val);    //vs_getdimensionbyindex + vs_getvalue to create a new dim value.
       dimresult = vs_getvalue(result, dim->dimensionid);
      if (dimresult!=NULL){
        vresult = dimresult->value;
        vresult.floatvalue += 1;
        vs_setvalue(result,dim->dimensionid,vresult);
      } else {
        vcheck.floatvalue=1;
        vs_setvalue(result,dim->dimensionid,vcheck);
        vs_reindex(result);
      }
       i++;
     }
  }
  return(result);
}


