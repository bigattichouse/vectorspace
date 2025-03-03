#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include "vs_core.h"
#include "vs_list.h"
#include "vs_queue.h"
#include "vs_cache.h"
#include "vs_cursor.h"
#include "vs_disc_index.h"
#include "vs_disc_data.h"
#include "vs_disc_dimensions.h"
#include "vs_disc_interface.h"
#include "vs_disc_interpreter.h"
#include "vs_query.h"
#include "vs_cluster.h"
#include "vs_thesaurus.h"
#include "vs_modes.h"
#include "vs_persist.h"
#include "vsd_cursor.h" 

int vsd_interpreter_execute ( __vdi_interface *intf,char *thesaurus_directory,vs_cursor *cursor,vs_cursor *thesaurus,interpreter_session *session, char command, t_uuid elementid,float value, vs_queue *output_buffer ){
  vs_value val;
  vector *temp;
  char *did;
  int result,min;
  result = 0;
  time(&session->touched);
  if (command=='M'){
      did = HashToString(elementid);
      min = atoi(did);
      free(did);
      session->mode = (int)min;
      session->thesaurus = value;
  }

  if (command=='Q'){
             session->current_vector = vs_createvector (elementid,(int) value);
           //  printf ("Q%d\n",(int) value);
             session->vectorcount=0;   session->command=command;
             result = 1;
           }
  if (command=='V'){
          //   printf ("V%d\n",(int) value);
             session->current_vector = vs_createvector (elementid,(int) value);
             session->vectorcount=0;   session->command=command;
             result = 1;
           }

  if (command=='T'){
          //   printf ("V%d\n",(int) value);
             session->current_vector = vs_createvector (elementid,(int) value);
             session->vectorcount=0;   session->command=command;
             result = 1;
           }

  if (command=='D'){
            if (session->current_vector!=NULL){
             val.floatvalue= value;
             vs_setvalue(session->current_vector,elementid,val);
             session->vectorcount+=1;
            if (session->vectorcount==session->current_vector->dimensioncount){
              if (session->command=='V'){
                                        	
                 if (session->mode==CLASSIFIED_VECTOR_MODE){
                   vst_classify_vector(session->current_vector,thesaurus,session->thesaurus);
                 }
                 
                 if (session->mode==META_VECTOR_MODE){
                   temp = vst_metavector(session->current_vector,thesaurus,session->thesaurus);
                   vs_destroyvector(&session->current_vector);
                   session->current_vector = temp;
                 }                                

                //DISK REPLACE VECTOR

                 vsd_replacevector(intf,session->current_vector);
                 vs_destroyvector(&session->current_vector);
                 session->current_vector=NULL;
              }
              if (session->command=='T'){
               //DISK REPLACE THESAURUS
               vsd_replacevector(intf,session->current_vector);
               vs_destroyvector(&session->current_vector);
              }
            }
             result = 1;
           }
  }
  if (command=='-'){
     //DISK DELETE VECTOR
     vsd_deletevector(intf,elementid);
      result = 1;
  }

  if (command=='E'){
                 if (session->mode==CLASSIFIED_VECTOR_MODE){
                   vst_classify_vector(session->current_vector,thesaurus,session->thesaurus);
                 }
                 
                 if (session->mode==META_VECTOR_MODE){
                   temp = vst_metavector(session->current_vector,thesaurus,session->thesaurus);
                   vs_destroyvector(&session->current_vector);
                   session->current_vector = temp;
                 }
     //DISK QUERY
      vsd_query(intf,session->id,session->current_vector,value,output_buffer);
      vs_destroyvector(&session->current_vector);
      session->current_vector=NULL;
      result = 1;
  }

  if (command=='X'){
                 if (session->mode==CLASSIFIED_VECTOR_MODE){
                   vst_classify_vector(session->current_vector,thesaurus,session->thesaurus);
                 }

                 if (session->mode==META_VECTOR_MODE){
                   temp = vst_metavector(session->current_vector,thesaurus,session->thesaurus);
                   vs_destroyvector(&session->current_vector);
                   session->current_vector = temp;
                 }
     //DISK QUERY    
      vsd_query(intf,session->id,session->current_vector,value,output_buffer);
      vs_destroyvector(&session->current_vector);
      result = 2;
  }


 return(result);
}
