include ../Make.defines

# LIBS += -lncursesw
CFLAGS = -I../lib -g -D_REENTRANT -Wall
PROGS = client test_common test_screen testcli finalproject


test_common: test_common.o common.o
		${CC} ${CFLAGS} -o $@ $@.o common.o ${LIBS}

finalproject:	finalproject.o extend.o common.o
		${CC} ${CFLAGS} -o $@ finalproject.o extend.o common.o ${LIBS}

testcli:	testcli.o common.o
		${CC} ${CFLAGS} -o $@ testcli.o common.o ${LIBS}
syscontroller:	syscontroller.o common.o
		${CC} ${CFLAGS} -o $@ syscontroller.o common.o ${LIBS}

client:	client.o screen.o common.o
		${CC} ${CFLAGS} -o $@ $@.o screen.o common.o ${LIBS} -lncursesw

test_screen: test_screen.o screen.o common.o
		${CC} ${CFLAGS} -o $@ $@.o screen.o common.o ${LIBS} -lncursesw



clean:
	rm -f ${CLEANFILES} ${PROGS}
