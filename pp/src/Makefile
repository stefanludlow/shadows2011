# Makefile for RPI Engine, $Revision: 2.15 $
# Copyright (C) 2006, Shadows of Isildur

CC=g++

RESETDATE = `touch comm.cpp`
#SVN_VERS = -D'SVN_REV="$(shell svnversion -n . && touch comm.cpp)"'
CFLAGS = -DLINUX -ggdb -w -m32
#-fstack-protector-all
CXXFLAGS = $(CFLAGS)
LFLAGS = -lm -lmysqlclient -lcrypt -lnsl -m32

INSTALL_DIR = ..

INCLUDE = -I/usr/local/mysql/include/ -I/usr/include/mysql/ -I/usr/include/openssl/ -I/usr/local/mysql/ -I/usr/local/include/mysql/ -I/usr/lib/mysql

LIBDIRS = -L/usr/lib32/ -L/usr/lib32/mysql/

OBJS = \
account.o act.comm.o act.informative.o act.movement.o act.offensive.o \
act.other.o arena.o auctions.o character.o clan.o comm.o \
commands.o commerce.o constants.o control.o crafts.o \
create_mobile.o creation.o db.o destruction.o \
dwellings.o edit.o fight.o group.o guest.o \
guides.o handler.o hash.o item_card.o item_tossable.o \
limits.o magic.o mobact.o mobprogs.o mysql.o \
nanny.o net_link.o object_damage.o objects.o olc.o \
perception.o psionics.o roles.o roomprogs.o save.o \
server.o somatics.o \
staff.o stringstack.o structs.o transformation.o utility.o weather.o wounds.o

HEADERS = \
account.h clan.h character.h constants.h decl.h dynamicvariable.h group.h net_link.h \
object_damage.h protos.h room.h server.h stringstack.h structs.h trigram.h utility.h utils.h weather.h

# default target
../bin/server: $(OBJS)
	if test -f ../bin || install -v -d ../bin; then \
$(CC) $(INCLUDE) $(LIBDIRS) -o $@ $(LFLAGS) $(CFLAGS) $(OBJS) $(PFLAGS); fi

%.o: %.cpp $(HEADERS)
	$(CC) -c $(CFLAGS) $< $(PFLAGS)

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $< $(PFLAGS)

tags:
	ctags *.c *.h

clean:
	rm -f *.o $(INSTALL_DIR)/bin/server

install:
	cp -f ~shadows/shadows/pp/bin/server ~shadows/shadows/pp/bin/server.last
	cp -f ~shadows/shadows/bp/bin/server ~shadows/shadows/bp/bin/server.last
	cp -f ~shadows/shadows/rpi_engine/pp/bin/server ~shadows/shadows/pp/bin/server
	cp -f ~shadows/shadows/rpi_engine/pp/bin/server ~shadows/shadows/bp/bin/server

#does not create a .last. Use when updating a second time without a reboot/crash
#occuring first. Goal is to keep the .last edition the same as the currently 
#running, for gdb'ing its core.

updatedinstall:
	cp -f ~shadows/shadows/rpi_engine/pp/bin/server ~shadows/shadows/pp/bin/server
	cp -f ~shadows/shadows/rpi_engine/pp/bin/server ~shadows/shadows/bp/bin/server
