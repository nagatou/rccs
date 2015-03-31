#
#   Makefile for the development project of R-CCS interpreter.
#  R-CCS interpreter have reflection, mobile agent development
#  language.
#                         Copyright (C) 1998 Naoyuki Nagatou
#
SRCS=parse.c scan.c shand.c
OBJS=${SRCS:.c=.o}
HEADERS=${SRCS:.c=.h}

.c.o:
	${CC} -c $< ${CFLAGS} -I${INCLUDE}
clean:
	rm -f ./*.o;rm -f ./*.a

${OBJS}: ${SRCS} ${HEADERS} comm.h
libparse.a : ${OBJS}
	ar crv $@ $?; \
	ranlib $@; \
	cp $@ ${LIBPATH}
