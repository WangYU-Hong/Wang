include ../Make.defines

LIBS += -lncursesw
CFLAGS = -I../lib -g -D_REENTRANT -Wall
PROGS = client test_common

client:	client.o screen.o common.o
		${CC} ${CFLAGS} -o $@ $@.o screen.o common.o ${LIBS}

test_common: test_common.o common.o
		${CC} ${CFLAGS} -o $@ $@.o common.o ${LIBS}

test_screen: test_screen.o screen.o common.o
		${CC} ${CFLAGS} -o $@ $@.o screen.o common.o ${LIBS}

clean:
	rm -f ${CLEANFILES} ${PROGS}