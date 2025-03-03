#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "vs_core.h"
#include "vs_list.h"
#include "vs_cursor.h"
#include "vs_rowset.h"
#include "vs_cluster.h"
#include "vs_normalize.h"

long vsc_normalize_hasdimension (vector *v,t_uuid dimensionid){
    dimension *dim;
    dim = vs_getvalue( v,dimensionid);
    if (dim!=NULL){
     return 1;
    } else return 0;
}


long vsc_normalize_incriment_dimension (vector *v,t_uuid dimensionid){
    dimension *dim;
    vs_value val;
    dim = vs_getvalue(v,dimensionid);
    if (dim!=NULL){
          val = dim->value;
          val.floatvalue += 1;
          vs_setvalue (v,dimensionid,val);
          return(1);
    } else {
       val.floatvalue  = 1;
       vs_setvalue (v,dimensionid,val);
       return(1);
    }
    return(0);
}


vector *vsc_normalize_vector (vector *v,vector *norm){
    long max,i,dimval ;
    dimension *dim,*dims;
    vector *result;
    t_uuid id;
    id = NullGuid();
    max = v->dimensioncount;
    dims= v->dimensions;
    result = vs_createvector(v->vectorid,0);
    for (i=0;i<max;i++){
       dimval = (long)dims + (long)(i * sizeof(dimension));
       dim = (dimension *)(dimval);
       if (vsc_normalize_hasdimension(norm,id)){
            vs_setvalue(result,id,dim->value);
       }
    }
    if(result->dimensioncount==0){vs_destroyvector(&result); result=NULL;}
    return(result);
}

vs_cursor *vsc_normalize (vs_cursor *source){
  vs_cursor *normalized;
  vs_rowset *rowset;
  dimension *dims,*dim ;
  vector *v, *view_v, *compare, *final;
  t_uuid id;
  long max,i,dimval,c;
  normalized = vsc_createcursor();

		printf( "Normalize: %ld vectors\r\n",source->count); 
  vsc_reset (source) ;
 // view = vsc_create_view(source);
  printf("Normalizing: %ld vectors\r\n",source->count);
  c=0;
  while ( !vsc_eof (source) ){
    c++;
    v = vsc_readnext(source);

  // printf("   Vector %ld of %ld : has %ld ",c,source->count, v->dimensioncount);


    compare = vs_createvector(NullGuid(),0);
    
    max = v->dimensioncount;
    dims= v->dimensions;

    rowset = vsr_createrowset(source,v);
    vsr_reset(rowset);
    while (!vsr_eof (rowset)){
      view_v = vsr_readnext(rowset);
      if (!IsEqualGuid(v->vectorid,view_v->vectorid)){
          for (i=0;i<max;i++){
            dimval = (long)dims + (long)(i * sizeof(dimension));
            dim = (dimension *)(dimval);
            id = dim->dimensionid;
            if (vsc_normalize_hasdimension(view_v,id)){
              vsc_normalize_incriment_dimension(compare,id);
            }
          }
      }
    }
    //now we have compare, holding some dimensions with values>1 matching our vector.
    //we need to clean out any dimensions in our vector that don't have corresponding values
    final = vsc_normalize_vector(v,compare);
   //  printf("  ending at %ld \r\n",compare->dimensioncount);
    if(final!=NULL){
      vsc_rawloadvector(normalized,final);
    }
    vsr_destroyrowset(&rowset);
    vs_destroyvector(&compare);
  }
  return(normalized);
}





