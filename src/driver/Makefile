#
#   Makefile for the development project of R-CCS interpreter.
#  R-CCS interpreter has the capability for reflection and mobile agent.
#
#                         Copyright (C) 1998 Naoyuki Nagatou
#
SRCS=driver.c ehand.c fhand.c primitive.c channel.c osenv.c
OBJS=${SRCS:.c=.o}
HEADERS=${SRCS:.c=.h}

.c.o:
	${CC} -c $< ${CFLAGS} -I${INCLUDE}
clean:
	rm -f ./*.o;rm -f ./*.a

${OBJS}: ${SRCS} ${HEADERS} comm.h verifier.c
libdriver.a : ${OBJS}
	ar crv $@ $?; \
	ranlib $@; \
	cp $@ ${LIBPATH}
