#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "port.h"
#include "uuid.h"
#include "vs_queue.h"

vs_queue *vs_prepare_queue(){
 vs_queue *queue;
 queue = malloc(sizeof(vs_queue));
 queue->size=0;
 queue->first=NULL;
 queue->last=NULL;
 return(queue);
}

void vs_clear_queue(vs_queue *queue){
  vs_queuenode *node;
  while (queue->size>0){
     node = vs_queue_pop(queue);
     vs_destroy_node(&node);
  }
}

void vs_destroy_queue(vs_queue **queue){
  vs_clear_queue(*queue);
  free(*queue);
}

void vs_destroy_node(vs_queuenode **node){
 vs_queuenode *n;
 n = *node;
 if (n->buffer!=NULL){free(n->buffer);}
 free(*node);
}

vs_queuenode *vs_queue_pop (vs_queue *queue) {
 vs_queuenode *node;
 node=NULL;
 if (queue->first!=NULL){
   node = queue->first;
   if (node->next!=NULL){
      queue->first = node->next;
      queue->size-=1;
   } else {
      queue->first = NULL;
      queue->last  = NULL;
      queue->size=0;
   }
 }
 return(node);
}

void vs_queue_putback (vs_queue *queue,vs_queuenode *node){
  node->next=queue->first;
  queue->first=node;
  if (queue->last==NULL){queue->last=node;}
}

void vs_queue_push (vs_queue *queue,char *buffer, int size, t_uuid channel){
 vs_queuenode *node;
 node=malloc(sizeof(vs_queuenode));
 node->size = size;
 node->buffer = buffer;
 node->next = NULL;
 if (queue->first==NULL){
   queue->first=node;
   queue->size=1;
   queue->last=node;
 } else {
  queue->last->next = node;
  queue->last = node;
  queue->size+=1;
 }
 node->channel = channel;
}

int vs_queue_size (vs_queue *queue){
  return(queue->size);
}
