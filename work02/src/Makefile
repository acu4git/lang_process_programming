OBJS=main.o scan.o parser.o
CFLAGS = -g -Wall

all: pp

pp: ${OBJS}
	${CC}  -o $@ $^

coverage:
	- ${CC} -coverage -c *.c
	- ${CC} -coverage -o pp *.o

clean:
	${RM} *.o *.gcov *.gcda *.gcno pp *~
