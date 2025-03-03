

//wraps vs_disc_interface 
void vsd_replacevector( __vdi_interface *intf,vector *v);
void vsd_deletevector( __vdi_interface *intf,t_uuid vectorid) ;
int vsd_query( __vdi_interface *intf,t_uuid sessionid,vector *query,float threshold,vs_queue *queue) ;

