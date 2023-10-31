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
#include "vs_tensor.h"


vector *vsdb_tensor(vs_cursor *cursor,vector *query,float threshold){
 vector *v, *output;
 float dbcos,dp ;
 long count; 
 vs_indexrowset *rowset;
 count=0; 
 output = vs_createvector (query->vectorid,0);
 rowset = vsi_createrowset(cursor,query);
 vsi_reset(rowset);
 while (vsi_eof(rowset)==0){
   v = vsi_readnext(rowset);
   if (v!=NULL){
   //magd = vs_relativity(v,query);
   dbcos = vs_relativequerycosine(query,v,&dp); //magnitudes are already calc'ed by storage load and above.
   if (dbcos>threshold){
       if(count==0){
          vs_clone (v,output);
       } else {
          vs_sumvectorsweighted (output,v,dbcos);
      }
      count++;
   }
   //now divide output by count for average
   }
 }
 //printf(" %ld vectors merged.\n",count);
 if (count>0) vs_modifyvector( output, 1 / (float) count);
 vsi_destroyrowset(&rowset);
 return(output);
}



void vsdb_tensor_remove_query(vector *tensor,vector *query, float modifier){
      vs_sumvectorsweighted (tensor,query,modifier);
}
