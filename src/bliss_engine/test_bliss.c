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
#define eclock() times(NULL);

 vs_value value,_one,_zero,_query;
 arc_entity *entity;
 
 arc_memory *memory;
 arc_story *walkeatgrass;
 arc_moment *cowgrasswalkhungry,*cowgrasseatnohungry,*nowmoment;
  
 entity = arc_create_entity(newid());

 _one.floatvalue = 1;
 _zero.floatvalue = 0;
 _query.floatvalue = 0.75;

 _grass = StringToHash("09d440e487d45777c05c3a6552ad9154");
 _self = StringToHash("dfed8998cf17326dafd61191510cb0e2");  //self_me
 _rock = StringToHash("9a1f30943126974075dbd4d13c8018ac");
 _NOThungry = StringToHash("520e70a2c46290e1d52a78b1f1707010");
 _offset = StringToHash("00000000000000000000000000000001");
 _walk = StringToHash("46f9631576015f35f527af2bc67351a6");
 _eat = StringToHash("9a97eda6d89f5c9e84560468a0679707");
 
 arc_entity_bliss_quickset(entity, _NOThungry, _one);
 arc_entity_bliss_quickset(entity, _offset, _one);
 
 // each clock tick, and we become more hungry, so we need some kind of internal state
 arc_entity_bliss_quickset(entity, _NOThungry, _one);
 arc_entity_bliss_quickset(entity, _grass, _one);
 arc_entity_bliss_quickset(entity, _offset, _one);
 
memory = arc_create_memory();
 walkeatgrass = arc_create_story();
      cowgrasswalkhungry =arc_create_moment();   //hungry and walking
              v=vs_createvector(newid(),2);
              vs_setvalue (v,_self,_one);
              vs_quickset(v,_NOThungry,0.1);              
              vs_quickset(v,_grass,10);              
              vs_quickset(v,_walk,1);               
              arc_push(cowgrasswalkhungry,v);
      cowgrasseatnohungry = arc_create_moment();     //found and ate some grass, was no longer hungry
              v=vs_createvector(newid(),2);
              vs_setvalue (v,_self,_one);
              vs_quickset(v,_NOThungry,0.1);              
              vs_quickset(v,_grass,0.1);              
              vs_quickset(v,_eat,1); 
              arc_push(cowgrasseatnohungry,v);
 arc_story_push (walkeatgrass,cowgrasswalkhungry);
 arc_story_push (walkeatgrass,cowgrasseatnohungry);
arc_store(memory,walkeatgrass);
 

