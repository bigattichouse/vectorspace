#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "port.h" 
#include "vs_core.h"
#include "vs_queue.h"
#include "vs_output.h"

void vs_queued_printvector(t_uuid sessionid, vector *v, vs_queue *queue ){
  char *did;
  char *outbuff;
  long count,i,sz;
  dimension *dim;
  t_uuid id;

    did = HashToString(v->vectorid);
    outbuff = malloc(512);
    sz = sprintf(outbuff,  "V %s %ld\r\n",did,v->dimensioncount);
    outbuff[sz]=0;
    vs_queue_push(queue,outbuff,sz,sessionid);
    free(did);
    count=v->dimensioncount;
    for (i=0;i<count;i++){
       id = vs_getdimensionbyindex(v,i);
       dim = vs_getvalue(v,id);
       did = HashToString(id);
       outbuff = malloc(512);
       sz = sprintf(outbuff,  "D %s %f\r\n",did,dim->value.floatvalue);
       outbuff[sz]=0;
       vs_queue_push(queue,outbuff,sz,sessionid);
       free(did);
    }

}
