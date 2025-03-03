#include <string.h>
#include <stdlib.h>
#include <stdio.h>     
#include "port.h"
#include "uuid.h"

extern const long _VS_MAX_DIMS;

typedef struct
{
//  t_uuid uniqueidentifier;
  double  floatvalue;
//  long    longvalue;
} vs_value;

typedef struct {
  t_uuid vectorid;  //TODO: <<< IS THIS NEEDED? kinda wasting memory
  t_uuid dimensionid;
  long dimensiontype;
  vs_value value;
} dimension;

typedef struct {
 t_uuid vectorid;
 long dimensioncount;
 long reference;
 double magnitude;
 void *index;
 dimension *dimensions;
} vector;

t_uuid newid();
vector *vs_createvector (t_uuid vectorid,long dimensioncount);
long vs_rawfinddimension (vector *v, t_uuid dimensionid);
long vs_finddimension (vector *v, t_uuid dimensionid);
long vs_setvalue (vector *v, t_uuid dimensionid,vs_value value);
void vs_quickset (vector *v, t_uuid dimensionid,float floatvalue);
long vs_appendvalue (vector *v, t_uuid dimensionid,vs_value value); //if you know we don't have the dim already
long vs_setvaluebyindex (vector *v,long idx,t_uuid dimensionid,vs_value value);
dimension *vs_getvalue(vector *v, t_uuid dimensionid); //returns long=0 if value isn't set
t_uuid vs_getdimensionbyindex (vector *v,long idx);
long vs_clone (vector *src,vector *dst);
long vs_destroyvector (vector **v);

void vs_mergevectors (vector *movablev,vector *staticv);
void vs_mergevectorsweighted (vector *movablev,vector *staticv, float weight);
void vs_mergevectorsweightedaveraged (vector *movablev,vector *staticv, float weight, float avg);

void vs_sumvectors (vector *movablev,vector *staticv);
void vs_sumvectorsweighted (vector *movablev,vector *staticv, float weight);

void vs_diffvectors (vector *movablev,vector *staticv);
void vs_diffvectorsweighted (vector *movablev,vector *staticv, float weight);

float vs_magnitude(vector *v);
float vs_relativemagnitude(vector *v,vector *ref);
float vs_dotproduct(vector *a,vector *b);
float vs_cosine(vector *a,vector *b);
float vs_querycosine(vector *a,vector *b,float *dp);
float vs_relativity(vector *v,vector *ref);
float vs_relativequerycosine(vector *a,vector *b,float *dp);

void vs_clearindex (vector *v);
void vs_reindex (vector *v);

void vs_printvector( vector *v);

void vs_modifyvector( vector *v, float modifier);
void vs_modifydimension( vector *v, t_uuid id, float modifier);

dimension *vs_lowest_dimension( vector *v);
dimension *vs_highest_dimension( vector *v);

vector *vs_gravity(vector *a,vector* b, float gravity, float massMultiplier); 




