#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include "vs_core.h"
#include "vs_list.h"
#include "vs_cursor.h"

/*

dump a cursor/thesaurus to disk and reload again... dumps similar to interpreter commands, but uses binary
for better storage.

*/

int vp_save_cursor (vs_cursor *cursor, char *filename){
 FILE *fp;
 vector *v;
 dimension *dim;
 t_uuid id;
 vs_value value;
 char *v_buffer,*d_buffer ;
 int count,i ;
 i=0;

 v_buffer = malloc(1+sizeof(t_uuid)+sizeof(int));
 d_buffer = malloc(1+sizeof(t_uuid)+sizeof(vs_value));

 fp = fopen(filename,"wb");
 vsc_reset(cursor);
 if (fp){
 fseek ( fp , 0 , SEEK_SET );

 while (!vsc_eof(cursor)){
    v = vsc_readnext(cursor);

    id = v->vectorid;
    count=v->dimensioncount;

    v_buffer[0] = 'V';//fwrite("V",1,1, fp);
    memcpy(&v_buffer[1],&id,sizeof(t_uuid)); //fwrite(&id,sizeof(t_uuid),1, fp);
    memcpy(&v_buffer[1+sizeof(t_uuid)],&count,sizeof(int)); //fwrite(&count,sizeof(int),1, fp);
    fwrite(v_buffer,1+sizeof(t_uuid)+sizeof(int) ,1,fp);

    for (i=0;i<count;i++){
       id = vs_getdimensionbyindex(v,i);
       dim = vs_getvalue(v,id);
       value=dim->value;
       d_buffer[0] = 'D';
       memcpy(&d_buffer[1],&id,sizeof(t_uuid));
       memcpy(&d_buffer[1+sizeof(t_uuid)],&value,sizeof(vs_value));
       fwrite(d_buffer,1+sizeof(t_uuid)+sizeof(vs_value) ,1,fp);

    }
 }
 fflush(fp);
 fclose(fp);
 }
 return(0);
}

int vp_load_cursor (vs_cursor *cursor, char *filename){
 FILE *fp;
 vector *v;
 t_uuid id;
 vs_value value;
 int count,i,j,sz;
 long pos;
 char c;
 i=0;
 j=0;
 pos=0;
 fp = fopen(filename,"rb");
 if (fp){
 fseek ( fp , 0 , SEEK_SET );
 while (!feof(fp)){
  // if (i % 1000 == 0){usleep(20);}  //let's not get completely greedy with the CPU
   sz = fread(&c,1,1,fp); pos+=sz;
   if(sz==1){
   if (c=='V'){
     sz = fread(&id,sizeof(t_uuid),1,fp)*sizeof(t_uuid);pos+=sz;
     sz+= fread(&count,sizeof(int),1,fp)*sizeof(int);pos+=sz;
     if(sz!=sizeof(t_uuid)+sizeof(int)){
        printf("Error: Could not read UUID/vector count @ %ld, read %d expected %ld.\n",
        pos,sz,sizeof(t_uuid)+sizeof(int));exit(1);
     }
     //printf("%s %d\r\n",HashToString(id),count);
     v=vs_createvector(id,count);
     i=0;  j++;
     //if (j % 100 ==0){printf("%d - %d\r\n",j,count);}
   }
   if (c=='D'){
     sz=fread(&id,sizeof(t_uuid),1,fp)*sizeof(t_uuid);pos+=sz;
     sz+=fread(&value,sizeof(vs_value),1,fp)*sizeof(vs_value);pos+=sz;
     if(sz!=sizeof(t_uuid)+sizeof(vs_value)){printf("Error: Could not read UUID/dimensions @ %ld.\n",pos);exit(1);}
     vs_setvalue (v, id,value);
     i++;
     //printf("%s %f\r\n",HashToString(id),value.floatvalue);
     if (i==count){
        vsc_rawloadvector(cursor,v);
        vs_destroyvector(&v);
     }
   }
   }
 }
 fclose(fp);
 }
 return(0);
}

vector *vp_readvector (FILE *fp){
 vector *v;
 t_uuid id;
 vs_value value;
 int count,i,j,sz;
 long pos;
 char c;
 i=0;
 j=0;
 pos=0;
 v = NULL;
 while (!feof(fp)){
  // if (i % 1000 == 0){usleep(20);}  //let's not get completely greedy with the CPU
   sz=fread(&c,1,1,fp);
   if(sz!=1){printf("Error: Could not read Vector command.\n");exit(1);}
   if (c=='V'){
     sz=fread(&id,sizeof(t_uuid),1,fp);
     sz+=fread(&count,sizeof(int),1,fp);
     if(sz!=sizeof(t_uuid)+sizeof(int)){printf("Error: Could not read UUID/vector count @ %ld.\n",pos);exit(1);}
    // printf(" %d dimensions\r\n",count);
     v=vs_createvector(id,count);
     i=0;  j++;
     //if (j % 100 ==0){printf("%d - %d\r\n",j,count);}
   }
   if (c=='D'){
     sz=fread(&id,sizeof(t_uuid),1,fp);
     sz+=fread(&value,sizeof(vs_value),1,fp);
     if(sz!=sizeof(t_uuid)+sizeof(vs_value)){printf("Error: Could not read UUID/dimensions @ %ld.\n",pos);exit(1);}
     vs_setvalue (v, id,value);
     i++;
     if (i==count){
         return(v);
     }
   }
 }
 return(v);
}


void vp_writevector (FILE *fp,vector *v){
 t_uuid id;
 vs_value value;
 int count,i;
 dimension *dim;
 i=0;
    id = v->vectorid;
    count=v->dimensioncount;

    fwrite("V",1,1, fp);
    fwrite(&id,sizeof(t_uuid),1, fp);
    fwrite(&count,sizeof(int),1, fp);

    for (i=0;i<count;i++){
       id = vs_getdimensionbyindex(v,i);
       dim = vs_getvalue(v,id);
       value=dim->value;
       fwrite("D",1,1, fp);
       fwrite(&id,sizeof(t_uuid),1, fp);
       fwrite(&value,sizeof(vs_value),1, fp);
    }

}

void vp_clearvector (FILE *fp,vector *v){
 t_uuid id;
 vs_value value;
 int count,i;
 i=0;
    id = NullGuid();
    count=0;

    fwrite("\0",1,1, fp);
    fwrite(&id,sizeof(t_uuid),1, fp);
    fwrite(&count,sizeof(int),1, fp);

    for (i=0;i<v->dimensioncount;i++){
       id = NullGuid(); 
       value.floatvalue=0;
       fwrite("\0",1,1, fp);
       fwrite(&id,sizeof(t_uuid),1, fp);
       fwrite(&value,sizeof(vs_value),1, fp);
    }

}

int vp_load_text_cursor (vs_cursor *cursor, char *filename){
 FILE *fp;
 vector *v;
 t_uuid id;
 char *vectorid ;
 float value;
 char command;
 char *buffer;
 int count,i,pos,sz,vectors;
 char c;
 vs_value val;
 i=0;vectors=0;
 pos=0; count=0;
 buffer = (char *)malloc(255);
 vectorid = (char *)malloc(32);
 memset(buffer,0,255);
 fp = fopen(filename,"r");
 if (fp){
 while (!feof(fp)){
    sz=fread(&c,1,1,fp);
    if(sz!=0){
    buffer[pos]=c;
    pos++;
 //    printf("%s!\n",buffer);
  if ((c==13) || (c==10)){
  if (sscanf(buffer,"%c %32s %f",&command,vectorid,&value)==3){
 //   printf("%s!\n",buffer);
   pos=0;
   id = StringToHash(vectorid);
   if (command=='V'){
     v=vs_createvector( id,value);
     count=value;
     vectors+=1;
     if(vectors%2500==0){ printf("   %d Vectors loaded.\n",vectors); }
     i=0;
   }
   if (command=='D'){
     val.floatvalue= value;
     vs_setvalue (v, id, val);
     i++;
     if (i==count){
        vsc_rawloadvector(cursor,v);
        vs_destroyvector(&v);
     }
   }
  }
  pos=0;
  memset(buffer,0,255);

 }}
 }
 fclose(fp);
 }
 return(0);
}
