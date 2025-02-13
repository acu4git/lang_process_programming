OBJS=main.o scan.o parser.o cross_referencer.o compiler.o
CFLAGS = -g -Wall

all: mpplc

mpplc: ${OBJS}
	${CC}  -o $@ $^

coverage:
	- ${CC} -coverage -c *.c
	- ${CC} -coverage -o mpplc *.o

clean:
	${RM} *.o *.gcov *.gcda *.gcno mpplc *~
