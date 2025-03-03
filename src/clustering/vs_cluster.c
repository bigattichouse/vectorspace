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


void vsc_clearreferences (vs_cursor *source){
 vector *test;
 vsc_reset(source);
 while (!vsc_eof(source)){
   test = vsc_readnext(source);
   test->reference = 0;
 }
}

/*
creates a metavector, which shows other vectors in the cursor as dimensions of THIS vector... those that pass the threshold anyway.
additionally, only those that pass the min_cluster_size are added to the meta vector.
We also manage this with a single pass.
*/

vs_cursor *vsc_metacursor (vs_cursor *source,int segment,int segmentsize,float threshold,int cluster_minimum_size){
  vs_cursor *metacursor,*view;
  int i;
  vector *v,*view_v,*meta_v;
  vs_value val;
  t_uuid id;
  float score,dp;
 /*
 Segment/Segment size is a way to break down the problem into handlable pieces...
 So we can say "I want to start with Segment 2 with size of 1000 .. so I start with 2000 and go to 2999 
 This way the controlling program can show status and progress.. happy!
 */
  vsc_clearreferences(source);
  vsc_reset (source) ;
  metacursor = vsc_createcursor();
  view = vsc_create_view(source);
  i=0;
  while ((i<(segment * segmentsize)-1) && (!vsc_eof (source))){     
    v = vsc_readnext(source);
    i++;
  }
  while ((!vsc_eof (source)) && (i<(segment+1) * segmentsize)){
    i++;
    v = vsc_readnext(source);
    id = v->vectorid;
    meta_v = vs_createvector( id,0);
    vsc_reset(view);
    while (!vsc_eof (view)){
       view_v = vsc_readnext(view);
       score = vs_relativequerycosine(v,view_v,&dp);
        if (score>=threshold){
            val.floatvalue = score;
            vs_appendvalue(meta_v,view_v->vectorid,val);
        }
    }
    if(meta_v->dimensioncount>=cluster_minimum_size){
       vsc_rawinsertvector(metacursor,meta_v);
    } else {
       vs_destroyvector(&meta_v);
    } 
  }
 vsc_destroycursor(&view);
 return(metacursor);
}

/*
Now we can make a few passes to merge any metavectors that might be similar, minimizing overlap.
*/
vs_cursor *vsc_consolidate(vs_cursor *metacursor,float threshold,int cluster_minimum_size){
 int changes,i,max;
 float score,dp;
 vs_cursor *view,*meta2;
 vector *v,*best,*view_v;
 changes=1;
  vsc_clearreferences(metacursor);
 meta2 = vsc_createcursor();
while(changes!=0){
   changes=0;
   vsc_reset(metacursor); i =0;
   view = vsc_create_view(metacursor);
   while (!vsc_eof (metacursor)){
    i++;
    v = vsc_readnext(metacursor);
    vs_printvector(v);
    max=0; best = NULL;
   if (v->reference!=-1){   
    vsc_reset(view);
    while (!vsc_eof (view)){
       dp=0;
       view_v = vsc_readnext(view);
      if (!IsEqualGuid(view_v->vectorid,v->vectorid)){
       if (view_v->reference!=-1){
       score = vs_relativequerycosine(v,view_v,&dp);
        if ((score>max) && (score>=threshold)){
            best = v;
            max = score;
        } else {
          view_v->reference=-1;
        }
       }
       }
      }
    }
    if(best!=NULL){
      if (!IsEqualGuid(best->vectorid,v->vectorid)){
        if (v->reference!=-1){
          if (best->dimensioncount>v->dimensioncount){
            vsc_rawloadvector(meta2,best);
            v->reference=-1;
            changes+=1;
          } else {
            vsc_rawloadvector(meta2,v);
            best->reference=-1;
            changes+=1;
          }
        }
      }
      }

    }
   if (changes>0){
       vsc_destroycursor(&metacursor);
       metacursor = meta2;
       meta2 = vsc_createcursor();
       vsc_destroycursor(&view);
       view = vsc_create_view(metacursor);
   } else {vsc_destroycursor(&view);}
  }
 return(metacursor);
}


vs_cursor *vsc_translate(vs_cursor *source,vs_cursor *metacursor){
 vs_cursor *result;
 vector *v,*sv;
 result = vsc_createcursor();
 vsc_reset(metacursor);
 while (!vsc_eof (metacursor)){
    v = vsc_readnext(metacursor);
    sv = vsc_findvector(source,v->vectorid);
    if (sv!=NULL){
       vsc_rawloadvector(result,sv);
    }
 }
 return(result);
}

vs_cursor *vsc_vector_cluster(vs_cursor *source,int segment,int segmentsize,float threshold,int cluster_minimum_size){
  vs_cursor  *metacursor,*consolidated,*final,*normalized;
  normalized = vsc_normalize (source);
  metacursor = vsc_metacursor(normalized,segment,segmentsize,threshold,cluster_minimum_size);
  consolidated = vsc_consolidate(metacursor,threshold,cluster_minimum_size); //metacursor is already freed
  final = vsc_translate(normalized,consolidated); //translate back to our original vectors.
  vsc_destroycursor(&metacursor);
  vsc_destroycursor(&normalized);
  vsc_destroycursor(&consolidated);
  return(final);
}

vs_cursor *vsc_vector_metacluster(vs_cursor *source,int segment,int segmentsize,float threshold,int cluster_minimum_size){
  vs_cursor  *metacursor,*consolidated ;
  metacursor = vsc_metacursor(source,segment,segmentsize,threshold,cluster_minimum_size);
  consolidated = vsc_consolidate(metacursor,threshold,cluster_minimum_size);
  return(consolidated);
}




