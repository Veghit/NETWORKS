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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_USERS 15
#define MAX_FILES_PER_USER 15
#define MAX_FILE_SIZE 512
#define MAX_USERNAME_LENGTH 25
#define MAX_PASSWORD_LENGTH 25

int parseUsersFile(char * users_file, char * users[]) {

	int n = 0;
	char const* const fileName = users_file;
	FILE *file = fopen(fileName, "r");
	char line[MAX_USERNAME_LENGTH + MAX_PASSWORD_LENGTH + 1];
	while (fgets(line, sizeof(line), file)) {
		printf("%s", line);
		strcpy(users[n], line);
		n += 1;
	}
	fclose(file);
	return n;
}

char * getUserName(char * users[], int j) {
	char * username = calloc(sizeof(char), 1 + MAX_USERNAME_LENGTH);
	int i = 0;
	while (i < MAX_USERNAME_LENGTH) {
		if (users[j][i] == ' ')
			break;
		username[i] = users[j][i];
		i += 1;
	}
	username[i + 1] = 0;
	return username;
}

void sendHello() {
	printf("Welcome! Please log in.");
}

int isUserConnected() {
	return 0;
}

int waitForUser()
{
	// connect TCP
	int newsockfd, clilen, n;
	char buffer[256];
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
		perror("ERROR on accept");
		exit(1);
	}

	struct sockaddr_in my_addr, cli_addr;

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(7000);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0) {
		perror("ERROR on binding");
		exit(1);
	}

	listen(sockfd, 5);

	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

	if (newsockfd < 0) {
		perror("ERROR on accept");
		exit(1);
	}

	bzero(buffer, 256);
	n = read(newsockfd, buffer, 255);

	if (n < 0) {
		perror("ERROR reading from socket");
		exit(1);

	}

	printf("Here is the message: %s\n", buffer);

	n = write(newsockfd, "I got your message", 18);

	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);

	}
	return newsockfd;
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
	char * users[MAX_USERS];

	for (int i = 0; i < MAX_USERS; i++) {
		users[i] = calloc(sizeof(char),
		MAX_PASSWORD_LENGTH + MAX_USERNAME_LENGTH + 1);
	}

	int port = 1337;
	if (argc == 4) {
		port = atoi(argv[3]);
		// get port from cmd arg
	}

	printf("ALL GOOD\n\t users_file:%s\n\t dir_path:%s\n\t port:%d\n",
			users_file, dir_path, port);

	// read users file
	int usersNum = parseUsersFile(users_file, users);
	printf("%d users were found.\n", usersNum);
	// open a folder for each user

	int j;
	//char * username = getUserName(users, 0);
	//printf("%s",username);

	for (j = 0; j < usersNum; j++) {
		char * folderName = calloc(sizeof(char), 256);
		strcat(folderName, "SERVER/DATA/");
		strcat(folderName, getUserName(users, j));
		printf("%s\n", folderName);
		mkdir(folderName, 777);
	}






	while (1) {
		int userSocket = waitForUser();

		if (isUserConnected()) {
			sendHello();

		}
		sleep(2);
		printf("Waiting for a user to connect.\n");

	}
}
