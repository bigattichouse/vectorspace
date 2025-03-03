







int __vpi_create_persist_interface(){
 //creates persist file

}

int __vpi_append_vector(){
 //slap on end of file

}


int __vpi_delete_vector(){
 //just zeros out

}

int __vpi_replace_vector(){
 //if same size, just overwrites, otherwise deletes and appends.
}

vector *__vpi_load_vector(){
  
}


vpi_cursorindex is almost identical to cursor index, just stores the file position instead of the vector.