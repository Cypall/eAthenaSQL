all: login-server

COMMON_OBJ = ../common/core.o ../common/socket.o ../common/timer.o ../common/db.o
COMMON_H = ../common/core.h ../common/socket.h ../common/timer.h ../common/mmo.h ../common/version.h ../common/db.h
LIB_S = -L/usr/local/lib/mysql -lmysqlclient -lz
login-server: login.o md5calc.o strlib.o $(COMMON_OBJ)
	$(CC) -o ../$@ $> $(LIB_S)
login.o: login.c login.h md5calc.h strlib.h $(COMMON_H)
md5calc.o: md5calc.c md5calc.h
strlib.o: strlib.c strlib.h
clean:
	rm -f *.o ../login-server
