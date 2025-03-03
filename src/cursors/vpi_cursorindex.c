#include <stdlib.h>
#include <stdio.h>
#include <math.h>      
#include <unistd.h>
#include "vs_core.h"
#include "vs_list.h"
#include "vs_cursor.h"
#include "vs_cursorindex.h"

//int counter;


__cdim_node *__cdim_createnode (){
  __cdim_node *node;
  node = (__cdim_node *)malloc(sizeof(__cdim_node));
  node->greater=NULL;
  node->lesser=NULL;
  node->vectors = vs_prepare_list();
//  counter++;
//  printf("creating #%d\r\n",counter);
  return(node);
}

void __cdim_destroynode (__cdim_node **node){
  __cdim_node *temp;
  __cdim_node *t2;
  if (node!=NULL){
   temp = *node; 
   if (temp!=NULL){
     t2 =  temp->greater;
     __cdim_destroynode(&t2);
     t2 =  temp->lesser;
     __cdim_destroynode(&t2);
     vs_clean_list(temp->vectors);
     vs_destroy_list(&temp->vectors);
     free(*node);
//     counter--;
  //   printf("destroying #%d\r\n",counter);
   }
  }
}

__cdim_node *__cdim_recurse_find  (__cdim_node *parent,__cdim_node *node, t_uuid dimensionid){
  int CMP;
  if (node!=NULL){
    CMP = CompareGuids(dimensionid,node->dimensionid);
    if (CMP<0){
      return __cdim_recurse_find (node,node->lesser,dimensionid);
    }
    if (CMP>0){
      return __cdim_recurse_find (node,node->greater,dimensionid);
    }
    if (CMP==0){return node; }
  }
  return NULL;
}



__cdim_node *__cdim_recurse_find_insert  (__cdim_node *node, t_uuid dimensionid,__cdim_node *inserting){
  int CMP;
  if (node!=NULL){
    CMP = CompareGuids(dimensionid,node->dimensionid);
    if (CMP<0){
      if (node->lesser==NULL){
        node->lesser=inserting;
      } else {
        return __cdim_recurse_find_insert(node->lesser,dimensionid,inserting);
      }
    }
    if (CMP>0){
      if (node->greater==NULL){
        node->greater=inserting;
      } else {
        return __cdim_recurse_find_insert(node->greater,dimensionid,inserting);
      }
    }
    if (CMP==0){return node; }
  }
  return inserting;
}



__cdim_node *__cdim_find_dimension_node (__cdim_node *node,t_uuid dimensionid){
   __cdim_node *found,*result;
   int CMP;
   if (node!=NULL){
     found = __cdim_recurse_find(NULL,node,dimensionid);
     if (found!=NULL){
     CMP = CompareGuids(dimensionid,found->dimensionid);
     if (CMP!=0){
         result = __cdim_createnode();
         result->dimensionid = dimensionid;
         if (CMP>0){found->greater=node;}
         if (CMP<0){found->lesser=node;}
     } else {result=found;}
    } else {
      result = __cdim_createnode();
      result->dimensionid = dimensionid;
      __cdim_recurse_find_insert(node,dimensionid,result);
    }
   } else {
     result = __cdim_createnode();
     result->dimensionid = dimensionid;
   }
   return result;
}


long __cdim_insertvector (__cdim_index *index, vector *v){
  __cdim_node *found;
 long max,i,val;
 int a;
 dimension *dim,*dims;
 max = v->dimensioncount;
 i=0; a=0;
 dims = v->dimensions;
 while (i<max){
    val = (long)dims + (long)(i * sizeof(dimension));
    dim = (dimension *)(val);
    found = __cdim_find_dimension_node(index->root,dim->dimensionid);
    if (index->root==NULL){index->root = found;}
    if(vs_list_find(found->vectors,v->vectorid)==NULL){vs_list_add(found->vectors,v,v->vectorid); a=i; }
    i++;
 }
 return(a);
}



long __cdim_deletevector (__cdim_index *index, vector *v){
  __cdim_node *found;
 long max,i,val; 
 dimension *dim,*dims;
 max = v->dimensioncount;
 i=0;  
 dims = v->dimensions;
 while (i<max){
    val = (long)dims + (long)(i * sizeof(dimension));
    dim = (dimension *)(val);
    found = __cdim_find_dimension_node(index->root,dim->dimensionid);
    vs_list_erase(found->vectors,v->vectorid);
    i++;
 }
 return(1);
}


__cdim_index *__cdim_createcursorindex (vs_cursor *cursor){
 vector *v;
 __cdim_index *index;
// counter=0;
 index = (__cdim_index *)malloc(sizeof(__cdim_index));
 index->root=NULL;
 vsc_reset(cursor);
 while (vsc_eof(cursor)==0){
   v = vsc_readnext(cursor);
   __cdim_insertvector(index,v);
 }
 return index;
}


void __cdim_destroyindex(__cdim_index **index){
  __cdim_index *temp_index;
  __cdim_node *root;
  temp_index = *index;
  root= temp_index->root;
  __cdim_destroynode(&root);
  free(*index)    ;
}



/*

ROWSET read out - keep a vslist (null pointers, just id's... ), if we've already done a vector, then skip !  hah!
one pass, yet we still do distinct.

*/

vs_list *__cdim_query(vs_cursor *c, vector *v){
  __cdim_node *found;
  __cdim_index *index;
  vs_list *result;
 long max,i,val;
 dimension *dim,*dims;
 found = NULL;
 if (c->index!=NULL){
    index = (__cdim_index *)c->index;
    max = v->dimensioncount;
    i=0;
    result = vs_prepare_list(); //indexed.
    dims = v->dimensions;
    while (i<max){
     val = (long)dims + (long)(i * sizeof(dimension));
     dim = (dimension *)(val);
     found = __cdim_find_dimension_node(index->root,dim->dimensionid);
     if (found!=NULL){
        vs_list_copy (found->vectors,result);
     }
     i++;
    }
 } else {
   result = NULL;
 }
 return(result);
}
