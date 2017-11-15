#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



int main(int argc, char *argv[]) {
	if ((argc != 3) && ( argc != 4)) {
		printf("should receive 3 or 4 cmd args. Received %d args", argc);
		return 1;
	}
	// get other vars from command
	char * users_file = argv[1];
	char * dir_path = argv[2];
	int port = 1337;
	if (argc==4)
	{
		port = atoi(argv[3]);
	// get port from cmd arg
	}

	printf("ALL GOOD./n users_file:%s , dir_path:%s , port:%d",users_file,dir_path,port);



}
