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

#define MAX_USERS 15
#define MAX_FILES_PER_USER 15
#define MAX_FILE_SIZE 512
#define MAX_USERNAME_LENGTH 25
#define MAX_PASSWORD_LENGTH 25

int parseUsersFile(char * users_file,char * users[]) {

	int n = 0;
	char const* const fileName = users_file;
	FILE *file = fopen(fileName, "r");
	char line[MAX_USERNAME_LENGTH + MAX_PASSWORD_LENGTH + 1];
	while (fgets(line, sizeof(line), file)) {
		printf("%s", line);
		strcpy(users[n],line);
		n += 1;
	}
	fclose(file);
	return n;
}

char * getUserName(char * users[], int j){
	char * username = calloc(sizeof(char), 1+MAX_USERNAME_LENGTH);
	int i =0;
	while(i<MAX_USERNAME_LENGTH){
		if(users[j][i]==' ')
			break;
		username[i]=users[j][i];
		i+=1;
	}
	username[i+1] = 0;
	return username;
}

void sendHello(){
	printf("Welcome! Please log in.");
}

int isUserConnected(){
	return 0;
}

int main(int argc, char *argv[]) {

	//char cwd[1024];
	//printf("%s",getcwd(cwd,sizeof(cwd)));


	if ((argc != 3) && (argc != 4)) {
		printf("should receive 3 or 4 cmd args. Received %d args", argc);
		return 1;
	}
	// get other vars from command
	char * users_file = argv[1];
	char * dir_path = argv[2];
	char * users [MAX_USERS];

	for (int i=0;i<MAX_USERS;i++){
		users[i] = calloc(sizeof(char),MAX_PASSWORD_LENGTH+MAX_USERNAME_LENGTH+1);
	}

	int port = 1337;
	if (argc == 4) {
		port = atoi(argv[3]);
		// get port from cmd arg
	}

	printf("ALL GOOD\n\t users_file:%s\n\t dir_path:%s\n\t port:%d\n", users_file,
			dir_path, port);

	// read users file
	int usersNum = parseUsersFile(users_file,users);
	printf("%d users were found.\n", usersNum);
	// open a folder for each user

	int j;
	char * username = getUserName(users,0);
	//printf("%s",username);


	for (j=0;j<usersNum;j++)
	{
		char * folderName  = calloc(sizeof(char),256);
		strcat(folderName, "SERVER/DATA/");
		strcat(folderName,getUserName(users,j));
		printf("%s\n",folderName);
		mkdir(folderName,777);
	}

	// connect TCP
	int sock = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in my_addr,client_adrr;

	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons( 80 );
	myaddr.sin_addr = htonl(0x8443FC64);

	bind(sock, &myaddr, sizeof(myaddr));

	listen(sock, 5);

	sin_size = sizeof(struct sockaddr_in);
	new_sock = accept(sock, (struct sockaddr*) &client_adrr, &sin_size);



	while (1) {

	if (isUserConnected())
	{
		sendHello();

	}
	sleep(2);
	printf("Waiting for a user to connect.\n");



	}
}
