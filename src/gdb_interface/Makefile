LDLIBS = -L/usr/lib -L/opt/gnome/lib -lglib -L../../lib -ldriver -lerror -lparse -lprint -llist -L../ltl -lltl
CFLAGS = -I. -I../../include -I/usr/include/glib-2.0 -I/usr/lib/glib/include \
		 -D MC_LOG ${CFLAGS}

SRCS=debugger.c errors.c log_printf.c mi_parser.c utils.c mc_interface.c gdb_control.c
OBJS=${SRCS:.c=.o}
HEADERS=${SRCS:.c=.h}

all : libgdb_interface.a test

${OBJS}: ${SRCS} ${HEADERS}

.c.o:
	${CC} -c $< ${CFLAGS}

clean : 
		rm -f *.a *.o *.exe *~

libgdb_interface.a : ${OBJS}
					ar rcs $@ $^ 
	
#gcc  -o test test.c  -L. -lgdb_interface -L/usr/lib -lglib -L/opt/gnome/lib -lglib -I. -I../../include -I/usr/include/glib-1.2 -I/usr/lib/glib/include -g

test : libgdb_interface.a test.c
#	gcc  -o test test.c  -L../driver -ldriver -L. -lgdb_interface -L/usr/lib -lglib -L/opt/gnome/lib -lglib -I. -I../../include -I/usr/include/glib-1.2 -I/usr/lib/glib/include -g

count:
	sloccount ${SRCS} ${HEADERS}
