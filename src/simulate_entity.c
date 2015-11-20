#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/times.h>
#include <mcheck.h>
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
#define eclock() times(NULL)   
/*
tests to see if indexing is working
*/


int main(){
 vector *v,*bliss,*internal,*nowvector ;
 arc_memory *memory,*remembered ;
 arc_story *walkeatgrass,*nowstory,*rememberedstory;
 arc_moment *cowgrasswalkhungry,*cowgrasseatnohungry,*nowmoment;
 arc_moment *found, *rememberedmoment;
 vs_value value,_one,check,_query;
 t_uuid  _self,_grass,_rock,_NOThungry,_offset,_walk,_eat;
 dimension *d;
 float score,dp;
 int time;
 float cowx,cowy,grassx,grassy,rockx,rocky;
 vs_cursor *foundcursor;

 
 mtrace();

 cowx=100;cowy=100;
 grassx=95;grassy=95;
 rockx=90;rocky=90;

 _one.floatvalue = 1;
 _query.floatvalue = 0.75;

 _grass = StringToHash("09d440e487d45777c05c3a6552ad9154");
 _self = StringToHash("dfed8998cf17326dafd61191510cb0e2");  //self_me
 _rock = StringToHash("9a1f30943126974075dbd4d13c8018ac");
 _NOThungry = StringToHash("520e70a2c46290e1d52a78b1f1707010");
 _offset = StringToHash("00000000000000000000000000000001"); //used to offset _query
 _walk = StringToHash("46f9631576015f35f527af2bc67351a6");
 _eat = StringToHash("9a97eda6d89f5c9e84560468a0679707");


//ok, we have a cow, with bliss of not being hungry

 bliss=vs_createvector(newid(),2);
 vs_setvalue (bliss,_NOThungry,_one);
 vs_setvalue (bliss,_offset,_one);

 // each clock tick, and we become more hungry, so we need some kind of internal state
 internal=vs_createvector(newid(),2);
 vs_setvalue (internal,_NOThungry,_one);
 vs_setvalue (internal,_offset,_one);
 vs_setvalue (internal,_grass,_one); 


    //so the cow perceives itself
    score = vs_relativequerycosine(bliss,internal,&dp);
    printf(" Cow Bliss: %f \r\n", score);

  memory = arc_create_memory();
    walkeatgrass = arc_create_story();
      cowgrasswalkhungry =arc_create_moment();   //hungry and walking
              v=vs_createvector(newid(),2);
              vs_setvalue (v,_self,_one);
              value.floatvalue=0.1 ;vs_setvalue (v,_NOThungry,value);
              value.floatvalue=10 ;vs_setvalue (v,_grass,value); //"distance" or potentiality
              value.floatvalue=1 ;vs_setvalue (v,_walk,value);  //value of my choice/action, half-ass, or full on
               
              arc_push(cowgrasswalkhungry,v);
      cowgrasseatnohungry = arc_create_moment();     //found and ate some grass, was no longer hungry
              v=vs_createvector(newid(),2);
              vs_setvalue (v,_self,_one);
              value.floatvalue=0.1 ;vs_setvalue (v,_NOThungry,value);
              value.floatvalue=0.1 ;vs_setvalue (v,_grass,value); //"distance" or potentiality
              value.floatvalue=1 ;vs_setvalue (v,_eat,value);  //value of my choice/action, half-ass, or full on
              arc_push(cowgrasseatnohungry,v);

  arc_story_push (walkeatgrass,cowgrasswalkhungry);
  arc_story_push (walkeatgrass,cowgrasseatnohungry);
  arc_store(memory,walkeatgrass);

 //now we need a clock, and let the cow get hungrier
 time=0;
 
 float grassvalue = 18;
 vector *bestv = NULL;
 
 while (time<2000000){
    time++;
    d = vs_getvalue(internal,_NOThungry);
    check = d->value;
    check.floatvalue = check.floatvalue * 0.79;  
    //printf("NOT HUNGRY: %f\r\n",check.floatvalue);
    vs_setvalue (internal,_NOThungry,check);
   
    //so the cow perceives itselfas getting hungry, and it isn't in a state of bliss.
    score = vs_relativequerycosine(bliss,internal,&dp);
    //printf(" Cow Bliss: %f \r\n", score);

    nowstory = arc_create_story();
    nowmoment = arc_create_moment();
    nowvector = vs_createvector (newid(),0);
              vs_setvalue (internal,_self,_query);
              vs_setvalue (internal,_offset,_query);
              vs_clone (internal,nowvector);
              value.floatvalue=grassvalue ;vs_setvalue (nowvector,_grass,value);
              
              arc_push(nowmoment,nowvector); 
              arc_story_push (nowstory,nowmoment);

 //printf("This Moment:\r\n");
 //arc_story_print(nowstory);
    vector *c;
    remembered = arc_recall(memory,nowstory,-1); 
    if(remembered->count>0){
      //arc_memory_print(remembered);
      arc_memory_reset(remembered);
      float best = 0 ;
      float score = 0 ;
      float dp = 0 ;
      while(!arc_memory_eof(remembered)){
      rememberedstory = arc_memory_scroll(remembered); 

      arc_story_reset(rememberedstory);
      while(!arc_story_eof(rememberedstory)){
      rememberedmoment = arc_story_scroll(rememberedstory); 
      found = arc_moment_vector_query(rememberedmoment, internal, -1);
      foundcursor = found->vectors;
      
      vsc_reset(foundcursor);
      while(!vsc_eof(foundcursor)){
      c = vsc_readnext(foundcursor);
      score = vs_relativequerycosine(c,nowvector,&dp);
      if(score>best){
        best=score; printf("%f ",score);
        if(bestv!=NULL) vs_destroyvector(&bestv);
         bestv = vs_createvector(newid(),0);
        vs_clone(c,bestv);
      }
      }
      arc_destroy_moment(&found);      
     }

        d = vs_getvalue(bestv,_walk);
      if(d!=NULL){
        if (d->value.floatvalue==1){
         printf("walking! %f\r\n",best);
         grassvalue-=1;
         }
      }
      
      d = vs_getvalue(bestv,_eat);
      if(d!=NULL){
        if (d->value.floatvalue==1) {
         printf("eating! %f\r\n",best);
         vs_setvalue (internal,_NOThungry,_one);
         value.floatvalue=20 ; vs_setvalue (internal,_grass,value);
         grassvalue=20; 
         }
      }      

      arc_destroy_memory(&remembered,0);
      arc_destroy_story(&nowstory);      
      
     }
 
    } else {
      printf("No matching story found\r\n");
    }

   


    //need a special way to check the END of the story for the outcome to score which memory is the best to use.
    //ahh.. we put internal+perception in as the start of the story, and the bliss as the END of the story query
    //and we should get the best possible story that ends with us with a full belly.


 }
 //vs_printvector(bliss);
 //vs_printvector(internal);
 if(bestv!=NULL) vs_destroyvector(&bestv);
 vs_destroyvector(&internal);
 vs_destroyvector(&bliss);
  
 arc_destroy_memory(&memory,1);

 return(0);
}
