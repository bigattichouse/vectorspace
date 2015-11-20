#include <stdio.h>
#include <sys/io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include "vs_core.h"
#include "vs_list.h"
#include "vs_queue.h"
#include "vs_cache.h"
#include "vs_cursor.h" 
#include "vs_query.h"
#include "vs_cluster.h"
#include "vs_thesaurus.h"
#include "vs_modes.h"
#include "vs_persist.h" 
#include "vs_disc_index.h"
#include "vs_disc_data.h"
#include "vs_disc_dimensions.h"

//int counter;


__ddim_node *__ddim_createnode (){
  __ddim_node *node;
  node = (__ddim_node *)malloc(sizeof(__ddim_node));
  node->greater=NULL;
  node->lesser=NULL;
  node->vectors = vs_prepare_list();
//  counter++;
//  printf("creating #%d\r\n",counter);
  return(node);
}

void __ddim_destroynode (__ddim_node **node){
  __ddim_node *temp;
  __ddim_node *t2;
  if (node!=NULL){
   temp = *node; 
   if (temp!=NULL){
     t2 =  temp->greater;
     __ddim_destroynode(&t2);
     t2 =  temp->lesser;
     __ddim_destroynode(&t2);
     vs_clean_list(temp->vectors);
     vs_destroy_list(&temp->vectors);
     free(*node);
//     counter--;
  //   printf("destroying #%d\r\n",counter);
   }
  }
}

__ddim_node *__ddim_recurse_find  (__ddim_node *parent,__ddim_node *node, t_uuid dimensionid){
  int CMP;
  if (node!=NULL){
    CMP = CompareGuids(dimensionid,node->dimensionid);
    if (CMP<0){
      return __ddim_recurse_find (node,node->lesser,dimensionid);
    }
    if (CMP>0){
      return __ddim_recurse_find (node,node->greater,dimensionid);
    }
    if (CMP==0){return node; }
  }
  return NULL;
}



__ddim_node *__ddim_recurse_find_insert  (__ddim_node *node, t_uuid dimensionid,__ddim_node *inserting){
  int CMP;
  if (node!=NULL){
    CMP = CompareGuids(dimensionid,node->dimensionid);
    if (CMP<0){
      if (node->lesser==NULL){
        node->lesser=inserting;
      } else {
        return __ddim_recurse_find_insert(node->lesser,dimensionid,inserting);
      }
    }
    if (CMP>0){
      if (node->greater==NULL){
        node->greater=inserting;
      } else {
        return __ddim_recurse_find_insert(node->greater,dimensionid,inserting);
      }
    }
    if (CMP==0){return node; }
  }
  return inserting;
}



__ddim_node *__ddim_find_dimension_node (__ddim_node *node,t_uuid dimensionid){
   __ddim_node *found,*result;
   int CMP;
   if (node!=NULL){
     found = __ddim_recurse_find(NULL,node,dimensionid);
     if (found!=NULL){
     CMP = CompareGuids(dimensionid,found->dimensionid);
     if (CMP!=0){
         result = __ddim_createnode();
         result->dimensionid = dimensionid;
         if (CMP>0){found->greater=node;}
         if (CMP<0){found->lesser=node;}
     } else {result=found;}
    } else {
      result = __ddim_createnode();
      result->dimensionid = dimensionid;
      __ddim_recurse_find_insert(node,dimensionid,result);
    }
   } else {
     result = __ddim_createnode();
     result->dimensionid = dimensionid;
   }
   return result;
}


int __ddim_insertvector (__vdd_dimensions *index, vector *v){
  __ddim_node *found;
  int max,i,val;
 int a;
 dimension *dim,*dims;
 max = v->dimensioncount;
 i=0; a=0;
 dims = v->dimensions;
 while (i<max){
    val = (int)dims + (int)(i * sizeof(dimension));
    dim = (dimension *)(val);
    found = __ddim_find_dimension_node(index->root,dim->dimensionid);
    if (index->root==NULL){index->root = found;}
    if(vs_list_find(found->vectors,v->vectorid)==NULL){vs_list_add(found->vectors,NULL,v->vectorid); a=i; }
    i++;
 }
 return(a);
}


int __ddim_insertvectorid (__vdd_dimensions *index, t_uuid dimensionid, t_uuid vectorid){
 __ddim_node *found;
 found = __ddim_find_dimension_node(index->root,dimensionid);
 if (found!=NULL){
   if (index->root==NULL){index->root = found;}
   if(vs_list_find(found->vectors,vectorid)==NULL){vs_list_add(found->vectors,NULL,vectorid);  }
   return 1;
 }
 return 0;
}



int __ddim_deletevector (__vdd_dimensions *index, vector *v){
  __ddim_node *found;
  int max,i,val;
 int a;
 dimension *dim,*dims;
 max = v->dimensioncount;
 i=0; a=0;
 dims = v->dimensions;
 while (i<max){
    val = (int)dims + (int)(i * sizeof(dimension));
    dim = (dimension *)(val);
    found = __ddim_find_dimension_node(index->root,dim->dimensionid);
    vs_list_erase(found->vectors,v->vectorid);
    i++;
 }
 return(1);
}


__vdd_dimensions *__vdd_create_disk_dimensions(){
  __vdd_dimensions *index;
 index = (__vdd_dimensions *)malloc(sizeof(__vdd_dimensions));
 index->root=NULL;
 return index;
}

void __ddim_destroyindex(__vdd_dimensions **index){
  __vdd_dimensions *temp_index;
  __ddim_node *root;
  temp_index = *index;
  root= temp_index->root;
  __ddim_destroynode(&root);
  free(*index)    ;
}


vs_list *__ddim_query(__vdd_dimensions *index, vector *v){
  __ddim_node *found;
  vs_list *result;
 int max,i,val;
 dimension *dim,*dims;
 found = NULL; 
    max = v->dimensioncount;
    i=0;
    result = vs_prepare_list(); //indexed.
    dims = v->dimensions;
    while (i<max){
     val = (int)dims + (int)(i * sizeof(dimension));
     dim = (dimension *)(val);
     found = __ddim_find_dimension_node(index->root,dim->dimensionid);
     if (found!=NULL){
        vs_list_copy (found->vectors,result);
     }
     i++;
    }

 return(result);
}
