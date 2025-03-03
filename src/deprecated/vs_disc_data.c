#include <stdio.h>
#include <sys/io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include "vs_core.h"
#include "vs_list.h"
#include "vs_queue.h"
#include "vs_cache.h"
#include "vs_cursor.h" 
#include "vs_query.h"
#include "vs_cluster.h"
#include "vs_thesaurus.h"
#include "vs_modes.h"
#include "vs_persist.h"
#include "vs_disc_data.h"



int __vdd_index_filesystem_exists(char *dir_or_file){
 struct stat buffer;
 if (stat(dir_or_file,&buffer)==-1){
     return 0;
 }
 return 1;
}

FILE *_vdd_open(char *base_directory,char *filename){
  char *hold;
  int len;
  len =strlen(base_directory)+10;
  hold = (char *)malloc(len);
  memset (hold,'\0',len);
  strcat(hold,base_directory);
  strcat(hold,filename);
//  printf("%s\r\n",hold);
  if (!__vdd_index_filesystem_exists(hold)){
     return fopen(hold,"w+b");
  }
  return fopen(hold,"r+b");
}

void _vdd_close(FILE *index){
  if (index) fclose(index);
}

vector *_vdd_retrieve (FILE *data,int filepos){
  fseek(data,filepos,0);
  return (vp_readvector (data));
}


void _vdd_delete (FILE *data,int filepos){
  vector *v;
  fseek(data,filepos,0);
  v = vp_readvector (data);
  fseek(data,filepos,0);
  vp_clearvector(data,v);
  vs_destroyvector(&v);
}

int _vdd_append(FILE *data,vector *v){
  int fpos;
 fseek(data,0,SEEK_END);
 fpos = ftell(data); 
 vp_writevector (data,v);
 return fpos;
}


int _vdd_replace (FILE *data,int filepos,vector *v){
  vector *v2;
  int dims;
  v2 = _vdd_retrieve(data,filepos);
  dims = v2->dimensioncount;
  vs_destroyvector(&v2);

  if(dims==v->dimensioncount){
    fseek(data,filepos,0);   //prevents fragmentation when vector is identical in size
    vp_writevector (data,v); //just writes any changes
    return filepos;
  } else {
    _vdd_delete(data,filepos);
    return _vdd_append(data,v);
  }
}



