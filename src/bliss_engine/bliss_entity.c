
arc_entity  *arc_create_entity(t_uuid id){
 arc_entity *me;
 me = malloc(sizeof(arc_entity));
 me->previous_bliss_score = 1.000;
 me->id = id;
 me->bliss=vs_createvector(id,2);
 me->internal=vs_createvector(newid(),2);
 me->short_term = arc_create_memory();
 me->long_term = arc_create_memory();
 me->actions = vs_createvector(newid(),0);
 me->perception = arc_create_story();
 me->perception_max_duration = 0;
 
 return me;
}

int arc_destroy_entity(arc_entity **entity){
  arc_entity *e;
  e = *entity;
  vs_destroyvector(e->bliss);
  vs_destroyvector(e->internal);
  arc_destroy_memory(e->short_term);
  arc_destroy_memory(e->long_term);
  arc_destroy_story(e->perception);
  free(e);
}

void arc_entity_bliss_quickset(arc_entity *entity, t_uuid id, float value){
  vs_quickset(entity->bliss,id,value);
}

void arc_entity_internal_quickset(arc_entity *entity, t_uuid id, float value){
  vs_quickset(entity->internal,id,value);
}


int arc_entity_perceive(arc_entity *entity,vs_vector *perception); 

int arc_entity_action(arc_entity *entity,arc_moment *action);  

int arc_entity_tick(arc_entity *entity,float clockdiff); //pushes now into perception, starts a new "now"

float arc_entity_bliss(arc_entity *entity){
   /*compare bliss to internal state, gives score between 1=good, 0=bad*/
   float score;
   float dp;
     score = vs_relativequerycosine(entity->bliss,entity->internal,dp);
   return score;
}

arc_moment *arc_entity_live(arc_entity *entity,float threshold,float clockdiff){
  float now_bliss_score;
  arc_moment *action;
  arc_entity_tick(entity,clockdiff); //create a moment, fill with bliss and now, and push into perception.
  now_bliss_score = arc_entity_bliss(entity);  //see how blissed out we are
  if (abs(now_bliss_score-entity->previous_bliss_score)>threshold){
     action = arc_entity_lookup(entity->perception);                //try and remember a similar situation - except ending with us all blissed out (extra moment)
     if (action==NULL){ //if story is empty
       action = arc_entity_random_action(entity);                    //if nothing, then just do something random.
     }
     arc_entity_action (entity,action); //adds the action to our memory
     arc_entity_remember(entity); //story in shortterm/longterm
  }
  entity->previous_bliss_score = now_bliss_score;
  return action;
}
This is close - but not quite....


/*
1. create an internal state of bliss as NOT-VALUES ... NOT-HUNGRY
2. add everything the creature sees (using distance as a dimension / difficulty ) in perceive
3. add feeling (vector internal state)
4


*/


