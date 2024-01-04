include ../Make.defines

LIBS += -lncursesw

client:	client.o
		${CC} ${CFLAGS} -o $@ client.o ${LIBS}

test_common: test_common.o common.o
		${CC} ${CFLAGS} -o $@ $@.o common.o ${LIBS}