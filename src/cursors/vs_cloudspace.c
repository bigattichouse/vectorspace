vs_cloudspace *create_cloudspace();
void destroy_cloudspace(vs_cloudspace **cloudspace);

/*need to add GUID to cursors when they are created*/
int insert_cursor(vs_cloudspace *cloudspace, vs_cursor *cursor);
void remove_cursor(vs_cloudspace *cloudspace, vs_cursor *cursor);

//this will provide our base for a more generic tool that can have more than one
//cursor running in the VSDB

