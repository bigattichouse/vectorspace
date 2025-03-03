#include <stdlib.h>

/*
void mergevectors (vector *movablev,vector *staticv);
void vsc_removenoise (vs_cursor *source,float threshold, int cluster_minimum_size);
vs_cursor *vsc_recurseclusters (vs_cursor **source,float threshold,int cluster_minimum_size);
vs_cursor *vsc_vectorcluster(vs_cursor *source,float threshold,int cluster_minimum_size);
*/

void mergevectors (vector *movablev,vector *staticv);
vs_cursor *vsc_removenoise (vs_cursor *source,float threshold, int cluster_minimum_size);
vector *vsc_findbestmatch(vs_cursor *source,vector *v,float threshold);
vs_cursor *vsc_recurseclusters (vs_cursor **source,float threshold,int cluster_minimum_size);
vs_cursor *vsc_vector_cluster(vs_cursor *source,int segment,int segmentsize,float threshold,int cluster_minimum_size);
vs_cursor *vsc_vector_metacluster(vs_cursor *source,int segment,int segmentsize,float threshold,int cluster_minimum_size);


