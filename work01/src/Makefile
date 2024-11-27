OBJS=main.o scan.o id-list.o

all: tc

tc: ${OBJS}
	${CC}  -o $@ $^

clean:
	${RM} *.o tc *~
