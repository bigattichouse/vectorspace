typedef struct {
  t_uuid id;
  float previous_bliss_score;
  arc_moment *bliss;  /* !hungry=1, !scared=1 .. likely only one vector, but figured I'd  leave it open for more */
  arc_moment *internal; //internal state
  arc_story  *perception; //everything around me
  vector     *actions;
  single perception_max_duration;  //how long things can stay in my working memory.
  arc_memory *short_term;
  arc_memory *long_term;
} arc_entity;

arc_entity  *arc_create_entity(t_uuid id);
int arc_destroy_entity(arc_entity **entity);

/*Loads our critter's perceptions with the world around it.*/
int arc_entity_perceive(arc_entity *entity,vs_vector *perception); //appends to now
int arc_entity_action(arc_entity *entity,arc_moment *action); //appends to now
//manipulate the internal state vectors directly via the cursor
int arc_entity_tick(arc_entity *entity,float clockdiff); //pushes now into perception, starts a new "now"

float arc_entity_bliss(arc_entity *entity); /*compare bliss to internal state, gives score between 1=good, 0=bad*/

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


