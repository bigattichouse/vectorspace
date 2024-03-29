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

typedef struct {
 int sock;
 char buffer[48];
 int bufferpos;
 int closewhencomplete;
 vs_queue *output_buffer;
 interpreter_session session;
 t_uuid sessionid;
} vs_connection;

vs_cursor *cursor,*thesaurus;

int debug_mode;
FILE 	*logfile;
int sock;            /* The socket file descriptor for our "listening"
                   	socket */
const int TOTALSOCKS = 200;
vs_connection connectlist[200];  /* Array of connected sockets so we know who
	 		we are talking to */
fd_set socks;        /* Socket file descriptors we want to wake
			up for, using select() */
int highsock;	     /* Highest #'d file descriptor, needed for select() */

/* Our process ID and Session ID */
pid_t pid, sid;




void process_command (vs_connection *connection){
 char *vectorid ;
 float value;
 char command;
 int res;
 t_uuid id;
 vectorid=malloc(33);

/*
 TODO!!!   AFter a Query runs, the cursor is blank? The next query will only return 0
 even if we are updating/inserting new records
 seems separate connections are ok, but multiple queries on one session?
*/

 if (sscanf(connection->buffer,"%c %32s %f\r\n",&command,vectorid,&value)==3){
  if (command=='~'){
           vp_save_cursor( cursor,(char *) __VSDB_CACHE_PATH);
           vp_save_cursor( thesaurus,(char *) __THESAURUS_CACHE_PATH);
  } else {
    id = StringToHash(vectorid);
    res = vsinterpreter_execute (cursor,thesaurus,&connection->session,command,id,value, connection->output_buffer, logfile);
    if(res == 2){ connection->closewhencomplete=1; };
  }

 } else {fprintf(logfile,"command failed. %s\n",connection->buffer);fflush(logfile);}
 free(vectorid);
 command=' ';
 value=0.0;
 memset(connection->buffer,0,sizeof(connection->buffer));
}


void setnonblocking(sock)
int sock;
{
	int opts;

	opts = fcntl(sock,F_GETFL);
	if (opts < 0) {
		fprintf(logfile,"fcntl(F_GETFL)\n"); fflush(logfile);
		exit(EXIT_FAILURE);
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(sock,F_SETFL,opts) < 0) {
		fprintf(logfile,"fcntl(F_SETFL)\n"); fflush(logfile);
		exit(EXIT_FAILURE);
	}
	return;
}

void build_select_list() {
	int listnum;	     /* Current item in connectlist for for loops */
	FD_ZERO(&socks);
	FD_SET(sock,&socks);
        highsock=0;
	for (listnum = 0; listnum < TOTALSOCKS; listnum++) {
		if (connectlist[listnum].sock != 0) {
			FD_SET(connectlist[listnum].sock,&socks);
			if (connectlist[listnum].sock > highsock)
				highsock = connectlist[listnum].sock;
		}
	}
        if (sock>highsock) highsock=sock;
}

void handle_new_connection() {
	int listnum;	     /* Current item in connectlist for for loops */
	int connection; /* Socket file descriptor for incoming connections */
      //  int window_size;
	/* We have a new connection coming in!  We'll
	try to find a spot for it in connectlist. */
	connection = accept(sock, NULL, NULL);
	if (connection < 0) {
		fprintf(logfile,"accept\n"); fflush(logfile);
		exit(EXIT_FAILURE);
	}

	setnonblocking(connection);
       // window_size = 512;
	//setsockopt(connection, SOL_SOCKET, SO_SNDBUF,(char *) &window_size, sizeof(window_size));

	for (listnum = 0; (listnum < TOTALSOCKS) && (connection != -1); listnum ++)
		if (connectlist[listnum].sock == 0) {
	if (debug_mode==1){	fprintf(logfile,"\nConnection accepted:   FD=%d; Slot=%d\n", connection,listnum); fflush(logfile);}
			connectlist[listnum].sock = connection;
			connectlist[listnum].closewhencomplete   =0;
			connectlist[listnum].session.mode = RAW_VECTOR_MODE;
			connectlist[listnum].session.thesaurus = 0.45;
			memset(connectlist[listnum].buffer, 0, sizeof(connectlist[listnum].buffer));
			connectlist[listnum].bufferpos=0;
			vs_clear_queue(connectlist[listnum].output_buffer);
			connection = -1;
		}
	if (connection != -1) {
		/* No room left in the queue! */
		fprintf(logfile,"\nNo room left for new client.\n"); fflush(logfile);
		sock_puts(connection,"E 1 Server Busy\r\n");
		close(connection);
	}
}

void deal_with_data( int listnum) {
    char *buff;

    vs_connection *connection;
    int sz ;
    int holdbuff;
    connection = &connectlist[listnum];

    buff = (char *)(connection->buffer);
    buff += connection->bufferpos;
    sz = read (connection->sock,buff,(47-(connection->bufferpos)));
     if (sz>0) {
         holdbuff = connection->bufferpos;
         holdbuff+=sz;
         connection->bufferpos = holdbuff;
         if (connection->bufferpos >=47){
         if (debug_mode==1){ fprintf(logfile,"RCV:%s",connection->buffer);fflush(logfile);}
          process_command (connection);
          memset(connection->buffer, 0, sizeof(connection->buffer));
          connection->bufferpos = 0;
          if (debug_mode==1){fprintf(logfile,"PROCESSED\n");fflush(logfile);}

        }
     }
     if(sz==0){
        /*socket is closed, so clean up.*/
        if (debug_mode==1){fprintf(logfile,"Closing %d\n",connection->sock);fflush(logfile);}
        memset(connection->buffer, 0, sizeof(connection->buffer));
        //for(sz=0;sz<47;sz++){connection->buffer[sz]=0;}
        FD_CLR(connection->sock,&socks);
        close(connection->sock);
        connection->bufferpos=0;
        connection->session.mode = RAW_VECTOR_MODE;
        connection->session.thesaurus = 0.45;
        connection->sock=0;
        connection->closewhencomplete=0;
      }
}


void read_socks() {
	int listnum;	     /* Current item in connectlist for for loops */

	/* OK, now socks will be set with whatever socket(s)
	   are ready for reading.  Lets first check our
	   "listening" socket, and then check the sockets
	   in connectlist. */

	/* If a client is trying to connect() to our listening
		socket, select() will consider that as the socket
		being 'readable'. Thus, if the listening socket is
		part of the fd_set, we need to accept a new connection. */

	if (FD_ISSET(sock,&socks)) {
		handle_new_connection();
	/* Now check connectlist for available data */
	 } else {
	/* Run through our sockets and check to see if anything
		happened with them, if so 'service' them. */

	for (listnum = 0; listnum < TOTALSOCKS; listnum++) {
		if (FD_ISSET(connectlist[listnum].sock,&socks))
			deal_with_data(listnum);
	} /* for (all entries in queue) */
	}
}


void open_logfile(){
  logfile = fopen("/var/log/vsdbd/vsdbd.log", "a+");
  freopen("/var/log/vsdbd/vsdbd.log", "a+",logfile); //force line buffered
  fprintf(logfile,"Logfile Open\n"); fflush(logfile);
}

void daemonize_process(){
        /* Fork off the parent process */
        pid = fork();
        if (pid < 0) {
                exit(EXIT_FAILURE);
        }
        /* If we got a good PID, then
           we can exit the parent process. */
        if (pid > 0) {
                exit(EXIT_SUCCESS);
        }

        /* Change the file mode mask */
        umask(0);

        /* Open any logs here */
        open_logfile();
        /* Create a new SID for the child process */
        sid = setsid();
        if (sid < 0) {
                /* Log the failure */
                exit(EXIT_FAILURE);
        }


        /* Change the current working directory
        if ((chdir("/opt/lsb-bigattichouse/bin/")) < 0) {
                exit(EXIT_FAILURE);
        }
        */

        /* Close out the standard file descriptors */
         close(STDIN_FILENO);
        close(STDOUT_FILENO);
       close(STDERR_FILENO);
 }



#define BUFSIZE          32*1024-1

int main (argc, argv)
int argc;
char *argv[];
{
  int i,writeable;
  char *errpos;
  vs_connection *connection;
  vs_queuenode *node;
        int bufsize;
	char *ascport;  /* ASCII version of the server port */
	char *cachedir;
	int port;       /* The port number after conversion from ascport */
	struct sockaddr_in server_address; /* bind info structure */
	int reuse_addr = 1;  /* Used so we can re-bind to our port
				while a previous connection is still
				in TIME_WAIT state. */
	struct timeval timeout;  /* Timeout for select */
	int readsocks;	     /* Number of sockets ready for reading */

     debug_mode=0; //daemonize_process();
       open_logfile();  // debug_mode=1;

        fprintf(logfile,"Preparing Server\n"); fflush(logfile);
	/* Make sure we got a port number as a parameter */
	if (argc < 2) {
		fprintf(logfile,"Usage: %s [PORT] [Data directory] [debug]\r\n",argv[0]);  fflush(logfile);
		exit(EXIT_FAILURE);
	}
        cachedir=NULL;
        if (argc>3){
          if (strcmp(argv[3],"debug")==0){
             debug_mode=1;
          }
        }
        if (argc>2){
          cachedir = argv[2];
        }

	/* Obtain a file descriptor for our "listening" socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
	if (debug_mode==1){	fprintf(logfile,"socket\n");  fflush(logfile);  }
		exit(EXIT_FAILURE);
	}

	/* So that we can re-bind to it without TIME_WAIT problems */
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,
		sizeof(reuse_addr));
       bufsize=BUFSIZE;
       if ((setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize))) == -1) {
         if (debug_mode==1){      fprintf(logfile,"pitch - setsockopt failed\n");fflush(logfile);   }
               exit(0);
        }

	/* Set socket to non-blocking with our setnonblocking routine */
	//setnonblocking(sock);
if (debug_mode==1){  fprintf(logfile,"Reading CLI args!\n"); fflush(logfile);}
	/* Get the address information, and bind it to the socket */
 	ascport = argv[1]; /* Read what the user gave us */
        port = strtol(ascport,&errpos,0);
        if ( (errpos[0] != 0) || (port < 1) || (port > 65535) ){
        if (debug_mode==1){	fprintf(logfile,"Invalid Port Number\n");fflush(logfile);   }
		return(0);
	}

if (debug_mode==1){  fprintf(logfile,"Binding to service\n"); fflush(logfile);}
	memset((char *) &server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(port);
	if (bind(sock, (struct sockaddr *) &server_address,
	  sizeof(server_address)) != 0 ) {
	if (debug_mode==1){	fprintf(logfile,"bind\n");  fflush(logfile);  }
		close(sock);
		exit(EXIT_FAILURE);
	}

	/* Set up queue for incoming connections. */
	if (listen(sock,499)!=0){
	    if (debug_mode==1){    fprintf(logfile,"listen\n"); fflush(logfile);  }
		close(sock);
		exit(EXIT_FAILURE);
                };

	/* Since we start with only one socket, the listening socket,
	   it is the highest socket so far. */
	highsock = sock;
if (debug_mode==1){fprintf(logfile,"Listening on Socket: %d\n",highsock); fflush(logfile);}


       cursor = vsc_createcursor();
       thesaurus = vsc_createcursor();

       for (i=0;i<TOTALSOCKS;i++) {
             connectlist[i].sock=0;
             connectlist[i].bufferpos = 0;
             connectlist[i].output_buffer = vs_prepare_queue();
       }


      signal(SIGPIPE, SIG_IGN);

if (debug_mode==1){fprintf(logfile,"Server Started\n"); fflush(logfile);}

fflush(logfile);

//LOAD OUR CACHE
if (debug_mode==1){fprintf(logfile,"Reloading Cache...\n"); fflush(logfile);}

      __VSDB_CACHE_PATH = (char *)malloc(1024);
      strcat(__VSDB_CACHE_PATH,cachedir);
      strcat(__VSDB_CACHE_PATH,"vsdb.dat");

      __VSDB_DYNAMIC_CACHE_PATH = (char *)malloc(1024);
      strcat(__VSDB_DYNAMIC_CACHE_PATH,cachedir);
      strcat(__VSDB_DYNAMIC_CACHE_PATH,"vsdb.do");

      __THESAURUS_CACHE_PATH = (char *)malloc(1024);
      strcat(__THESAURUS_CACHE_PATH,cachedir);
      strcat(__THESAURUS_CACHE_PATH,"thesaurus.dat");

if (debug_mode==1){fprintf(logfile,"Loading Cache from %s...\r\n",cachedir); fflush(logfile);}

      vp_load_cursor( cursor,(char *) __VSDB_CACHE_PATH);
      vp_load_cursor( thesaurus,(char *)__THESAURUS_CACHE_PATH);

if (debug_mode==1){fprintf(logfile,"Loading Compiled Cache from %s\r\n",cachedir); fflush(logfile);}

      vsdl_load ( cursor, (char *) __VSDB_DYNAMIC_CACHE_PATH )

if (debug_mode==1){fprintf(logfile,"Loaded %d Vectors, creating main cursor index\r\n",(int)cursor->count); fflush(logfile);}




if (debug_mode==1){fprintf(logfile,"Done. Ready.\r\n" ); fflush(logfile);}


//TODO: Throttle tv_usec - if we just did something, then only pause 10 or 50 ms, other wise bring up to 50 or 75
//      Then if nothing, up to 100, finally 150 or 250.. this should cut down on cpu usage, but make things snappy
//      and better adapt to loads.

	while (1) { /* Main server loop - forever */

		build_select_list();
		timeout.tv_sec = 0;
		timeout.tv_usec = 75;
		readsocks = select(highsock+1, &socks, (fd_set *) 0,
		  (fd_set *) 0, &timeout);
		if (readsocks < 0) {
		if (debug_mode==1){	fprintf(logfile,"select");   }
			exit(EXIT_FAILURE);
		}

  	        if (readsocks > 0) {
                     read_socks();
                     }


for (i=0;i<TOTALSOCKS;i++){
  connection = &connectlist[i];
  if ( vs_queue_size(connection->output_buffer)>0 ){
      node = vs_queue_pop(connection->output_buffer);
      //signal(SIGPIPE, SIG_IGN);
      if((writeable=send(connection->sock,node->buffer,node->size,0  ))>0){   // SO_NOSIGPIPE  or MSG_NOSIGNAL
                vs_destroy_node(&node);
            } else {

           if ((errno==EWOULDBLOCK) || (errno==EAGAIN)){
                 vs_queue_putback(connection->output_buffer,node);
            } else {vs_destroy_node(&node);}
           }
       }
     // signal(SIGPIPE, SIG_DFL);
     if (connection->closewhencomplete==1){
       if ( vs_queue_size(connection->output_buffer)==0 ){
           memset(connection->buffer, 0, sizeof(connection->buffer));
           close(connection->sock);
           connection->bufferpos=0;
           connection->sock=0;
           connection->session.mode = RAW_VECTOR_MODE;
           connection->session.thesaurus = 0.45;
           connection->closewhencomplete=0;
        }
       }
     }


} /*while active*/
  exit(EXIT_SUCCESS);
} /*main*/
