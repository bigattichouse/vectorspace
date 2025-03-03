#include "sockhelp.h"
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <fcntl.h>      
#include <sched.h>
#include "vs_core.h"
#include "vs_list.h"
#include "vs_queue.h"
#include "vs_cursor.h"
#include "vs_query.h"
#include "vs_thesaurus.h"
#include "vs_modes.h"
#include "vs_persist.h"
#include "vs_cache_config.h"
#include "vs_disc_dimensions.h" 
#include "vs_disc_interface.h"


int vp_load_text_to_interface (__vdi_interface *intf, char *filename){
 FILE *fp;
 vector *v;
 t_uuid id;
 char *vectorid ;
 float value;
 char command;
 char *buffer;
 int count,i,pos,vcount,ignore;
 char c;
 vs_value val;
 i=0;
 pos=0; count=0;  vcount=0;
 buffer = (char *)malloc(255);
 vectorid = (char *)malloc(32);
 memset(buffer,0,255);
 fp = fopen(filename,"r");
 if (fp){
 while (!feof(fp)){
    ignore=fread(&c,1,1,fp);
    buffer[pos]=c;
    pos++;
  //  printf("%s!\n",buffer);
  if ((c==13) || (c==10)){
  if (sscanf(buffer,"%c %32s %f",&command,vectorid,&value)==3){
   //printf("%s!\n",buffer);
   pos=0;
   id = StringToHash(vectorid);
   if (command=='V'){
     v=vs_createvector( id,value);
     count=value;
     i=0;
   }
   if (command=='D'){
     val.floatvalue= value;
     vs_setvalue (v, id, val);
     i++;
     if (i==count){
        __vdi_rawappend_vector(intf,v);
        vs_destroyvector(&v);
        vcount++;

        if(vcount % 10  == 0){ printf("  %d\r\n",vcount);}
     }
   }
  }
  pos=0;
  memset(buffer,0,255);

 }}
 fclose(fp);
 }
 return(0);
}

int main (argc, argv)
int argc;
char *argv[];
{
  vs_cursor *cursor;
  __vdi_interface *intf;
  int i;

   cursor = vsc_createcursor();

   vsc_reset(cursor);
   i=0; printf("Importing %s into %s\r\n",argv[1],argv[2]);
   intf = __vdi_create_disk_interface(argv[2]);
   vp_load_text_to_interface (intf,argv[1]);

  exit(EXIT_SUCCESS);
} /*main*/


