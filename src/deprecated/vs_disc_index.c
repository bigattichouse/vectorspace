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
#include "vs_disc_index.h"


//this IS PURELY a "where the hell is that vector" sort of index.




int __disk_index_filesystem_exists(char *dir_or_file){
 struct stat buffer;
 if (stat(dir_or_file,&buffer)==-1){
     return 0;
 }
 return 1;
}

FILE *__disk_index_open(char *base_directory,char *filename){
  char *hold;
  int len;
  len =strlen(base_directory)+10;
  hold = (char *)malloc(len);
  memset (hold,'\0',len);
  strcat(hold,base_directory);
  strcat(hold,filename);
//  printf("%s\r\n",hold);
  if (!__disk_index_filesystem_exists(hold)){
     return fopen(hold,"w+b");
  }
  return fopen(hold,"r+b");
}

void __disk_index_close(FILE *index){
  if (index) fclose(index);
}

void __disk_set_node(FILE *index,int nodepos,__disk_node node){
  int curr;
  curr = ftell(index);
  fseek(index,nodepos,0);
  fwrite(&node,sizeof(__disk_node),1,index);
  fseek(index,curr,0);
}

__disk_node __disk_get_current_node(FILE *index){
  __disk_node result;
  int i;
  i=fread(&result,sizeof(__disk_node),1,index);
  return result;
}

__disk_node __disk_get_node(FILE *index,int nodepos){
  int curr,i;
  __disk_node result;
  curr = ftell(index);
  fseek(index,nodepos,0);
  i=fread(&result,sizeof(__disk_node),1,index);
  fseek(index,curr,0);
  return result;
}


void __disk_print_node(__disk_node node){
  printf("Node:\r\n");
  printf("  Key %s\r\n",GuidToString(node.keyid));
  printf("    Greater: %d\r\n",node.greater);
  printf("    Lesser:  %d\r\n",node.lesser);
  printf("    Data:    %d\r\n",node.data);
  printf("\r\n");
}


int __disk_querynode_compare (t_uuid query,t_uuid original){
  return (CompareGuids(query,original));
}

int __disk_append_node (FILE *index,__disk_node node){
  int fpos;
  fseek(index,0,SEEK_END);
  fpos = ftell(index);
  fseek(index,fpos,0);
  fwrite(&node,sizeof(__disk_node),1,index);
  return fpos;
}

int __disk_seek_node (FILE *index, t_uuid keyid, int data,int lastnode,int autoinsert,int fsize){
  __disk_node node,newnode;
  int cmp,currnode ;

  if (fsize==0){
      //nothing here folks!
     if (autoinsert){
         newnode.keyid=keyid;
         newnode.greater=-1;
         newnode.lesser=-1;
         newnode.data = data;
         __disk_append_node(index,newnode);
         return data;
     } else {return -1;}

  } else {

  //someone hands me a node with a vector and pos in data file
  //now, I get the current node.
  currnode = ftell(index);
  node = __disk_get_current_node(index);
 // __disk_print_node(node);
  //I then compare
  cmp = __disk_querynode_compare(node.keyid,keyid);
  if (cmp>0){
     //v vectorid is greater than node vectorid
     //printf("    Is Greater\r\n");
     if (node.greater==-1){ //doesn't exist
         if (autoinsert){
         newnode.keyid=keyid;
         newnode.greater=-1;
         newnode.lesser=-1;
         newnode.data = data;
         node.greater = __disk_append_node(index,newnode);
         __disk_set_node(index,currnode,node);
         return data;
         } else {
         return -1;
         }

     } else {
       return __disk_seek_node (index,keyid,data,node.greater,autoinsert,fsize);
     }

  } else {
    if (cmp==0){
        //   printf("    Is Equal\r\n");
     //this is our node!
      if (autoinsert){
        node.data = data;
        __disk_set_node(index,currnode,node);
      }
      return node.data; //kind of anticlimatic, huh?



    } else {
      //printf("    Is Lesser\r\n");
     //v vectorid is lesser than node vectorid
     if (node.lesser==-1){ //doesn't exist
        if (autoinsert){
         newnode.keyid=keyid;
         newnode.greater=-1;
         newnode.lesser=-1;
         newnode.data = data;
         node.lesser = __disk_append_node(index,newnode);
         __disk_set_node(index,currnode,node);
         return data;
         } else {
         return -1;
         }


     } else {
       return __disk_seek_node (index,keyid,data,node.lesser,autoinsert,fsize);
     }
    }
  }
  }
  return -1; //can't find it!
}

int __disk_replace_key_value (FILE *index, t_uuid keyid, int data){
   int index_pos,fsize;
   fseek(index,0,SEEK_END);
   fsize = ftell(index);
   fseek(index,0,0);
   index_pos = __disk_seek_node(index,keyid,data,0,1,fsize);
   return(index_pos);
}


int __disk_fetch_value (FILE *index,t_uuid keyid ){
   int index_pos,fsize;
   fseek(index,0,SEEK_END);
   fsize = ftell(index);
   fseek(index,0,0);
   index_pos = __disk_seek_node(index,keyid,0,0,0,fsize);
   return(index_pos);
}

int __disk_key_exists (FILE *index,t_uuid keyid){
   int index_pos,fsize;
   fseek(index,0,SEEK_END);
   fsize = ftell(index)+1;
   fseek(index,0,0);
   index_pos = __disk_seek_node(index,keyid,0,0,0,fsize);
   return((index_pos!=-1));
}

