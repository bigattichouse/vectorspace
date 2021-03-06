#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "vs_core.h"
#include "vs_list.h"

//another damn index... the find in lists is too slow.
typedef struct {
  void *greater,*lesser;
  t_uuid id;
  int position;
  void *item;
  vs_listnode *node;
} __list_node;

typedef struct {
  __list_node *root;
} __list_index;



void __list_clear(__list_node **tree) {
  void *lesser,*greater;
  __list_node *treeref;
  __list_node *lnode,*gnode;
  if (tree!=NULL){
  treeref = (*tree);
  if (treeref!=NULL){
    lesser =  (treeref->lesser);
    greater =  (treeref->greater);
    lnode = (__list_node *) lesser;
    gnode = (__list_node *) greater;
    __list_clear(&lnode);
    __list_clear(&gnode);
    free(*tree);
  }
  }
}

void __list_clearlistindex (vs_list *list){
    __list_index *index;
    __list_node *root;
    index = list->index;
    if(index!=NULL){
       root = index->root;
       index->root = NULL;
       __list_clear (&root);
    }
}


__list_node *__list_insertnode(vs_list *list,t_uuid id,int position,void *item,vs_listnode *node,__list_node *start){
 t_uuid rid,iid;
   __list_node *inserted;
   int CMP;
   if(start==NULL) {
      inserted = (__list_node *)malloc(sizeof (__list_node));
      inserted->id = id;
      inserted->position = position;
      inserted->item = item;
      inserted->node = node;
      inserted->lesser = NULL;
      inserted->greater = NULL;
      return(inserted);
   } else {
   iid = id;
   rid = start->id;
   CMP =CompareGuids(iid,rid);
   if(CMP<0) {
      inserted = __list_insertnode(list,id,position,item,node,(__list_node *)start->lesser);
      if (start->lesser==NULL){
        start->lesser = (__list_node *)inserted;
      }
      return(inserted);
   } else  if(CMP>0) {
      inserted = __list_insertnode(list,id,position,item,node,(__list_node *)start->greater);
      if (start->greater==NULL){
        start->greater = (__list_node *)inserted;
      }
      return(inserted);
   } else {
     return(start);
   }
   }
}

void __list_indexitem (vs_list *list, void *item, t_uuid id, int position, vs_listnode *vsnode){
    __list_index *index;
    __list_node *inserted,*root;
    index = list->index;
    if (index!=NULL){
      root = index->root;
      inserted = __list_insertnode(list,id,position,item,vsnode,root);
      if (inserted!=NULL){
          if (index->root==NULL){
          index->root = inserted;
          }
      }
    }
}

int __list_createlistindex (vs_list *list){
 __list_index *index;
 vs_listnode *node;
 int max,i ;
 list->index = (__list_index *)malloc(sizeof(__list_index));
 index = list->index;
 index->root = NULL;
 max = list->size;
 i=0;
 node = list->first;
 while (node!=NULL){
    __list_indexitem(list,node->item,node->id,i,node);
    i++;
    node = node->next;
 }
 return(max);
}

void  __list_rebuildlistindex(vs_list *list){
    __list_clearlistindex (list);
    __list_createlistindex(list);
}




vs_list *vs_prepare_unindexed_list(){
 vs_list *list;
 list = malloc(sizeof(vs_list));
 list->size=0;
 list->first=NULL;
 list->last=NULL;
 list->index=NULL;
 return(list);
}


vs_list *vs_prepare_list(){
 vs_list *list;
 list = vs_prepare_unindexed_list();
 __list_createlistindex(list);
 return(list);
}

void vs_clear_list(vs_list *list){
  vs_listnode *node;
  __list_clearlistindex(list);
  if (list!=NULL){
  while (list->size>0){
     node = list->first;
     if (node!=NULL){
        vs_list_delete(list,node->id);
     }
  }
  }
}

void vs_destroy_list(vs_list **list){
 if (*list!=NULL){
  vs_clear_list(*list);
  if ((*list)->index!=NULL){free((*list)->index);}
  free(*list);
 }
}

void vs_destroy_listnode(vs_listnode **node){
 vs_listnode *n;
 n = *node;
 if (node!=NULL){
   if (n->item!=NULL){free(n->item);}
   free(*node);
 }
}

void vs_list_delete(vs_list *list,t_uuid id){
 vs_listnode *node,*last,*next;
 int found; last=NULL;
 node = list->first; found=0;
 while ((found==0) && (node!=NULL)){
   if (IsEqualGuid(node->id,id)){
     if (last==NULL){
      //we are deleting the first element
      list->first = node->next;
      node->previous = NULL;
     } else {
      next = node->next;
      if (next!=NULL){
       last->next = next;
       next->previous = last;
      } else {
       last->next = NULL;
      }
     }
     list->size-=1;
     vs_destroy_listnode(&node);
     found=1;
   } else {
     last = node;
     node = node->next;
   }
 }
}

void vs_list_erase(vs_list *list,t_uuid id){
 vs_listnode *node,*last,*next;
 int found; last=NULL;
 node = list->first; found=0;
 while ((found==0) && (node!=NULL)){
   if (IsEqualGuid(node->id,id)){
     if (last==NULL){
      //we are deleting the first element
      list->first = node->next;
      node->previous = NULL;
     } else {
      next = node->next;
      if (next!=NULL){
       last->next = next;
       next->previous = last;
      }
     }
     list->size-=1;
     free(node);  //ignore the item.. we don't own it.
     found=1;
   } else {
     last = node;
     node = node->next;
   }
 }
}


void vs_clean_list(vs_list *list){
  vs_listnode *node;
  __list_clearlistindex(list);
  if (list!=NULL){
  while (list->size>0){
     node = list->first;
     vs_list_erase(list,node->id);
  }
  }
}




 vs_listnode *vs_list_find_linkedlist(vs_list *list,t_uuid id){
 vs_listnode *node,*foundnode;
 int found; foundnode=NULL;
 node = list->first; found=0;
 while ((!found) && (node!=NULL)){
   if (IsEqualGuid(node->id,id)){
     foundnode=node;
     found=1;
   } else {
     node = node->next;
   }
 }
 return(foundnode);
}


__list_node *__list_recursefindbyID(__list_node *tree, t_uuid id) {
   t_uuid rid ;
   int CMP;
   if(tree!=NULL){
   rid = tree->id;
   CMP = CompareGuids(id,rid);
   if(CMP==0){
     return(tree);
   } else {
     if(CMP<0){
      return(__list_recursefindbyID((__list_node *)tree->lesser, id));
     } else  if(CMP>0) {
      return(__list_recursefindbyID((__list_node *)tree->greater, id));
     }
   }
   }
  return(NULL); //if we get this far, we haven't found it.
}

 vs_listnode *vs_list_find(vs_list *list,t_uuid id){
    __list_node *found;
    __list_index *index;
    if (list->index!=NULL){
      index = list->index;
      found =  __list_recursefindbyID(index->root,id);
      if (found!=NULL){return(found->node);}
      return(NULL);
     } else {
    return(vs_list_find_linkedlist(list,id));
    }
 }

 vs_listnode *vs_list_findbyindex(vs_list *list,int index){
 vs_listnode *node,*foundnode;
 int found,c; foundnode=NULL;
 node = list->first; found=0; c=0;
 while ((!found) && (node!=NULL)){
   if (c==index){
     foundnode=node;
     found=1;
   } else {
     node = node->next;
     c++;
   }
 }
 return(foundnode);
}


void vs_list_add (vs_list *list,void *item, t_uuid id){
 vs_listnode *node;
 node=malloc(sizeof(vs_listnode));
 node->item = item;
 node->next = NULL;
 if (list->first==NULL){
   list->first=node;
   list->size=1;
   list->last=node;
   node->previous = NULL;
 } else {
  list->last->next = node;
  node->previous = list->last->next;
  list->last = node;
  list->size+=1;
 }
 node->id = id;
  if (list->index!=NULL){
   __list_indexitem ( list,  item, id, (list->size)-1,node);
  }
}

int vs_list_size (vs_list *list){
  return(list->size);
}


int vs_list_copy(vs_list *src,vs_list *dst){
 vs_listnode *node;
 int max;
 int found;
 node = src->first; found=0; max=0;
 while ((!found) && (node!=NULL)){
   if(vs_list_find(dst,node->id)==NULL){
     vs_list_add(dst,node->item,node->id);
    }
     node = node->next;
     max++;
   }
 return(max);
}


int vs_list_copy_ids(vs_list *src,vs_list *dst){
 vs_listnode *node;
 int max;
 int found;
 node = src->first; found=0; max=0;
 while ((!found) && (node!=NULL)){
   if(vs_list_find(dst,node->id)==NULL){
     vs_list_add(dst,NULL,node->id);
    }
     node = node->next;
     max++;
   }
 return(max);
}
