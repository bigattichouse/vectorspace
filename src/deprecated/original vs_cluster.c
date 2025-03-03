#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "vs_core.h"
#include "vs_list.h"
#include "vs_cursor.h"
#include "vs_cluster.h"


void mergevectors (vector *movablev,vector *staticv){
 float mvv,svv;
 dimension *dim;
 vs_value value;
 int i,max;
 t_uuid id;
 //Cycle thru and make sure our movable has all necessary dimensions.
 max = staticv->dimensioncount;
 for (i=0;i<max;i++){
  id = vs_getdimensionbyindex(staticv,i);
  dim = vs_getvalue(movablev,id);
  if (dim==NULL){
    value.floatvalue = 0;
    vs_setvalue (movablev,id,value);
  }
 }
 //now average them!
 max = movablev->dimensioncount;
 for (i=0;i<max;i++){
  id = vs_getdimensionbyindex(movablev,i);
  dim = vs_getvalue(movablev,id);
  mvv = dim->value.floatvalue;
  svv=0.00;
  dim = vs_getvalue(staticv,id);
  if (dim!=NULL){
    svv = dim->value.floatvalue;
  }
  value.floatvalue =(mvv+svv)/2;
  vs_setvalue (movablev,id,value);
 }
}

vs_cursor *vsc_removenoise (vs_cursor *source,float threshold, int cluster_minimum_size){
 vs_cursor *querycursor,*denoised;
 vector *query,*v;
 float dbcos,magq,dp,magd;
 int refcount;
 v=NULL;
 //1. query cluster with each vector at a strict threshold
 denoised = vsc_createcursor();
 querycursor = vsc_clonecursor(source);
// printf("Cloned: %d vectors\n",querycursor->count);
 vsc_reset(querycursor);
 while (!vsc_eof(querycursor)){
  query = vsc_readnext(querycursor);
  magq = vs_magnitude(query);
  refcount=0;
 vsc_reset(source);
 //printf("vector:  %f %f\n",vs_getvalue(query,vs_getdimensionbyindex(query,0))->value.floatvalue,vs_getvalue(query,vs_getdimensionbyindex(query,1))->value.floatvalue);
 while (!vsc_eof(source)){
  v = vsc_readnext(source);
  if (v!=NULL){
   magd = vs_relativity(v,query);
   dbcos = vs_relativequerycosine(query,v,&dp);
   //printf("compare:%f\n",dbcos);
   if (dbcos>threshold){
      refcount+=1;
   }
  }
 } // while we have references to check
 if (refcount>=cluster_minimum_size){
   vsc_replacevector(denoised,query);   //2. add vector to denoised has more than cluster_minimum_size related
 }
 } //while we have vectors to as queries
 vsc_destroycursor(&querycursor);
 return(denoised);
}


vector *vsc_findbestmatch(vs_cursor *source,vector *v,float threshold){
 float highscore;
 vector *best,*test;
 float dp,dbcos,magq,magd;
 int ref;
 highscore=-1;
 vsc_reset(source);
 best=NULL;
 magq = vs_magnitude(v);
 while (!vsc_eof(source)){
   test = vsc_readnext(source);
   ref = 0;
   ref = test->reference;
   magd = vs_relativity(test,v);
   dbcos = vs_relativequerycosine(v,test,&dp);
   printf("match: %d ? %d\n",dbcos,highscore);
   if (dbcos>highscore){
    if (ref==0){
       if(!IsEqualGuid(test->vectorid,v->vectorid))
        if (test->reference=0){
            best=test;
            highscore=dbcos;
           }
      }
   }
 }

 if (best!=NULL){
   best->reference=1;
 }
 return(best);
}

vs_cursor *vsc_recurseclusters (vs_cursor **source,float threshold,int cluster_minimum_size){
 vs_cursor *final,*search ;
 vector *v,*best;
 int startcount,endcount;
 search = vsc_clonecursor(*source);
 final = vsc_createcursor();
 vsc_reset(search);
 startcount = (*source)->count;
 while (!vsc_eof(search)){
   v = vsc_readnext(search);
   printf("test\n");
   best = vsc_findbestmatch(*source,v,threshold);
   if (best==NULL){
     v->reference=0;
     v->vectorid = newid();
     vsc_replacevector(final,v);
   } else {
     mergevectors(best,v);
     vsc_deletevector(*source,v->vectorid);
     vsc_replacevector(search,best);
     (*source)->readposition-=1; //next node would skip, cause we're pulling one out.
   }
 }
 vsc_destroycursor(&search);
 endcount = (*source)->count;
 vsc_destroycursor(source);
 if (startcount!=endcount){
   return(vsc_recurseclusters(&final,threshold,cluster_minimum_size));
 } else {
   return(final);
 }
}

vs_cursor *vsc_vectorcluster(vs_cursor *source,float threshold,int cluster_minimum_size){
  vs_cursor *base,*denoised,*final;
  base = vsc_clonecursor(source);
  denoised = vsc_removenoise(base,threshold,cluster_minimum_size);
  vsc_destroycursor(&base);
  final = vsc_recurseclusters(&denoised,threshold,cluster_minimum_size);
  return(final);
}



