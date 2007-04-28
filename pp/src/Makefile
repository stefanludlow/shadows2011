# Makefile for RPI Engine, $Revision: 2.15 $
# Copyright (C) 2006, Shadows of Isildur

CC=g++

CFLAGS = -ggdb -DLINUX -Wall
LFLAGS = -lm -lmysqlclient -lcrypt -lnsl

INSTALL_DIR = ..

INCLUDE = -I/usr/local/mysql/include/ -I/usr/include/mysql/ -I/usr/include/openssl/ -I/usr/local/mysql/ -I/usr/local/include/mysql/

LIBDIRS = -L/usr/lib64/mysql

OBJS = \
account.o act.comm.o act.informative.o act.movement.o \
act.offensive.o act.other.o arena.o auctions.o clan.o comm.o \
commands.o commerce.o constants.o control.o crafts.o \
create_mobile.o creation.o db.o destruction.o \
dwellings.o edit.o employment.o fight.o group.o guest.o \
guides.o handler.o hash.o item_card.o item_tossable.o \
limits.o magic.o mobact.o mobprogs.o mysql.o \
nanny.o net_link.o object_damage.o objects.o olc.o \
perception.o psionics.o roles.o roomprogs.o save.o somatics.o \
staff.o transformation.o utility.o weather.o wounds.o

HEADERS = \
account.h clan.h constants.h decl.h group.h net_link.h \
object_damage.h protos.h server.h structs.h trigram.h utils.h

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
