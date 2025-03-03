/*
VS Persist is run after the vsdb service starts, it opens the file
specified for this service (vsdb-9998.dat) and loads the vsdbd with it.
periodically, vspersist will connect and download the current dataset,
overwriting the file.
*/


