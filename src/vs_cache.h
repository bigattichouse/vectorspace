#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
 vector **page;
 int pagesize;
 int count;
} vs_cache;

vs_cache *vsc_prepare  ();
int vsc_destroycache (vs_cache **cache);
 


