#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/times.h>
#include <math.h>
#include "vs_core.h"
#include "vs_list.h"
#include "vs_cursor.h"
#include "vs_rowset.h"
#include "vs_queue.h"
#include "vs_query.h"
#include "vs_cluster.h"
#include "vs_thesaurus.h"
#include "vs_persist.h"
#include "arc_moment.h"

arc_moment *arc_create_moment(){
  arc_moment *result;
  result= (arc_moment *)malloc(sizeof(arc_moment));
  result->vectors = vsc_createcursor();
  result->duration =0;
  result->next=NULL;
  return(result);
}

int arc_destroy_moment(arc_moment **moment){
  vs_cursor *vectors;
  arc_moment *temp;
  if (moment!=NULL){
     temp = (*moment);
     vectors = temp->vectors;
     vsc_destroycursor(&vectors);
     free(*moment);
     return(1);
  } else { return(0);}
}

arc_moment *arc_clone_moment (arc_moment *source){
  arc_moment *result;
  result= (arc_moment *)malloc(sizeof(arc_moment));
  result->vectors = vsc_clonecursor(source->vectors);
  result->duration = source->duration;
  result->next=NULL;
  return result;
}

float arc_moment_similarity_by_proximity(arc_moment *source, arc_moment *query, int proximal){
  //ok.. we are taking the average of all scoring combinations... so has to be > 0 to count.
  int scoringcount,sourcepos,querypos;
  float scoringtotal,score,mdif;
  float dp;
  vector *q,*v;
  scoringtotal=0; score=0;scoringcount=0;
  vsc_reset(query->vectors);
  sourcepos=0;

  while(!vsc_eof(query->vectors)){
     q = vsc_readnext(query->vectors);
     vsc_reset(source->vectors);
     querypos=0;
     while(!vsc_eof(source->vectors)){
        v = vsc_readnext(source->vectors);
        score = vs_relativequerycosine(q,v,&dp);
        if (score>0){
          scoringcount++;

         if (proximal){
          mdif = 1 * ( pow(0.5, abs(querypos-sourcepos)));
          scoringtotal=scoringtotal+(score * mdif);
         } else {
          scoringtotal=scoringtotal+score;
         }

        }
        querypos++;
     }
     sourcepos++;
  }

  if (scoringcount>0){
    return (scoringtotal/scoringcount);
  } else {
    return 0;
  }
}


arc_moment *arc_moment_vector_query(arc_moment *source, vector *query, float threshold){
  arc_moment *result;
  float score;
  float dp;
  vector *v,*vc;  
  result = arc_create_moment(); 
  vsc_reset(source->vectors);
  while(!vsc_eof(source->vectors)){
        v = vsc_readnext(source->vectors); 
        score = vs_relativequerycosine(query,v,&dp);
        if(score>threshold){
          vc = vs_createvector(newid(),0);
          vs_clone(v,vc);
          arc_push(result,vc); 
        }
     }
  return result;   
}

float arc_moment_similarity(arc_moment *source, arc_moment *query){
  return arc_moment_similarity_by_proximity(source,query,0);
}


int arc_push(arc_moment *moment, vector *v){
  int i;
 // i = vsc_replacevector(moment->vectors,v); --clones the vector
  i = vsc_rawinsertvector(moment->vectors,v);
  return i;
}
