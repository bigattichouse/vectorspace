#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "vs_core.h"
#include "vs_queue.h"
#include "vs_list.h"
#include "vs_cursor.h"
#include "vs_query.h"
#include "vs_disc_data.h"
#include "vs_disc_index.h"
#include "vs_disc_dimensions.h"
#include "vs_disc_interface.h" 


//wraps vs_disc_interface

void vsd_replacevector( __vdi_interface *intf,vector *v){
 
 __vdi_replace_vector(intf,v);
 
}

void vsd_deletevector( __vdi_interface *intf,t_uuid vectorid){
 
  __vdi_delete_vectorbyid(intf,vectorid); 
}

int vsd_query( __vdi_interface *intf,t_uuid sessionid,vector *query,float threshold,vs_queue *queue){
 vs_cursor *cursor;
 int count,sz;
 float magq,magd,dbcos,dp;
 vector *v;
 char *did,*outbuff; 

 count =0;
 cursor = vsc_createcursor();
 
 __vdi_query_load_cursor( intf,  query, cursor);

 count=0;
 magq = vs_magnitude(query);
 vsc_reset(cursor); 
 while (vsc_eof(cursor)==0){
   v = vsc_readnext(cursor);
   if (v!=NULL){
   magd = vs_relativity(v,query);
   dbcos = vs_relativequerycosine(query,v,&dp); //magnitudes are already calc'ed by storage load and above.
   if (dbcos>threshold){
      did = HashToString(v->vectorid);
      outbuff = malloc(512);
      sz = sprintf(outbuff, "R %s %f %f %f\r\n",did,magd,dp,dbcos);      
      outbuff[sz]=0;
      vs_queue_push(queue,outbuff,sz,sessionid);
      count++;
      free(did);
   }
   }
 }
 outbuff = malloc(512);
 sz = sprintf(outbuff,"Q %d DONE\r\n",count);
 outbuff[sz]=0;
 vs_queue_push(queue,outbuff,sz,sessionid);

 vsc_destroycursor(&cursor);
 

 return(count);
}

