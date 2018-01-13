all: server user

server:server.c header.h
	gcc -g -Wall -o file_server server.c

user:user.c header.h
	gcc -g -Wall -o file_user user.c

clean: 
	$(RM) file_server file_user
