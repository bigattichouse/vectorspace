#include <stdlib.h>
#include <math.h>
#include <unistd.h>
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
#include "vs_disc_index.h"
#include "vs_disc_dimensions.h"
#include "vs_disc_interface.h"


int __vdd_vectorscan (__vdi_interface *intf,FILE *fp, t_uuid *dimensionid, t_uuid *vectorid){
 vector *v;
 int count,i,j,ignore ;
 vs_value value;
 char c;
 i=0;
 j=0;
 v = NULL;
 if (!feof(fp)){
   ignore= fread(&c,1,1,fp);
   if (c=='V'){       
     ignore=fread(vectorid,1,sizeof(t_uuid),fp);
     ignore=fread(&count,sizeof(int),1,fp);
     return 1;
   } else {
     ignore=fread(dimensionid,1,sizeof(t_uuid),fp);
     ignore=fread(&value,sizeof(vs_value),1,fp);
   }
   }
  return 0;
}

void __vdi_rebuild_dimensionindex (__vdi_interface *intf){
   t_uuid dimensionid,vectorid;
   FILE *df;
   int i;
   df = __disk_index_open(intf->base_directory,"vsdb.dat");
  if(df){
   fseek(df,0,0);
   i=0;
   while( __vdd_vectorscan(intf,df,&dimensionid,&vectorid)){
    // printf("idx>%d\r\n",i);i++;
     __ddim_insertvectorid (intf->Dindex,dimensionid,vectorid);
   }
   fclose(df);
  }
}


__vdi_interface *__vdi_create_disk_interface(char *base_directory){
  __vdi_interface *result;

  result = malloc(sizeof(__vdi_interface));
  result->data =  _vdd_open(base_directory,"vsdb.dat");
  result->Vindex =  __disk_index_open(base_directory,"vsdb.idx");
  result->Dindex =  __vdd_create_disk_dimensions() ;
  result->base_directory = base_directory;
  __vdi_rebuild_dimensionindex(result);
  return result;
}

void __vdi_close(__vdi_interface **intf){
  __vdi_interface *i;
  i = (*intf);
  _vdd_close(i->data);
  __disk_index_close(i->Vindex);
  __ddim_destroyindex(&i->Dindex);
  free(*intf);
}


 void __vdi_replace_vector(__vdi_interface *intf,vector *v){
  int filepos,newpos;
  filepos = __disk_fetch_value(intf->Vindex,v->vectorid);
  if (filepos!=-1){
    newpos = _vdd_replace(intf->data,filepos,v); //it might move the vector
    if(newpos!=filepos){
      __disk_replace_key_value(intf->Vindex,v->vectorid,filepos);
      __ddim_deletevector(intf->Dindex,v);
      __ddim_insertvector (intf->Dindex,v);
    }
  } else {
      filepos = _vdd_append(intf->data,v);
      __disk_replace_key_value(intf->Vindex,v->vectorid,filepos);
      __ddim_insertvector (intf->Dindex,v);
  }
}





 void __vdi_rawappend_vector(__vdi_interface *intf,vector *v){
  int filepos ;
      filepos = _vdd_append(intf->data,v);
      __disk_replace_key_value(intf->Vindex,v->vectorid,filepos);

}


void  __vdi_delete_vector(__vdi_interface *intf,vector *v){
  int filepos;
  filepos = __disk_fetch_value(intf->Vindex,v->vectorid);
  if (filepos!=-1){
    __disk_replace_key_value (intf->Vindex,v->vectorid,-1);
    __ddim_deletevector(intf->Dindex,v);
    _vdd_delete(intf->data,filepos);
  }
 }



void  __vdi_delete_vectorbyid(__vdi_interface *intf,t_uuid vectorid){
  vector *v;
  int filepos;
  filepos = __disk_fetch_value(intf->Vindex, vectorid);
   v = _vdd_retrieve(intf->data,filepos);
  __ddim_deletevector( intf->Dindex, v);
}

int __vdi_query_load_cursor(__vdi_interface *intf, vector *query, vs_cursor *cursor){
  vs_list *results;
  vector *v;
  vs_listnode *listnode;
  t_uuid vectorid;
  int i,filepos,sz;
  results = __ddim_query(intf->Dindex,query);
  sz = results->size;
  for(i=0;i<sz;i++){
      listnode = vs_list_findbyindex(results,i);
      vectorid = listnode->id;
      filepos =   __disk_fetch_value(intf->Vindex,vectorid);
      v = _vdd_retrieve(intf->data,filepos);
      if (v!=NULL){vsc_rawinsertvector(cursor,v);}
  }

  vs_destroy_list(&results);
  return sz;
}
