#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include "vs_core.h"
#include "vs_list.h"
#include "vs_queue.h"
#include "vs_cache.h"
#include "vs_cursor.h"
#include "vs_interpreter.h"
#include "vs_query.h"
#include "vs_tensor.h"
#include "vs_cluster.h"
#include "vs_thesaurus.h"
#include "vs_modes.h"
#include "vs_persist.h"
#include "vs_output.h"


int vsinterpreter_execute (vs_cursor *cursor,vs_cursor *thesaurus,interpreter_session *session, char command, t_uuid elementid,float value, vs_queue *output_buffer , FILE 	*logfile){
  vs_value val;
  vs_cursor *cluster;
  vector *temp,*tensor;
  vector *output;
  char *did,*cCX,*cCY,*cCZ,*cCA;
  int result,min, CX,CY, CZ;
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

                 vsc_replacevector(cursor,session->current_vector);
                 vs_destroyvector(&session->current_vector);
                 session->current_vector=NULL;
              }
              if (session->command=='T'){
               vsc_replacevector(thesaurus,session->current_vector);
               vs_destroyvector(&session->current_vector);
              }
            }
             result = 1;
           }
  }
  if (command=='-'){
      vsc_deletevector(cursor,elementid);
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
      vsdb_query(session->id,cursor,session->current_vector,value,output_buffer);
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

     if (logfile) { fprintf(logfile,"Querying Threshold %f\n",value); fflush(logfile); }

      vsdb_query(session->id,cursor,session->current_vector,value,output_buffer);
      vs_destroyvector(&session->current_vector);
      result = 2;
  }
  
  if (command=='*'){
                 if (session->mode==CLASSIFIED_VECTOR_MODE){
                   vst_classify_vector(session->current_vector,thesaurus,session->thesaurus);
                 }

                 if (session->mode==META_VECTOR_MODE){
                   temp = vst_metavector(session->current_vector,thesaurus,session->thesaurus);
                   vs_destroyvector(&session->current_vector);
                   session->current_vector = temp;
                 }

     if (logfile) { fprintf(logfile,"Converting Query to Tensor %f\n",value); fflush(logfile); }

      tensor = vsdb_tensor(cursor,session->current_vector,value);
      vsdb_tensor_remove_query(tensor,session->current_vector,-1);
      
      vs_queued_printvector(session->id, tensor, output_buffer );
      
      vs_destroyvector(&tensor);
      vs_destroyvector(&session->current_vector);
      result = 2;
  }
  
  
  if (command=='>'){
                   if (session->mode==CLASSIFIED_VECTOR_MODE){
                   vst_classify_vector(session->current_vector,thesaurus,session->thesaurus);
                 }

                 if (session->mode==META_VECTOR_MODE){
                   temp = vst_metavector(session->current_vector,thesaurus,session->thesaurus);
                   vs_destroyvector(&session->current_vector);
                   session->current_vector = temp;
                 }

     if (logfile) { fprintf(logfile,"Converting Query to Tensor %f\n",value); fflush(logfile); }

      tensor = vsdb_tensor(cursor,session->current_vector,value);
      vsdb_tensor_remove_query(tensor,session->current_vector,-0.001);
      
      
      output = vs_createvector(session->id,0);
      dimension *dim;
      dim = vs_highest_dimension(tensor);
      vs_setvalue (output,dim->dimensionid,dim->value);
      
      vs_queued_printvector(session->id, output, output_buffer );
      
      vs_destroyvector(&output);
      vs_destroyvector(&tensor);
      vs_destroyvector(&session->current_vector);
      result = 2;
  }
  

  if (command=='C'){
      did = HashToString(elementid);  // XXXXXXXXXXYYYYYYYYYYZZZZZZZZZZAA
      //min = atoi(did);
      cCX = malloc(10);       cCY = malloc(10);       cCZ = malloc(10); cCA = malloc(2);
      sscanf(did,"%10s%10s%10s%2s",cCX,cCY,cCZ,cCA);
      CX = atoi(cCX);      CY = atoi(cCY);      CZ = atoi(cCZ);
      free(did);                        //X          Y           Z
    //  cluster = vsc_vector_cluster(cursor,CX,CY,value,CZ);
    //  vsdb_outputcursor(session->id,cluster,output_buffer);
    //  vsc_destroycursor(&cluster);
    //  result = 1;

 // cluster = vsc_vector_metacluster(cursor,CX,CY,value,CZ);
  cluster = vsc_vector_cluster(cursor,CX,CY,value,CZ);
  vsdb_outputcursor(session->id,cluster,output_buffer);
  vsc_destroycursor(&cluster);
  result=1;

  }

  if (command=='^'){ /*Go find a vector by ID!*/ 
      output = vsc_findvector (vs_cursor *cursor,t_uuid vectorid)
      vs_queued_printvector(session->id, output, output_buffer );
      result = 1;
  }

  if (command=='K'){
      did = HashToString(elementid);
      min = atoi(did);
      free(did);
      cluster = vsc_vector_metacluster(cursor,0,cursor->count,value,min);
      vsdb_outputcursor(session->id,cluster,output_buffer);
      vsc_destroycursor(&cluster);
      result = 1;
  }

  if (command=='S'){
     //used by the persistor to dump the contents of the cursor
     //persistor then reloads everything using standard V commands.
     // vsdb_outputcursor(session->id,cursor,output_buffer);
     // result = 1;
  }






 return(result);
}
