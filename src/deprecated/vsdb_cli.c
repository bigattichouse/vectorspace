#include <stdio.h>
#include "sockhelp.h"
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <fcntl.h>
#include "vs_core.h"
#include "vs_list.h"
#include "vs_queue.h"
#include "vs_cursor.h"
#include "vs_query.h"
#include "vs_interpreter.h"
#include "vs_thesaurus.h"
#include "vs_modes.h"
#include "vs_persist.h"
#include "vs_cache_config.h"

const int _BUFFSIZE = 48;
char buffer[48];


 vs_cursor *cursor,*thesaurus;
 vs_queue *output_buffer;
 interpreter_session session;

void process_command (){
  vs_queuenode *node;
 char *vectorid ;
 float value;
 char command;
 int res;
 t_uuid id;
 vectorid=malloc(33);
while (fgets(buffer,_BUFFSIZE,stdin) != NULL ){
  if (sscanf(buffer,"%c %32s %f\n",&command,vectorid,&value)==3){
  if (command=='~'){
           vp_save_cursor( cursor,(char *) __VSDB_CACHE_PATH);
           vp_save_cursor( thesaurus,(char *) __THESAURUS_CACHE_PATH);
  } else {
    id = StringToHash(vectorid);

    res = vsinterpreter_execute (cursor,thesaurus,&session,command,id,value,output_buffer,stdout);
    while ( vs_queue_size(output_buffer)>0 ){
      node = vs_queue_pop(output_buffer);
      printf("%s\n",node->buffer);
    }
    if(res == 2){
                  break;
                };
  }
 } else {
    printf("Command Failed.");
 }
}
 free(vectorid);
 command=' ';
 value=0.0;
}


#define BUFSIZE          32*1024-1

int main (argc, argv)
int argc;
char *argv[];
{
    printf("# VSDB Start\n");

	char *cachedir;
	if (argc>1){
          cachedir = argv[1];
        }


               cursor = vsc_createcursor();
               thesaurus = vsc_createcursor();

      __VSDB_CACHE_PATH = (char *)malloc(1024);
      strcat(__VSDB_CACHE_PATH,cachedir);
      strcat(__VSDB_CACHE_PATH,"vsdb.dat");

      __THESAURUS_CACHE_PATH = (char *)malloc(1024);
      strcat(__THESAURUS_CACHE_PATH,cachedir);
      strcat(__THESAURUS_CACHE_PATH,"thesaurus.dat");

      vp_load_cursor( cursor,(char *) __VSDB_CACHE_PATH);
      vp_load_cursor( thesaurus,(char *)__THESAURUS_CACHE_PATH);


       output_buffer = vs_prepare_queue();
      session.mode = RAW_VECTOR_MODE;
      session.thesaurus = 0.45;

      process_command ();

      exit(EXIT_SUCCESS);
} /*main*/
