#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include "vs_core.h"
#include "vs_list.h"
#include "vs_queue.h"
#include "vs_cursor.h"
#include "vs_cursorindex.h"
#include "vs_query.h"
#include "vs_interpreter.h"
#include "vs_thesaurus.h"
#include "vs_modes.h"
#include "vs_cluster.h"
#include "vsdb_sdk.h"
#include "vs_persist.h"

EXPORT vs_cloudspace *create_cloudspace(){
  vs_cloudspace *result;
  result = malloc(sizeof(vs_cloudspace));
  result->cursor =  vsc_createcursor();
  result->cursor->indexed = 1;
  result->thesaurus = vsc_createcursor();
  result->output_buffer = vs_prepare_queue();
  result->session = malloc(sizeof(interpreter_session));
  return(result);
}

EXPORT void destroy_cloudspace(vs_cloudspace **cloudspace){
  vs_cloudspace *cs;
  cs = *cloudspace;
  vsc_destroycursor(&cs->cursor);
  vsc_destroycursor(&cs->thesaurus);
  vs_destroy_queue(&cs->output_buffer);
  if (cs->session->current_vector!=NULL){vs_destroyvector(&cs->session->current_vector);}
  free(cs->session);
  free(*cloudspace);
}

EXPORT int vsdb_execute(vs_cloudspace *cloudspace,char command,char *element,float value){
  int res;
  t_uuid id;
  id = StringToHash(element);
  res = vsinterpreter_execute (cloudspace->cursor,cloudspace->thesaurus,cloudspace->session,command,id,value,cloudspace->output_buffer);
  return(res);
}

EXPORT int vsdb_metacluster(vs_cloudspace *cloudspace,int segment,int segmentsize,float threshold,int cluster_minimum_size){
  vs_cursor *cluster;
  int sz;
  cluster = vsc_vector_metacluster(cloudspace->cursor,segment,segmentsize,threshold,cluster_minimum_size);
  vsdb_outputcursor(cloudspace->session->id,cluster,cloudspace->output_buffer);
  sz =cluster->count;
  vsc_destroycursor(&cluster);
  return (sz);
}

EXPORT int vsdb_outputbuffer(vs_cloudspace *cloudspace,char *buffer){
 int res;
 vs_queuenode *node;
 res=0;
 if ( vs_queue_size(cloudspace->output_buffer)>0 ){
   node = vs_queue_pop(cloudspace->output_buffer);
   memcpy(buffer,node->buffer,node->size);
   res = node->size;
   vs_destroy_node(&node);
 }
 return(res);
}

EXPORT int vsdb_savecloudspace(vs_cloudspace *cloudspace,char *data,char *thesaurus){
  vs_cursor *vcursor;
  vs_cursor *vthesaurus;
  vcursor=cloudspace->cursor;
  vthesaurus = cloudspace->thesaurus;
  if(vcursor->count>0) vp_save_cursor(cloudspace->cursor,data);
  if(vthesaurus->count>0) vp_save_cursor(cloudspace->thesaurus,thesaurus);
  return(1);
}

EXPORT int vsdb_loadcloudspace(vs_cloudspace *cloudspace,char *data,char *thesaurus){

  vp_load_cursor(cloudspace->cursor,data);
  vp_load_cursor(cloudspace->thesaurus,thesaurus);

  return(1);
}
