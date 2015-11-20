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

void arc_memory_reset(arc_memory *memory){
  memory->scroll=memory->first;
  memory->readposition=0;
}

int arc_memory_eof(arc_memory *memory){
  return(memory->readposition>(memory->count-1));
}

arc_story *arc_memory_scroll(arc_memory *memory){
  arc_story *scroll,*result;
  scroll = memory->scroll;
  memory->scroll = (arc_story *)scroll->next;
  if (arc_memory_eof(memory)){
     result = NULL;
    } else {
     result = scroll;
    }
  memory->readposition++;
  return result;
}


arc_memory *arc_create_memory(){
 arc_memory *result;
 result = (arc_memory *)malloc(sizeof(arc_memory));
 result->first=NULL;
 result->count=0;
 result->last=NULL;
 return result;
}


int arc_destroy_memory(arc_memory **memory, int destroy_stories){
  arc_story *story;
  arc_memory *temp_memory;
  temp_memory = (*memory);
  if(destroy_stories==1){
    arc_memory_reset(temp_memory);
    while (!arc_memory_eof(temp_memory)){
     story = arc_memory_scroll(temp_memory);
     arc_destroy_story(&story);
    }
    temp_memory->first = NULL;
    temp_memory->last = NULL;
  }
  free(*memory);
  return 1;
}

int arc_store(arc_memory *memory,arc_story *story){ //put story into memory - recent memories first.
  arc_story *first;
  first = memory->first; 
  if (story!=NULL){ 
    if (memory->first!=NULL){
       story->next = first;
       memory->first = story;
    } else {
      memory->first=story;
      story->next=NULL;
    }
    if (memory->last==NULL){ memory->last = story; }
    memory->count++;
  }
  return memory->count;
}

int arc_forget(arc_memory *memory){
  arc_story *last ;
   last = memory->last;
   if (memory!=NULL){
    if (memory->last!=NULL){
       memory->last = (arc_story *)last->next;
       arc_destroy_story(&last);
       memory->count--;
    }
  }
  return memory->count;
}

arc_memory *arc_recall(arc_memory *memory,arc_story *query, float threshold){ //may bring back more than one memory
  arc_story *story;
  arc_memory *result;
  float similarity;
  arc_memory_reset(memory);
  result = arc_create_memory();
  while (!arc_memory_eof(memory)){
    story = arc_memory_scroll(memory);
    similarity = arc_story_similarity(story,query);
    if (similarity>=threshold){
      story->score = similarity;
      arc_store(result,story);
    }
  }
 return(result);
}

void arc_memory_print(arc_memory *memory){
  arc_story *story;
  arc_memory_reset(memory);
  printf("M\r\n");
  while (!arc_memory_eof(memory)){
    story = arc_memory_scroll(memory);
    arc_story_print(story);
  }
}


arc_memory *arc_recall_moment(arc_memory *memory,arc_moment *query, float threshold){ //- forms a basic story, then may bring back more than one memory
 arc_story *story_query;
 arc_memory *result;
 result = NULL;
 if(query!=NULL){
   story_query = arc_create_story();
   arc_story_push(story_query,query);
   result = arc_recall(memory,story_query,threshold);
   arc_story_pop(story_query); //pop, so we don't accidentally delete.
   arc_destroy_story(&story_query);
 }
 return(result);
}

