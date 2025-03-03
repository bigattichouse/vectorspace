#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include "vs_core.h"
#include "vs_list.h"
#include "vs_queue.h"
#include "vs_cache.h"
#include "vs_cursor.h"
#include "vs_query.h"

/*
1. We create a "thesaurus" cursor.
2. Each "Vector" is a higher level item.. so md5("Dog") is a VectorID
3. Each sub term is a dimension ("Terrier","Hound",etc)
4. Before we Query, or insert a vector into our main cursor, we query the thesaurus.
5. for each value returned, we add a dimension to our vector the value of which was the matching value from #4
6. Now, when we query our main cursor, we get back a match between "Terrier and Hound" instead of 0.
*/

int vst_classify_vector (vector *v, vs_cursor *thesaurus, float threshold) {
 float dbcos,dp,magd;
 vs_value val;
 dimension *dim;
 int count;
 vector *classification;
 vsc_reset(thesaurus);
 count = 0;
 while (!vsc_eof(thesaurus)){
 /*Query vector against thesaurus*/
   classification = vsc_readnext(thesaurus);
   if (classification!=NULL){
   magd = vs_relativity(classification,v);
   dbcos = vs_relativequerycosine(v,classification,&dp); //magnitudes are already calc'ed by storage load and above.
     if (dbcos>threshold){
        val.floatvalue=dbcos * magd;
        dim = vs_getvalue(v,classification->vectorid);
        if (dim!=NULL){val.floatvalue += dim->value.floatvalue;}  //in case we have that dimension!
        vs_setvalue (v,classification->vectorid,val);
        count+=1;
     }
   }
  }  /*Thats it... the normal query/vector system will handle it from here!*/
 return(count);
}

vector *vst_metavector (vector *v, vs_cursor *thesaurus, float threshold) {
 float dbcos,dp,magd;
 vs_value val;
 dimension *dim;
 int count;
 vector *classification,*result;
 vsc_reset(thesaurus);
 result = vs_createvector (v->vectorid,1);
 result->dimensioncount=0;
 count = 0;
 while (!vsc_eof(thesaurus)){
 /*Query vector against thesaurus*/
   classification = vsc_readnext(thesaurus);
   if (classification!=NULL){
   magd = vs_relativity(classification,v);
   dbcos = vs_relativequerycosine(v,classification,&dp); //magnitudes are already calc'ed by storage load and above.
     if (dbcos>threshold){
        val.floatvalue=dbcos * magd;
        dim = vs_getvalue(v,classification->vectorid);
        if (dim!=NULL){val.floatvalue += dim->value.floatvalue;}  //in case we have that dimension!
        vs_setvalue (result,classification->vectorid,val);
        count+=1;
     }
   }
  }  /*Thats it... the normal query/vector system will handle it from here!*/
 return(result);
}
