#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "vs_core.h"
#include "vs_cache.h"

//cursors are an inmemory "chunk" that helps speed up comparisons.. by loading - say 1000 vectors into memory
//at a time... then allowing faster reads. reads are forward only, but can be reset back to origin.
//the vectors are stored as vector * in memory for fastest access.


vector **_vsc_preparepage (int pagesize){
  vector **v;
  int i;
  v = (vector **)malloc((pagesize) * sizeof(vector **));
  for (i=0;i<pagesize;i++){
    v[i] = NULL;
  }
  return(v);
}

void _vsc_releasepage (vs_cache *cache){
 int i;
 vector *ptr;
 if (cache->page!=NULL){
 for (i=0;i<cache->pagesize;i++){
    ptr = (vector *)cache->page[i];
    if (ptr!=NULL){vs_destroyvector(&ptr);}
 }
 free(cache->page);
 }
}

vs_cache *vsc_prepare (){
  vs_cache *cache;
  cache = malloc(sizeof(vs_cache));
  cache->page = _vsc_preparepage (_VS_MAX_DIMS);
  cache->pagesize=_VS_MAX_DIMS;
  cache->count = 0;
  return(cache);
}


int vsc_destroycache (vs_cache **cache){
  _vsc_releasepage(*cache);
  free(*cache);
  return(1);
}

