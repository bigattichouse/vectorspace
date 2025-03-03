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


int main (argc, argv)
int argc;
char *argv[];
{

    char *dbfile,*inputfile;
    printf("# VSDB Start\n");

	if (argc>1){
          inputfile = argv[1];
          dbfile = argv[2];
        }


      cursor = vsc_createcursor();
      printf("Loading vectorspace from %s into %s\n",inputfile,dbfile);

      vp_load_cursor( cursor, dbfile);
      printf("Loaded %ld Vectors from %s.\n", cursor->count, dbfile);
      long count;
      count = cursor->count;
      printf("Loading text Vectors from %s.\n", inputfile);
      vp_load_text_cursor (cursor, inputfile);
      printf("Loaded %ld Vectors from %s.\n", (cursor->count - count), dbfile);
  
      printf("Saving %ld Vectors to %s... \n",cursor->count,dbfile);
      vp_save_cursor( cursor,dbfile);

      exit(EXIT_SUCCESS);
} /*main*/
