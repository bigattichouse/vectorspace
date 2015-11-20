CC=/opt/lsb/bin/lsbcc
#CC=gcc

all: vsdbd testuuid testmerge testnoise testcluster testthesaurus testpersist

vsdbd:	
	${CC} -Wall -O3 uuid.c vs_dimensionindex.c vs_core.c vs_list.c \
 vs_cursor.c vs_cache.c vs_query.c sockhelp.c vs_queue.c \
 vs_thesaurus.c vs_interpreter.c vs_rowset.c vs_cluster.c vsdbd.c -ldl -o vsdbd -lm

testuuid:
	${CC} uuid.c testuuid.c -o testuuid -Wall -ldl

testmerge:
	${CC} -g3 -Wall -O3 uuid.c vs_dimensionindex.c vs_core.c vs_list.c \
 vs_cursor.c vs_cache.c vs_query.c sockhelp.c vs_queue.c \
 vs_thesaurus.c vs_interpreter.c vs_rowset.c vs_cluster.c test_merge.c  -ldl -o testmerge -lm -pg

testnoise:
	${CC} -g3 -Wall -O0 uuid.c vs_dimensionindex.c vs_core.c vs_list.c \
 vs_cursor.c vs_cache.c vs_query.c sockhelp.c vs_queue.c \
 vs_thesaurus.c  vs_interpreter.c vs_rowset.c vs_cluster.c test_denoise.c  -ldl -o testnoise -lm -pg

testcluster:
	${CC} -g3 -Wall -O0 uuid.c vs_dimensionindex.c vs_core.c vs_list.c \
 vs_cursor.c vs_cache.c vs_query.c sockhelp.c vs_queue.c \
 vs_thesaurus.c vs_interpreter.c vs_rowset.c vs_cluster.c test_cluster.c -ldl -o testcluster -lm -pg

testthesaurus:
	${CC} -g3 -Wall -O0 uuid.c vs_dimensionindex.c vs_core.c vs_list.c \
 vs_cursor.c vs_cache.c vs_query.c sockhelp.c vs_queue.c \
 vs_thesaurus.c vs_interpreter.c vs_rowset.c vs_cluster.c test_thesaurus.c -ldl -o testthesaurus -lm -pg

testpersist:
	${CC} -g3 -Wall -O0 uuid.c vs_dimensionindex.c vs_core.c vs_list.c \
 vs_cursor.c vs_cache.c vs_query.c sockhelp.c vs_queue.c \
 vs_thesaurus.c vs_interpreter.c vs_rowset.c vs_cluster.c vs_persist.c test_persist.c -ldl -o testpersist -lm -pg

testcore2:
	${CC} -g3 -Wall -O3 uuid.c vs_dimensionindex.c vs_core.c vs_list.c \
 vs_cursor.c vs_cache.c vs_query.c sockhelp.c vs_queue.c \
 vs_thesaurus.c vs_interpreter.c vs_rowset.c vs_cluster.c vs_persist.c test_core2.c -ldl -o testcore2 -lm -pg

install: vsdbd
	mkdir -p /var/log/vsdbd
	mkdir -p /opt/lsb-bigattichouse
	mkdir -p /opt/lsb-bigattichouse/bin/
	cp vsdbd /opt/lsb-bigattichouse/bin/
