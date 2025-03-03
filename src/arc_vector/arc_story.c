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
#include "arc_story.h"
#include "arc_memory.h"


arc_story *arc_create_story(){
 arc_story *result;
 result = (arc_story *)malloc(sizeof(arc_story));
 result->first=NULL;
 result->last=NULL;
 result->count=0;      
 result->readposition=0;
 return result;
}

int arc_destroy_story(arc_story **story){
  arc_moment *moment;
  arc_story *temp_story;
  if(story!=NULL){
  temp_story = (*story);
   while (temp_story->count>0){
      moment = arc_story_pop(temp_story);
      arc_destroy_moment(&moment);
   }
  free(*story);
  }
  return 1;
}


int arc_story_push (arc_story *story, arc_moment *moment){
  //push to front of list
  arc_moment *first;
  first = story->first;
  if (moment!=NULL){
    if (story->first!=NULL){
       moment->next = first;
       story->first = moment;
    } else {
      story->first=moment;
    }
    story->count++;
  }
  return story->count;
}


arc_moment *arc_story_pop (arc_story *story){
  //pop off top of list
  arc_moment *first, *result;
  first = story->first;
  result = NULL;
  if (first!=NULL){
    story->first = (arc_moment *)first->next;
    story->count--;
    result = first;
  }
  if(story->count<0){story->count=0;}
  return result;
}


int arc_story_elapse (arc_story *story, arc_moment *moment, float clock, float threshold){
  //push if score < threshold, otherwise add clock to duration of first.
  arc_moment *first;
  first = story->first;
  if (first==NULL){
     moment->duration=clock;
     arc_story_push(story,moment);
  } else {
    if(arc_moment_similarity(first,moment) > threshold){
      moment->duration=clock;
      arc_story_push(story,moment);
    } else {
      first->duration+=clock;
    }
  }
  return story->count;
}


int arc_story_clone(arc_story *src, arc_story *dest){
  arc_moment *moment,*copy,*lastcopy;
  moment = src->first;
  lastcopy=NULL;
  while (moment!=NULL){
    copy=arc_clone_moment(moment);
    if (dest->first==NULL){dest->first=copy;}
    if (lastcopy==NULL){lastcopy=copy;} else {lastcopy->next=copy;}
    dest->count++;
    moment = moment->next;
  }
  return dest->count;
}

void arc_story_print(arc_story *src){ 
  arc_moment *moment;
  moment = src->first; 
  printf("S\r\n");
  while (moment!=NULL){
    vsc_print_cursor(moment->vectors);
    moment = moment->next;
  }
}


float arc_story_similarity(arc_story *source, arc_story *query){
 //ok, we have two lists of moments
 /*
   A1   A2   A3
   B1   B2   B3

 and we score each moment to ALL the moments in the other story
 the scores as we move away from position are multiplied by some value so that
 as things are out of sync, the score goes down..
 the easiest one right now is 1,.5,.25,.125 ... there is probably some other ratio that /2 that would work better
 or may require a tunable parameter.

 ok.. so for all NON-ZERO scores, we average again.

 so I need a scrolling capability like the cursors.

 */
 int scorecounted;
 float scoretotal ,score,offset,best,bestoffset ;
 arc_moment *query_moment,*source_moment;
 int spos,qpos;

 scorecounted=0;
 scoretotal=0;
 qpos=0;
 spos=0;
 arc_story_reset(query);
 while(!arc_story_eof(query)){
    query_moment = arc_story_scroll(query);
    spos=0;
    arc_story_reset(source);
    best = 0; 
    if (query->count>source->count){bestoffset=query->count+1;} else {bestoffset=source->count+1;}


     //    printf(" Query \r\n");
    while(!arc_story_eof(source)){
         source_moment = arc_story_scroll(source);

         score=arc_moment_similarity(source_moment,query_moment);
         offset =  (float)fabs(spos-qpos) ;

       //  printf(" score   %f \r\n",score);
       if ((score>=best) && (offset<=bestoffset)){
         best = score;
         bestoffset =  (float)fabs(spos-qpos) ;  //position line up is good!
         //offset=1;
         //may want to add some modifier based on differences of duration... or not. I'm leaving out for now.

       }
       spos++;
    }
    if (best>0){
     scoretotal+= (best * pow(0.75,bestoffset));
     scorecounted+=1;
         // printf(" Best   %f \r\n",best);
         // printf("   Offs %f \r\n",bestoffset);
    }
   qpos++;
 } 
 if (scorecounted>0){
   return (scoretotal / (float)scorecounted);
 } else {
   return 0;
 }
}


void arc_story_reset(arc_story *story){
  story->scroll=story->first;
  story->readposition=0;
}

int arc_story_eof(arc_story *story){
  return(story->readposition>=(story->count));
}

arc_moment *arc_story_scroll(arc_story *story){
  arc_moment *scroll;
  scroll = story->scroll;
  story->scroll = scroll->next;
  story->readposition++;
  return scroll;
}




