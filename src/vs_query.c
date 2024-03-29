#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "vs_core.h"
#include "vs_queue.h"
#include "vs_list.h"
#include "vs_cursor.h"
#include "vs_indexrowset.h"


void vsdb_outputcursor(t_uuid sessionid,vs_cursor *cursor,vs_queue *queue){
  char *outbuff;
  char *did;
  long sz,count,i;
  vector *v;
  dimension *dim;
  t_uuid id;
    outbuff = malloc(512);
    sz = sprintf(outbuff, "C 00000000000000000000000000000000 %ld\r\n",cursor->count);
    outbuff[sz]=0;
    vs_queue_push(queue,outbuff,sz,sessionid);
// printf("C>reset:\n");
 vsc_reset(cursor);
// printf("C>print:\n");
 while (!vsc_eof(cursor)){
    v = vsc_readnext(cursor);
 //   printf("C>next:\n");
    did = HashToString(v->vectorid);
    outbuff = malloc(512);
    sz = sprintf(outbuff, "V %s %ld\r\n",did,v->dimensioncount);
    outbuff[sz]=0;
    vs_queue_push(queue,outbuff,sz,sessionid);
    free(did);
    count=v->dimensioncount;
    for (i=0;i<count;i++){
       id = vs_getdimensionbyindex(v,i);
       dim = vs_getvalue(v,id);
       did = HashToString(id);
       if (dim!=NULL){
          outbuff = malloc(512);
          sz = sprintf(outbuff, "D %s %f\r\n",did,dim->value.floatvalue);
          outbuff[sz]=0;
       } else {
  //      printf("C> %s not found\n",did);
       }
       free(did);
       vs_queue_push(queue,outbuff,sz,sessionid);
    }
 }
       outbuff = malloc(512);
       sz = sprintf(outbuff,"C %ld DONE\r\n",cursor->count);
       outbuff[sz]=0;
       vs_queue_push(queue,outbuff,sz,sessionid);

}


long vsdb_query(t_uuid sessionid,vs_cursor *cursor,vector *query,float threshold,vs_queue *queue){
 vector *v;
 float dbcos,dp,magd;
 long count;
 char *did;
 char *outbuff;
 long sz;
 vs_indexrowset *rowset;
 count=0;
 rowset = vsi_createrowset(cursor,query);
 vsi_reset(rowset);
 while (vsi_eof(rowset)==0){
   v = vsi_readnext(rowset);
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
 sz = sprintf(outbuff,"Q %ld DONE\r\n",count);
 outbuff[sz]=0;
 vs_queue_push(queue,outbuff,sz,sessionid);
 vsi_destroyrowset(&rowset);
 return(count);
}
