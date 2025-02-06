OBJS=main.o scan.o parser.o cross_referencer.o
CFLAGS = -g -Wall

all: cr

cr: ${OBJS}
	${CC}  -o $@ $^

coverage:
	- ${CC} -coverage -c *.c
	- ${CC} -coverage -o cr *.o

clean:
	${RM} *.o *.gcov *.gcda *.gcno cr *~
