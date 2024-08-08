# $Id: Makefile,v 1.2 2004/02/17 04:48:07 rovert Exp $

CC = gcc -pipe
PACKETDEF = -DPACKETVER=4 -DNEW_006b
#PACKETDEF = -DPACKETVER=3 -DNEW_006b
#PACKETDEF = -DPACKETVER=2 -DNEW_006b
#PACKETDEF = -DPACKETVER=1 -DNEW_006b

PLATFORM = $(shell uname)

ifeq ($(findstring CYGWIN,$(PLATFORM)), CYGWIN)
OS_TYPE = -DCYGWIN
else
OS_TYPE =
endif

CFLAGS = -g -O2 -Wall -I../common $(PACKETDEF) $(OS_TYPE)
MKDEF = CC="$(CC)" CFLAGS="$(CFLAGS)"
MYLIB = CC="$(CC)" CFLAGS="$(CFLAGS) -I/usr/local/include/mysql"

all clean: common/GNUmakefile login/GNUmakefile char/GNUmakefile
	cd common ; make $(MKDEF) $@ ; cd ..
	cd login ; make $(MYLIB) $@ ; cd ..
	cd char ; make $(MYLIB) $@ ; cd ..
	cd map && make $(MKDEF) $@ && cd ..
	
common/GNUmakefile: common/Makefile
	sed -e 's/$$>/$$^/' common/Makefile > common/GNUmakefile
login/GNUmakefile: login/Makefile
	sed -e 's/$$>/$$^/' login/Makefile > login/GNUmakefile
char/GNUmakefile: char/Makefile
	sed -e 's/$$>/$$^/' char/Makefile > char/GNUmakefile
map/GNUmakefile: map/Makefile
	sed -e 's/$$>/$$^/' map/Makefile > map/GNUmakefile
