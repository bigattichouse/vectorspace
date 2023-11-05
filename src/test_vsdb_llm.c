#include <stdio.h> 
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <fcntl.h>
#include "src/vs_core.h"
#include "src/vs_list.h"
#include "src/vs_queue.h"
#include "src/vs_cursor.h"
#include "src/vs_query.h"
#include "src/vs_interpreter.h"
#include "src/vs_thesaurus.h"
#include "src/vs_modes.h"
#include "src/vs_persist.h"
#include "src/vs_cache_config.h"

const int _BUFFSIZE = 48;
char buffer[48];
char *__VSDB_CACHE_PATH;

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
            printf("    Loaded. %ld Vectors in Dataset.\n",cursor->count);
  } 
  
  if ((command=='V') && (cursor->count % 2500 == 0)){
            printf("    Loaded. %ld Vectors in Dataset.\n",cursor->count);
  } 
   
  if(command=='S'){
     printf("Saving %ld Vectors to %s \n",cursor->count,__VSDB_CACHE_PATH);
     vp_save_cursor( cursor,(char *) __VSDB_CACHE_PATH);
  }
   
    id = StringToHash(vectorid);

    res = vsinterpreter_execute (cursor,thesaurus,&session,command,id,value,output_buffer,stdout);
    while ( vs_queue_size(output_buffer)>0 ){
      node = vs_queue_pop(output_buffer);
      printf("%s",node->buffer);
    }
    if(res == 2){
                  break;
                };
 
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

    char *cachedir;
    printf("# VSDB Start\n");


	if (argc>1){
          cachedir = argv[1];
          //datafile = argv[1];
        }


               cursor = vsc_createcursor();
               thesaurus = vsc_createcursor();
               
       __VSDB_CACHE_PATH = (char *)malloc(1024);
      strcat(__VSDB_CACHE_PATH,cachedir);
      strcat(__VSDB_CACHE_PATH,"/vsdb.dat");
      printf("Loading vectorspace from %s\n",__VSDB_CACHE_PATH);
/*
      

      __THESAURUS_CACHE_PATH = (char *)malloc(1024);
      strcat(__THESAURUS_CACHE_PATH,cachedir);
      strcat(__THESAURUS_CACHE_PATH,"thesaurus.dat");
*/

      vp_load_cursor( cursor,__VSDB_CACHE_PATH);
      printf("Loaded %ld Vectors.\n", cursor->count);
     // vp_load_cursor( thesaurus,(char *)__THESAURUS_CACHE_PATH);


       output_buffer = vs_prepare_queue();
      session.mode = RAW_VECTOR_MODE;
      session.thesaurus = 0.45;

      process_command ();

      exit(EXIT_SUCCESS);
} /*main*/
