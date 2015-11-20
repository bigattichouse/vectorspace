#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "vs_core.h"
#include "vs_list.h"
#include "vs_cursor.h"
#include "vs_rowset.h"
#include "vs_cluster.h"
#include "vs_normalize.h"
#include "vs_cursorindex.h"


//

double vs_term_freqency(vector *v, t_uuid dimensionid){
  int max,i,val;
  double term_total, document_total;
  dimension *dim,*dims; 
  t_uuid  v_dimensionid;
  vs_value value;

  dim = vs_getvalue(v,dimensionid);
  term_total=0;
  if(dim!=NULL){
    value = dim->value; 
    term_total = value.floatvalue;
  }
  document_total = 0;
  max = v->dimensioncount;
  i=0;  
  dims = v->dimensions; 
  while (i<max){
    val = (int)dims + (int)(i * sizeof(dimension));
    dim = (dimension *)(val);
    v_dimensionid = dim->dimensionid;
    value = dim->value;
    if(!IsEqualGuid(dimensionid,v_dimensionid)){
        document_total+=value.floatvalue;
    }
    i++;
  }
 if(document_total==0){
   return 0;
 } 
  return (term_total / document_total );
}

vector *vs_create_idf_vector(vs_cursor *source, vector *v){
  int max,i,val;
  double a,document_frequency,document_count,idf;
  dimension *dim,*dims;
  vs_value value;
  vector *v_out;
   __cdim_index *index;
  t_uuid dimensionid;
  max = v->dimensioncount;
  i=0; a=0;
  v_out = vs_createvector(v->vectorid,v->dimensioncount);
  dims = v->dimensions;
   index = (__cdim_index *)source->index; 
   document_count = (double) source->count;
  while (i<max){
    val = (int)dims + (int)(i * sizeof(dimension));
    dim = (dimension *)(val);
    dimensionid = dim->dimensionid;
    value = dim->value;
    document_frequency = (double) __cdim_dimension_frequency (index, dimensionid);
    idf = log( document_count / document_frequency);
    printf("docs:%f Freq:%f IDF:%f\r\n",document_count,document_frequency,idf);
    value.floatvalue = value.floatvalue * idf;
    value.floatvalue = value.floatvalue * vs_term_freqency(v,dimensionid); //http://en.wikipedia.org/wiki/Tf%E2%80%93idf
    vs_setvalue(v_out,dimensionid,value); 
    i++;
  }
  return (v_out);
}

vs_cursor *vs_create_idf_cursor (vs_cursor *source){
   vs_cursor *Output,*view;
   vector *v,*idf_v; 

   Output = vsc_createcursor();   view = vsc_create_view(source);
   vsc_reset(view);
  
   while (!vsc_eof (view)){
       v = vsc_readnext(view); 
       idf_v = vs_create_idf_vector(source,v);
       vsc_rawinsertvector(Output,idf_v);
   } 

   vsc_indexcursor(Output);
   vsc_destroycursor(&view);
   return (Output);
}



