#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "vs_core.h"
#include "vs_dimensionindex.h"

typedef struct {
  void *greater,*lesser;
  t_uuid id;
  long position;
  dimension *dim;
} __vdim_node;

typedef struct {
  __vdim_node *root;
} __vdim_index;


t_uuid largestguid(){
 t_uuid val;
 char c;
 c = 255;
 memset(&val,c,sizeof(t_uuid));
 return(val);
}

t_uuid smallestguid(){
 return(NullGuid());
}



void __vdim_clear(__vdim_node **tree) {
  void *lesser,*greater;
  __vdim_node *lnode,*gnode;
  if ((*tree)!=NULL){
    lesser =  ((*tree)->lesser);
    greater =  ((*tree)->greater);
    lnode = (__vdim_node *) lesser;
    gnode = (__vdim_node *) greater;
    __vdim_clear(&lnode);
    __vdim_clear(&gnode);
    free(*tree);
  }
}

void __vdim_clearvectorindex (vector *v){
    __vdim_index *index;
    index = v->index;
    if(index!=NULL){
       __vdim_clear (&index->root);
       free(index);
    }
    v->index=NULL;
}


__vdim_node *__vdim_insert(__vdim_node *tree, __vdim_node *inserting) {
   t_uuid rid,iid;
   __vdim_node *inserted;
   long CMP;
   if(tree==NULL) {
      return(inserting);
   } else {
   iid = inserting->id;
   rid = tree->id;
   CMP = CompareGuids(iid,rid);
   if(CMP<0) {
      inserted = __vdim_insert((__vdim_node *)tree->lesser, inserting);
      if (inserted!=NULL){
        tree->lesser = (__vdim_node *)inserting;
      }
      return(NULL);
   } else  if(CMP>0) {
      inserted = __vdim_insert((__vdim_node *)tree->greater, inserting);
      if (inserted!=NULL){
        tree->greater = (__vdim_node *)inserting;
      }
      return(NULL);
   } else {
      free(inserting); //already exists
   }
   }
  return(NULL);
}

void __vdim_indexdimension (vector *v, dimension *dim, long position){
    __vdim_index *index;
    __vdim_node *node, *inserted;
    index = v->index;
    if (index!=NULL){
     node = (__vdim_node *)malloc(sizeof(__vdim_node));
     node->dim = dim;
     node->id = dim->dimensionid;
     node->position = position;
     node->lesser = NULL;
     node->greater = NULL;
      inserted = __vdim_insert(index->root,node);
      if (inserted!=NULL){index->root = inserted;}
    }
}

long __vdim_createvectorindex (vector *v){
 __vdim_index *index;
 long max,i,val;
 dimension *dim,*dims;
 v->index = (__vdim_index *)malloc(sizeof(__vdim_index));
 index = v->index;
 index->root = NULL;
 max = v->dimensioncount; 
 i=0;
 dims = v->dimensions;
 while (i<max){
    val = (long)dims + (long)(i * sizeof(dimension));
    dim = (dimension *)(val);
    __vdim_indexdimension(v,dim,i);
    i++;
 }
 return(max);
}

void __vdim_reindexvector(vector *v){
  __vdim_clearvectorindex (v);
  __vdim_createvectorindex (v);
}


__vdim_node *__vdim_recursefind(__vdim_node *tree, t_uuid id) {
   t_uuid rid ;
   long CMP;
   if(tree!=NULL){
   rid = tree->id;
   CMP = CompareGuids(id,rid);
   if(CMP==0){
     return(tree);
   } else {
     if(CMP<0){
      return(__vdim_recursefind((__vdim_node *)tree->lesser, id));
     } else  if(CMP>0) {
      return(__vdim_recursefind((__vdim_node *)tree->greater, id));
     }
   }
   }
  return(NULL); //if we get this far, we haven't found it.
}


dimension *__vdim_find_dimension (vector *v,t_uuid dimensionid){
   __vdim_node *found,*root;
   __vdim_index *index;
   index = v->index;
   if (index!=NULL){
   root = (__vdim_node *)(index->root);
   found =  __vdim_recursefind(root,dimensionid);
   if (found!=NULL){
   return (found->dim);
   } else {
   return (NULL);
   }
   } else return(NULL);
}

long __vdim_find (vector *v,t_uuid dimensionid){
   __vdim_node *found,*root;
   __vdim_index *index;
   index = v->index;
   if (index!=NULL){
   root = (__vdim_node *)(index->root);
   found =  __vdim_recursefind( root,dimensionid);
   if (found!=NULL){
   return (found->position);
   } else {
   return (-1);
   }
   } else return(-1);
}
