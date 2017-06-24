#
#   Makefile for the development project of R-CCS interpreter.
#  R-CCS interpreter with reflection is mobile agent development
#  language.
#  
#                         Copyright (C) 1998 Naoyuki Nagatou
#
#  Compiler: gcc 2.7.2 ,   C library: libc-5.3.12
#  Make: GNU make 3.79.1
CC=gcc
CHECK=
MAKE=make
TEX=pdflatex

INSTALL_PATH=/usr/local/bin/

#RCCS_HOME=${HOME}/rccs
#RCCS_HOME=/home/sushil/D/programming/nagatou_san/rccs
RCCS_HOME=$(PWD)/..
SRCPATH=${RCCS_HOME}/src
INCLUDE=${RCCS_HOME}/include
LIBPATH=${RCCS_HOME}/lib

TARGET=rccs
VPATH=${INCLUDE}:${SRC_PATH}:${RCCS_HOME}/rrl

SRCS=ccs.c
OBJS=${SRCS:.c=.o}
HEADERS=${SRCS:.c=.h}
COMPONENTS=list.a error.a parse.a print.a driver.a gdb_interface.a ltl.a
LIBS=-ldriver -lerror -lparse -lprint -llist \
      -L${RCCS_HOME}/src/ltl -lltl\
      -L${RCCS_HOME}/src/gdb_interface -lgdb_interface -L/usr/lib/ -lglib-2.0
BINDIR=

OPT=-I${INCLUDE} -I${RCCS_HOME}/src/gdb_interface\
                 -I${RCCS_HOME}/src/ltl\
                 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include
#                 -I/usr/include/glib-1.2 -I/usr/lib/glib/include
DEBUG_FLAG=-g -O0 -Wall # empty now, but assign -g -p for debugging
MC_FLAGS = -D MC_LINUX -D DEBUG_MC_S #Cannot remove for Linux
CFLAGS=${OPT} ${DEBUG_FLAG} ${MC_FLAGS}

LDFLAGS=-L${LIBPATH}

rebuild:
	${MAKE} rccs "FRC=frc"
frc:

clean:
	rm -f *.o;\
	rm -f ${TARGET};\
	rm -f ${LIBPATH}/*.a;\
	cd ${SRCPATH}/parse; ${MAKE} clean;\
	cd ${SRCPATH}/list; ${MAKE} clean;\
	cd ${SRCPATH}/print; ${MAKE} clean;\
	cd ${SRCPATH}/driver; ${MAKE} clean;\
	cd ${SRCPATH}/error; ${MAKE} clean;\
	cd ${SRCPATH}/gdb_interface; ${MAKE} clean;\
	cd ${SRCPATH}/ltl; ${MAKE} clean;\

doc:
	cd Docs;\
	${TEX} manual-en.tex
te_rccs : ${FRC} ${SRCS} ${COMPONENTS}
	${CC} -o $@ ${SRCS}\
		${CFLAGS}\
		${LDFLAGS}\
	 	${LIBS}
${TARGET} : ${FRC} ${SRCS} ${COMPONENTS}
	${CC} -o $@ ${SRCS}\
		${CFLAGS}\
		${LDFLAGS}\
	 	${LIBS}
lib : ${COMPONENTS} libem.a
${COMPONENTS}:
	cd ${SRCPATH}/$* ; ${MAKE} lib${@} \
			"CC=${CC}" \
			"CFLAGS=${CFLAGS}" \
			"INCLUDE=${INCLUDE}" \
			"VPATH=${VPATH}" \
			"LIBPATH=${LIBPATH}"	
libem.a :
	cd ${RCCS_HOME}/rrl ; ${MAKE} ${@} \
			"CC=${CC}" \
			"CFLAGS=${CFLAGS}" \
			"INCLUDE=${INCLUDE}" \
			"RCCS_HOME=${RCCS_HOME}" \
			"VPATH=${VPATH}" \
			"LIBPATH=${LIBPATH}"	
test : echos.c
	cd cont ; \
	${CC} -o echos echos.c -DDEBUG_EHAND ${CFLAGS} ${LIBS}

install: rccs
	cp ${SRC_PATH}/rccs ${INSTALL_PATH}
