file_server : server.c aux.h
        gcc -Wall -g server.c
file_user: user.c aux.h
	gcc -Wall -g user.c
clean :
        rm edit file_server file_user
