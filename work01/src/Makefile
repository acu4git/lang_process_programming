OBJS=main.o scan.o id-list.o

all: tc

tc: ${OBJS}
	${CC}  -o $@ $^

coverage:
	- ${CC} -coverage -c *.c
	- ${CC} -coverage -o tc *.o

clean:
	${RM} *.o *.gcov *.gcda *.gcno tc *~
