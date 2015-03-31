#
#   Makefile for the development project of R-CCS interpreter.
#  R-CCS interpreter have reflection, mobile agent development
#  language.
#                         Copyright (C) 1998 Naoyuki Nagatou
#

LDLIBS =
CFLAGS = -I../gdb_interface -I../../include -I/usr/include/glib-1.2 -I/usr/lib/glib/include -I. -g -Wall

SRCS=linked-list.c ltl-scanner.c ltl-parser.c ltl-token.c buchi-node.c ltl-converter.c buchi-converter.c normalizer.c ltl.c
OBJS=${SRCS:.c=.o}
HEADERS=${SRCS:.c=.h}

all: libltl.a test

${OBJS}: ${SRCS} ${HEADERS}

.c.o:
	${CC} -c $< ${CFLAGS}

libltl.a : ${OBJS}
	ar crv $@ $?; \
	ranlib $@;

test: linked-list-test ltl-test
	./linked-list-test
	#./ltl-test "[] <> ! (ax = 3)"
	#./ltl-test "[]!(ax = 3)\/(bx=4)" #NG?
	#./ltl-test "<>!(ax = 3)\/(bx=4)"
	./ltl-test "<>(ax = 15)"
	#./ltl-test "((<> (name_on_label_Tube="Bob")) \/ ([] <> ! (name_on_label_Tube=says_name_Receipt)))"
	dot -Tpng -o ltl-tree.png ltl-tree.dot
	dot -Tpng -o ltl-tree-norm.png ltl-tree-norm.dot
	dot -Tpng -o buchi-graph.png buchi-graph.dot

linked-list-test: linked-list-test.c linked-list.*
	gcc -o linked-list-test ${LDLIBS} ${CFLAGS} -DLINKED_LIST_TEST linked-list-test.c linked-list.c

ltl-test: ltl-test.c linked-list.* ltl-scanner.* ltl-parser.* ltl-token.* normalizer.* buchi-node.* ltl-converter.* buchi-converter.* ltl2dot.c buchi2dot.c
	gcc -o ltl-test ${LDLIBS} ${CFLAGS} -DLTL_TEST -DLTL_SCANNER_DEBUG -DLTL_PARSER_DEBUG -DLTL_CONVERTER_DEBUG ltl-test.c linked-list.c ltl-scanner.c ltl-parser.c ltl-token.c buchi-node.c ltl-converter.c normalizer.c ltl2dot.c buchi2dot.c buchi-converter.c ../driver/fhand.c

clean:
	rm -f ./*.o ./*.a ./*.dot ./*.png ./*~ ./linked-list-test ./ltl-test
