project1: server_program\
			client_program

server_program: server_program.o misc.o
	cc server_program.o misc.o -o server_program

server_program.o: server_program.c misc.h
	cc -c server_program.c

client_program: client_program.o misc.o
	cc client_program.o misc.o -o client_program

client_program.o: client_program.c misc.h
	cc -c client_program.c
misc.o: misc.c misc.h
	cc -c misc.c
clean:
	rm server_program.o client_program.o misc.o server_program client_program