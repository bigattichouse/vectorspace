#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "vs_core.h"
#include "vs_dimensionindex.h"

const long _VS_MAX_DIMS = 10000 ;

t_uuid newid(){
 t_uuid result;
 CreateGUID(&result);
 return(result);
}

vector *vs_createvector (t_uuid vectorid,long dimensioncount){
   long i,dsz, val;
   dimension *dim,*dims;
   vector *vp;
   vp = malloc(sizeof(vector));
   vp->magnitude=0;
   vp->reference=0;
   vp->index=NULL;
   vp->vectorid=vectorid;
   vp->dimensioncount=dimensioncount;
   if ((dims=malloc((dimensioncount+1) * sizeof(dimension))) ==NULL) { //printf("FAIL MALLOC\n");
   }
   vp->dimensions=dims;
   //printf("dims=%d\n",dimensioncount);
   dsz = sizeof(dimension);
   for (i=0;i<dimensioncount;i++){
    val = (long)dims + (long)(i * dsz);
    //printf(">%d\n",i);
    dim = (dimension *)(val);
    dim->dimensionid = NullGuid();
    dim->vectorid=vectorid; dim->dimensiontype=0;
    dim->value=0.0;
    }
   return(vp);
}




inline long vs_rawfinddimension (vector *v, t_uuid dimensionid){
 long max,i,idx,val;
 dimension *dim,*dims;
 t_uuid d;
 max = v->dimensioncount;
 idx = -1;
 i=0;
 dims = v->dimensions;
 while ((i<max) && (idx==-1)){
    val = (long)dims + (long)(i * sizeof(dimension));
    dim = (dimension *)(val);
    if (dim!=NULL){
      d = dim->dimensionid;
      if (IsEqualGuid(d,dimensionid)==1){idx=i; i=max; return idx;}
    }
    i++;
 }
 return(idx);
}

inline long vs_finddimension (vector *v, t_uuid dimensionid){
 if (v->index!=NULL){
   return(__vdim_find(v,dimensionid));
  } else {
   return(vs_rawfinddimension(v,dimensionid));
  }
}

void vs_clearindex (vector *v){
 if (v->index!=NULL){
   __vdim_clearvectorindex(v);
  }
}

void vs_reindex (vector *v){
 vs_clearindex(v);
  __vdim_createvectorindex(v);
}



long vs_findemptydimension (vector *v, t_uuid dimensionid){
 long max,i,idx,val;
 dimension *dim,*dims;
 max = v->dimensioncount;
 dims= v->dimensions;
 idx = -1;
 for (i=0;i<max;i++){
    val = (long)dims + (long)(i * sizeof(dimension));
    dim = (dimension *)(val);
    if (dim->dimensiontype==0){
        idx=i; i=max; return(idx);
    }
 }
 return(idx);
}

long vs_setvalue (vector *v, t_uuid dimensionid, float value){
 long result,idx,blankidx,val;
 dimension *dim,*dims,*redims;
 result =0;
 idx = vs_finddimension(v,dimensionid);

 dims = v->dimensions;
 if (idx==-1){
    blankidx = vs_findemptydimension(v,dimensionid);
    if (blankidx==-1){
      /*1. realloc + 1*/
      if ((redims=(dimension *)malloc((v->dimensioncount+1) * sizeof(dimension)))!=NULL){
        memcpy(redims,dims,(v->dimensioncount) * sizeof(dimension));
        free((void *)dims);
        v->dimensioncount=v->dimensioncount+1;
        v->dimensions=redims;
        val = (long)redims + (long)((v->dimensioncount-1) * sizeof(dimension));
        dim = (dimension *)(val);
        dim->dimensionid=dimensionid;
        dim->dimensiontype=1;
        dim->value = value;
        result=1;
        if (v->index!=NULL){ __vdim_indexdimension(v,dim,(v->dimensioncount-1));}//new, so index it!
      } else {
      /*failed*/
      result=-1;
      }
    } else {
       val = (long)dims + (long)(sizeof(dimension) * blankidx);
       dim = (dimension *)(val);
 /*     printf("NEW:%s=%f\n",GuidToString(dimensionid),value);*/
      dim->dimensionid=dimensionid;
      dim->dimensiontype=1;
      dim->value = value;
      if (v->index!=NULL){ __vdim_indexdimension(v,dim,blankidx);} //new, so index it!
      result=1;
    }
  } else {
    val = (long)dims + (long)(idx * sizeof(dimension));
    dim = (dimension *)(val);
/*      printf("EXIST:%s=%f\n",GuidToString(dimensionid),value);*/
      dim->dimensiontype=1;
      dim->value = value;
      result=1;
  }

 return(result);
}

void vs_quickset (vector *v, t_uuid dimensionid, float floatvalue){
 vs_setvalue(v, dimensionid, floatvalue);
}


long vs_appendvalue (vector *v, t_uuid dimensionid, float value){
 long result,val;
 dimension *dim,*dims,*redims;
 result = 0;
 dims = v->dimensions;
      /*1. realloc + 1*/
      if ((redims=(dimension *)malloc((v->dimensioncount+1) * sizeof(dimension)))!=NULL){
        if (dims!=NULL){
          memcpy(redims,dims,(v->dimensioncount) * sizeof(dimension));
          free((void *)dims);
        }
        v->dimensioncount=v->dimensioncount+1;
        v->dimensions=redims;
        val = (long)redims + (long)((v->dimensioncount-1) * sizeof(dimension));
        dim = (dimension *)(val);
        dim->dimensionid=dimensionid;
        dim->dimensiontype=1;
        dim->value = value;
        result=1;
        if (v->index!=NULL){__vdim_indexdimension(v,dim,(v->dimensioncount-1));}//new, so index it!
      } else {
      /*failed*/
      result=-1;
      }

 return(result);
}


long vs_setvaluebyindex (vector *v, long idx, t_uuid dimensionid, float value){
 long result,val;
 dimension *dim,*dims;
 result = 0;
 dims = v->dimensions;
 val = (long)dims + (long)(idx * sizeof(dimension));
 dim = (dimension *)(val);

  dim->dimensiontype=1;
  dim->value = value;
  result=idx;


 return(result);
}

t_uuid vs_getdimensionbyindex (vector *v,long idx){
 long val;
 dimension *dim,*dims ;
 dims = v->dimensions;
 val = (long)dims + (long)(idx * sizeof(dimension));
 dim = (dimension *)(val);
 return(dim->dimensionid);
}

dimension *vs_getvalue(vector *v, t_uuid dimensionid){ //returns long=0 if value isn't set
 long idx,val;
 dimension *dims,*result;
 idx = vs_finddimension(v,dimensionid);
 dims = v->dimensions;
 result = NULL;
 if (idx!=-1){
    val = (long)dims + (long)(idx * sizeof(dimension));
    result = (dimension *)(val);
 }
 return(result);
}

long vs_clone (vector *src,vector *dst){
 long max,i ;
 dimension  dim;
 dst->vectorid=src->vectorid;
 max = src->dimensioncount;
 dst->reference = src->reference;
 dst->magnitude = src->magnitude;
 if (src->index!=NULL){__vdim_createvectorindex(dst);}
 for (i=0;i<max;i++){
//    val = (long)dims + (long)(i * sizeof(dimension));
    dim = src->dimensions[i];
//    dim = (dimension *)(val);
    vs_setvalue(dst, dim.dimensionid, dim.value);
  }
  return(1);
}

long vs_destroyvector (vector **v){
 vector *vv ;
 if ((v!=NULL)){
 vv=*v;
 if  (vv!=NULL){
   if (vv->index!=NULL){ __vdim_clearvectorindex(vv);}
   if (vv->dimensions!=NULL){ free((void *)vv->dimensions);}
   free(*v);
 }
 }
 v=NULL;
 return(1);
}

float vs_magnitude(vector *v){
 long max,i;
 float vv,result;
 dimension dim;

 if (v!=NULL){
 max = v->dimensioncount;
 result=0;
 for (i=0;i<max;i++){
   // val = (long)dims + (long)(i * sizeof(dimension));
  //  dim = (dimension *)(val);
  dim = v->dimensions[i];
  vv = dim.value;
  result=result + (vv * vv);
 }
 result = (sqrtf)(result);
 v->magnitude=result;
 } else result=0;
 return(result);
}


float vs_dotproduct(vector *a,vector *b){
 long max,i,val;
 float v1,v2,result;
 dimension *dim,*dims,*dim2;
 t_uuid dimensionid;
 max = a->dimensioncount;
 dims = a->dimensions;
 result=0.0;
 for (i=0;i<max;i++){
   v1=0.0; v2=0.0;
    val = (long)dims + (long)(i * sizeof(dimension));
    dim = (dimension *)(val);
   v1 = dim->value;
   dimensionid=dim->dimensionid;
   if((dim2 = vs_getvalue(b,dimensionid))){
    v2 = dim2->value;
   } else {v2=0;}
   result=result + (v1 * v2);
 }
 return(result);
}


float vs_cosine(vector *a,vector *b){
 float m1,m2,mt,dp,result;
 m1 = vs_magnitude(a);
 m2 = vs_magnitude(b);
 mt=m1*m2;
 if (mt!=0){
   dp=vs_dotproduct(a,b);
   result= dp/(mt);
 }else{result=0;}
 return(result);
}

float vs_relativemagnitude(vector *v,vector *ref){
 long max,i,val;
 float vv,result;
 t_uuid dimensionid;
 dimension *dim,*dims, *vdim;
 max = ref->dimensioncount;
 dims = ref->dimensions;
 result=0;
 for (i=0;i<max;i++){
    val = (long)dims + (long)(i * sizeof(dimension));
    dim = (dimension *)(val);
    dimensionid=dim->dimensionid;
    vdim = vs_getvalue (v,dimensionid);
   if (vdim!=NULL){
      vv = vdim->value;
      result=result + (vv * vv);
   }
 }
 result = sqrt(result);
 return(result);
}

float vs_relativity(vector *v,vector *ref){
 long max,i,val;
 float vv,dv,result;
 dimension *dim,*dims,*vdim;
 max = ref->dimensioncount;
 dims = ref->dimensions;
 result=0;
 for (i=0;i<max;i++){
    val = (long)dims + (long)(i * sizeof(dimension));
    dim = (dimension *)(val);
    if (dim!=NULL){
    vdim = vs_getvalue (v,dim->dimensionid);
    if(vdim!=NULL){
      vv = vdim->value;
      dv = dim->value;
      if(vv * dv>0){result+=1;}
    }
    }
 }
 return(result);
}


float vs_querycosine(vector *a,vector *b,float *dp){
 float m1,m2,mt,ldp,result;
 m1 = a->magnitude;
 m2 = b->magnitude;
 mt=m1*m2;
 if (mt!=0){
   ldp=vs_dotproduct(a,b);
   result= ldp/(mt);
   *dp = ldp;
 }else{result=0;}
 return(result);
}

float vs_relativequerycosine(vector *a,vector *b,float *dp){
 float m1,m2,mt,ldp,result,arel,brel;
 m1 = vs_relativemagnitude(a,b);
 m2 = vs_relativemagnitude(b,a);
 mt=m1*m2;
 if (mt!=0){
   ldp=vs_dotproduct(a,b);
   result= ldp/(mt);
   *dp = ldp;
   if (a->dimensioncount>0){
    arel = a->dimensioncount;
    brel = vs_relativity(a,b);
    result = result * (brel / arel); //more matching dims gives better score.
   }
 }else{result=0;}
 return(result);
}

void vs_printvector( vector *v){

  char *did;
  long count,i;

  dimension *dim;
  t_uuid id;

    did = HashToString(v->vectorid);
    printf(  "V %s %ld\r\n",did,v->dimensioncount);
    free(did);
    count=v->dimensioncount;
    for (i=0;i<count;i++){
       id = vs_getdimensionbyindex(v,i);
       dim = vs_getvalue(v,id);
       did = HashToString(id);
       printf( "D %s %f\r\n",did,dim->value);
       free(did);
    }

}


void vs_mergevectorsweightedaveraged (vector *movablev,vector *staticv, float weight, float avg){
 float mvv,svv;
 dimension *dim;
 float value;
 long i,max;
 t_uuid id;
 //Cycle thru and make sure our movable has all necessary dimensions.
 max = staticv->dimensioncount;
 for (i=0;i<max;i++){
  id = vs_getdimensionbyindex(staticv,i);
  dim = vs_getvalue(movablev,id);
  if (dim==NULL){
    value = 0;
    vs_setvalue (movablev,id,value);
  }
 }
  //printf("Merge dimensions:\n");
 //now average them!
 max = movablev->dimensioncount;
 for (i=0;i<max;i++){
  id = vs_getdimensionbyindex(movablev,i);
  dim = vs_getvalue(movablev,id);
  mvv = 0.00;
  if (dim!=NULL){
     mvv = dim->value;
  }
  svv=0.00;
  dim = vs_getvalue(staticv,id);
  if (dim!=NULL){
    svv = dim->value * weight;
  }
  value =(mvv+svv)/avg;
  //printf("   %ld: %f + (%f * %f) = %f\n",i,svv,mvv,weight, value);
  vs_setvalue (movablev,id,value);
 }
}


void vs_mergevectorsweighted (vector *movablev,vector *staticv, float weight){ 
 vs_mergevectorsweightedaveraged (movablev,staticv, 1.000000, 2.0000);
}


void vs_mergevectors (vector *movablev,vector *staticv){
 vs_mergevectorsweighted (movablev,staticv, 1.000000);
}


void vs_sumvectorsweighted (vector *movablev,vector *staticv, float weight){
 float mvv,svv;
 dimension *dim;
 float value;
 long i,max;
 t_uuid id;
 //Cycle thru and make sure our movable has all necessary dimensions.
 max = staticv->dimensioncount;
 for (i=0;i<max;i++){
  id = vs_getdimensionbyindex(staticv,i);
  dim = vs_getvalue(movablev,id);
  if (dim==NULL){
    value = 0;
    vs_setvalue (movablev,id,value);
  }
 }
 //now average them!
 max = movablev->dimensioncount;
 for (i=0;i<max;i++){
  id = vs_getdimensionbyindex(movablev,i);
  dim = vs_getvalue(movablev,id);
  mvv = 0.00;
  if (dim!=NULL){
  mvv = dim->value;
  }
  svv=0.00;
  dim = vs_getvalue(staticv,id);
  if (dim!=NULL){
    svv = dim->value * weight;
  }
  value =mvv+svv;
  vs_setvalue (movablev,id,value);
 }
}

void vs_sumvectors (vector *movablev,vector *staticv){
 vs_sumvectorsweighted (movablev,staticv, 1.000000);
}

void vs_diffvectorsweighted (vector *movablev,vector *staticv, float weight){
 float mvv,svv;
 dimension *dim;
 float value;
 long i,max;
 t_uuid id;
 //Cycle thru and make sure our movable has all necessary dimensions.
 max = staticv->dimensioncount;
 for (i=0;i<max;i++){
  id = vs_getdimensionbyindex(staticv,i);
  dim = vs_getvalue(movablev,id);
  if (dim==NULL){
    value = 0;
    vs_setvalue (movablev,id,value);
  }
 }
 
 max = movablev->dimensioncount;
 for (i=0;i<max;i++){
  id = vs_getdimensionbyindex(movablev,i);
  dim = vs_getvalue(movablev,id);
  mvv = 0.00;
  if (dim!=NULL){
  mvv = dim->value;
  }
  svv=0.00;
  dim = vs_getvalue(staticv,id);
  if (dim!=NULL){
    svv = dim->value * weight;
  }
  value =svv-mvv;
  vs_setvalue (movablev,id,value);
 }
}

void vs_diffvectors (vector *movablev,vector *staticv){
 vs_sumvectorsweighted (movablev,staticv, 1.000000);
}



void vs_modifyvector( vector *v, float modifier){
  long count,i;
  float val;
  dimension *dim;
  float value;
  t_uuid id;
    count=v->dimensioncount;
    for (i=0;i<count;i++){
       id = vs_getdimensionbyindex(v,i);
       dim = vs_getvalue(v,id);
       val = dim->value;
       value = val * modifier;
       //printf(" %f * %f = %f\n",val,modifier,value);
       vs_setvalue (v,id,value);
    }
}

void vs_modifydimension( vector *v, t_uuid id, float modifier){
  float val;
  dimension *dim; 
  float value;
  dim = vs_getvalue(v,id);
  if (dim!=NULL){  
       val = dim->value;
       value = val * modifier;
       vs_setvalue (v,id,value);
  }
}

dimension *vs_lowest_dimension( vector *v){
  char *did;
  long count,i;
  float bestScore = 99999; 
  dimension *dim, *bestDim;
  t_uuid id;
    bestDim=NULL;
    count=v->dimensioncount;
    for (i=0;i<count;i++){
       id = vs_getdimensionbyindex(v,i);
       dim = vs_getvalue(v,id);
       did = HashToString(id);
       if (i==0){
         bestScore=dim->value;
       }
       if (dim->value<=bestScore){
         bestScore=dim->value;
         bestDim=dim;
       };
       free(did);
    }
   return bestDim;
}


dimension *vs_highest_dimension( vector *v){
  char *did;
  long count,i;
  float bestScore = -99999; 
  dimension *dim, *bestDim;
  t_uuid id;
    bestDim=NULL;
    count=v->dimensioncount;
    for (i=0;i<count;i++){
       id = vs_getdimensionbyindex(v,i);
       dim = vs_getvalue(v,id);
       did = HashToString(id);
       if (i==0){
         bestScore=dim->value;
       }
       if (dim->value>=bestScore){
         bestScore=dim->value;
         bestDim=dim;
       };
       free(did);
    }
   return bestDim;
}

//TODO: This was codellama70b, which did a pretty good job.
vector *vs_gravity(vector *a,vector* b, float gravity, float massMultiplier){
   //if you want to ignore mass, use 0 massMultiplier
    long count, i;
    dimension *dimA, *dimB;
    t_uuid id;
    float valueA, valueB;

    // If both vectors have the same number of dimensions...
    if (a->dimensioncount == b->dimensioncount) {
        count = a->dimensioncount;

        for (i = 0; i < count; i++) {
            id = vs_getdimensionbyindex(a, i);
            dimA = vs_getvalue(a, id);
            dimB = vs_getvalue(b, id);
            
            // Calculate the distance between the two vectors on each dimension...
            float distX = (dimA->value - dimB->value);
            valueA = 1 * dimA->value;
            valueB = gravity * massMultiplier * distX;
            
            // Apply the calculated distance to each vector...
            vs_setvalue(a, id, valueA);
            vs_setvalue(b, id, valueB);
        }
    }
    
    return b;
}

/*
vector *vs_gravity(vector *a, vector *b, float gravity, float massMultiplier) {
    // Calculate the distance between vectors A and B
    float dx = a->x - b->x;
    float dy = a->y - b->y;
    float dz = a->z - b->z;
    float distanceSquared = dx * dx + dy * dy + dz * dz;
    float forceMagnitude = (gravity * massMultiplier) / distanceSquared;

    // Calculate the force components on B due to A
    float fx = forceMagnitude * dx;
    float fy = forceMagnitude * dy;
    float fz = forceMagnitude * dz;

    // Update the velocity of B based on the applied force
    b->vx += fx / b->mass;
    b->vy += fy / b->mass;
    b->vz += fz / b->mass;

    return b;
}
*/
