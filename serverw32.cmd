gcc -O3 -DWIN32 -mcpu=i686 -mwindows -Wall -O3 uuid.c vs_dimensionindex.c vs_core.c vs_list.c vs_cursor.c vs_cache.c vs_rowset.c vs_query.c sockhelp.c vs_queue.c  vs_thesaurus.c vs_persist.c vs_interpreter.c vs_cluster.c vsdbd.c -ldl -o w32_vsdbd -lm -lole32



