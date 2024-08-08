#!/bin/sh
	rsqlt=`rm -rf *.o`
	gcc -c char-converter.c -I/usr/local/include/mysql/
	gcc -c strlib.c
	gcc -o char-converter char-converter.o strlib.o ../common/core.o ../common/socket.o ../common/timer.o ../common/db.o -L/usr/local/lib/mysql -lmysqlclient -lz
