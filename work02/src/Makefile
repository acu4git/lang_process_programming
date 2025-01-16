OBJS=main.o scan.o pretty_printer.o parser.o

all: pp

pp: ${OBJS}
	${CC}  -o $@ $^

coverage:
	- ${CC} -coverage -c *.c
	- ${CC} -coverage -o pp *.o

clean:
	${RM} *.o *.gcov *.gcda *.gcno pp *~
