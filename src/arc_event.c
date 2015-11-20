#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "vs_core.h"
#include "vs_dimensionindex.h"
#include "vs_list.h"
#include "arc_event.h"


const int ae_event = 1;
const int ae_VIEW = 2;

//events are an inmemory "chunk" that helps speed up comparisons.. by loading - say 1000 vectors into memory
//at a time... then allowing faster reads. reads are forward only, but can be reset back to origin.
//the vectors are stored as vector * in memory for fastest access.


int ae_reset (arc_event *event){
  event->readposition = 0 ;
  event->current=NULL;
  //event->count=vs_list_size(event->list) ;
  return(1);
}

arc_event *ae_createevent(){
  arc_event *event;
  event = (arc_event *)malloc(sizeof(arc_event)+1);
  event->readposition=0;
  event->count=0;
  event->indexed =1;
  event->type = ae_event;
  event->list = vs_prepare_list();
  return(event);
}


void ae_sync_view(arc_event *original,arc_event *view){
  view->count=original->count;
  view->indexed = original->indexed;
  view->list = original->list;
}

arc_event *ae_create_view(arc_event *original){
  arc_event *event;
  event = (arc_event *)malloc(sizeof(arc_event)+1);
  event->readposition=0;
  event->current=NULL;
  event->indexed = original->indexed;
  event->type = ae_VIEW;
  ae_sync_view(original,event);
  return(event);
}

int ae_clear(arc_event **event){
 vector *v;
 vs_list *list;
 arc_event *c;
 if((*event!=NULL) && (event!=NULL)){
  c = *event;
 if (c->type == ae_event){ // handles both, as views only get freed, while events get cleared and freed.
 ae_reset(c);
 while (!ae_eof(c)){
    v = ae_readnext(c);
    vs_destroyvector(&v);
    c->current->item = NULL;
 }
  ae_reset(c);
}
 }
 return(1);
}

int ae_destroyevent(arc_event **event){
 vector *v;
 vs_list *list;
 arc_event *c;
 ae_clear(event);
 if((*event!=NULL) && (event!=NULL)){
  c = *event;
  list = c->list;
  vs_destroy_list(&c->list);
  free(*event);
 }
 return(1);
}

arc_event *ae_cloneevent(arc_event *source){
  arc_event *dest;
  vector *sourcevector ;
  dest = ae_createevent();
  dest->indexed=source->indexed;
  ae_reset(source);
  while (!ae_eof(source)){
   sourcevector = ae_readnext(source);
   ae_replacevector(dest,sourcevector); //clones vector to unique copy.
  }
  ae_reset(dest); ae_reset(source);
  return(dest);
}

int ae_cloneevent(arc_event *source, arc_event *dest){
  arc_event *dest;
  vector *sourcevector ;
  dest->indexed=source->indexed;
  ae_clear(dest);
  ae_reset(source);
  while (!ae_eof(source)){
   sourcevector = ae_readnext(source);
   ae_replacevector(dest,sourcevector); //clones vector to unique copy.
  }
  ae_reset(dest); ae_reset(source);
  return(dest);
}

vector *ae_readnext (arc_event *event){
  vector *v;
  vs_listnode *nextnode,*current;
  v=NULL;
  current = event->current;
  if (current!=NULL){
    current->previous = current;
    nextnode = current->next;
    event->current = nextnode;
    if (nextnode!=NULL){v = nextnode->item; }
  } else {
    nextnode = event->list->first;
    if (nextnode!=NULL){v = nextnode->item; }
    event->current = nextnode;
  }
  event->readposition += 1;
  return(v);
}


/*
vector *ae_readnext (arc_event *event){
  vector *v;
  vs_listnode *nextnode,*current;
  v=NULL;
  current = event->current;
  if (current!=NULL){
    nextnode = current->next;
    event->current = nextnode;
    if (nextnode!=NULL){v = nextnode->item; }
  } else {
    nextnode = event->list->first;
    if (nextnode!=NULL){v = nextnode->item; }
    event->current = nextnode;
  }
  event->readposition += 1;
  return(v);
}
*/


int ae_eof (arc_event *event){
 return((event->readposition >= (event->count)));
}

vector *ae_findvector (arc_event *event,t_uuid vectorid){
 vector *v;
 vs_listnode *node;
 node = vs_list_find(event->list,vectorid);
 if (node!=NULL){
    v=node->item;
    return(v);
 } else {return(NULL);}
}

int ae_replacevector (arc_event *event,vector *v){
  vector *vold,*v2 ;
  vs_listnode *node;
  int result;
result=0;
if (event->type == ae_event){   //updates and deletes only valid on events;
  node = vs_list_find(event->list,v->vectorid);
 if (node!=NULL){
    if (v->dimensioncount>0){
       vold = node->item;
       vs_destroyvector(&vold);
       node->item = vs_createvector(v->vectorid,v->dimensioncount);
       vs_clone(v,(vector *)node->item);
       vs_magnitude((vector *)node->item);
       if(event->indexed==1){ __vdim_reindexvector((vector *)node->item);}
    } else {ae_deletevector(event,v->vectorid);}
 } else {
   if (v->dimensioncount>0){
    v2 =vs_createvector(v->vectorid,v->dimensioncount);
    vs_clone(v,v2);
    vs_list_add (event->list,v2,v->vectorid);
    if(event->indexed==1){ __vdim_reindexvector(v2); }
    event->count+=1;
   }
  }
       result = event->count - 1;
}
  return(result);
}

int ae_deletevector(arc_event *event,t_uuid vectorid){
  vs_listnode *node;
  vector *v;
  int result;
result=0;
if (event->type == ae_event){   //updates and deletes only valid on events;
  node = vs_list_find(event->list,vectorid);
  if (node!=NULL){
   if (node->item!=NULL){
    v =(vector *)node->item;
    if (node==event->current){
       event->current = node->next;
     }
    vs_destroyvector(&v);
    node->item = NULL;
    vs_list_delete(event->list,vectorid);
    event->count=vs_list_size(event->list) ;
   }
  }
  result=1;
}
 return(result);
}



