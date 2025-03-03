/*

wrapper client for C to handle stemming.

*/


vsdb_session();
vsdb_connect(session, server);
vsdb_replace(session, id, text);
vsdb_replace_raw(session,id,text);
vsdb_view(session,id);
vsdb_delete(session, id);
vsdb_query(session,id, text);
vsdb_id();
vsdb_readbuffer(session);