#include <string.h>
#include <stdlib.h>
#include <stdio.h>

long __vdim_createvectorindex (vector *v);
void __vdim_clearvectorindex (vector *v);
void __vdim_reindexvector(vector *v);
void __vdim_indexdimension (vector *v, dimension *dim, long position)  ;

long __vdim_find (vector *v,t_uuid dimensionid);
dimension *__vdim_find_dimension (vector *v,t_uuid dimensionid);
