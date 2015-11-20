


typedef struct {
        char *hostname;
	int	sd;
	struct sockaddr_in sin;
	struct sockaddr_in pin;
	struct hostent *hp;
} vsdb_socket;




vsdb_socket *vsdb_socket_prepare (char *host, int port);
int vsdb_socket_open (vsdb_socket *socket);
int vsdb_socket_close (vsdb_socket *socket);
int vsdb_socket_connected (vsdb_socket *socket);
int vsdb_socket_replace (vsdb_socket *socket,vector *v);
int vsdb_socket_thesaurus (vsdb_socket *socket,vector *add);
int vsdb_socket_delete (vsdb_socket *socket,t_uuid vectorid);
vector *vsdb_socket_view (vsdb_socket *socket,t_uuid vectorid);
vs_cursor *vsdb_socket_query (vsdb_socket *socket,vector *query,float threshold);

