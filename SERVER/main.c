#include "main.h"

#define MAX_USERS 15
#define MAX_FILES_PER_USER 15
#define MAX_FILE_SIZE 512
#define MAX_USERNAME_LENGTH 25
#define MAX_PASSWORD_LENGTH 25
#define BUFFER_SIZE 1000

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

int sendHello(int socket, char msg[]) {
	char * str = "Welcome! Please log in.";
	msgWrite(socket, msg, 0, str);
	return strlen(str) + 5;
}

int sendStatus(int socket, char msg[], int userID) {
	int filesCount = 0;
	char * str = "Hi Bob, you have 6 files stored.";
	msgWrite(socket, msg, 2, str);
	//printf("Hi %s, you have %d files stored.", getUserName(userID), filesCount);
	return strlen(str) + 5;
}

void sendFilesList() {
	printf("Welcome! Please log in.");
}

void deleteFile() {

}
void addFile() {
}
void getFile() {
}
void userQuit() {
}

int checkLogin(char msg[]) {
	return 2;
}

int isUserConnected() {
	return 0;
}

int waitForUser() {
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

	return newsockfd;
}

int getValueLength(char msg[]) {
	return msg[3] * 256 + msg[4];
}

// This assumes buffer is at least x bytes long,
// and that the socket is blocking.
int getNextMsg(int socket, char msg[]) {
	int minLen = 5;
	int bytesRead = 0;
	int result;
	while (bytesRead < minLen) {
		result = read(socket, msg + bytesRead, minLen - bytesRead);
		if (result < 1) {
			return 1;
		}

		bytesRead += result;
	}
	int valueLength = getValueLength(msg);

	while (bytesRead < minLen + valueLength) {
		result = read(socket, msg + bytesRead,
				minLen + valueLength - bytesRead);
		if (result < 1) {
			return 1;
		}

		bytesRead += result;
	}

	return 0;
}

int sendFail(int socket, char msg[], char * str) {
	return msgWrite(socket, msg, 6, str);
}

void sendSuccess(int userSocket, char msg[]) {
	msgWrite(userSocket, msg, 6, "");
}

int msgWrite(int userSocket, char msg[], char type, char * str) {
	msg[0] = 0x22;
	msg[1] = 0x1E;
	msg[2] = type;
	int i = 0;
	while (str[i]) {
		msg[5 + i] = str[i];
		i++;
	}
	msg[3] = i / 256;
	msg[4] = i % 256;

	return writeMsgToSocket(userSocket, msg, i+5);
}

int writeMsgToSocket(int socket, char msg[], int length) {
	printf("%s", msg);
	int bytesWritten = 0;
	int result;
	while (bytesWritten < length) {
		result = write(socket, msg + bytesWritten, length - bytesWritten);
		if (result < 1) {
			return 1;
		}

		bytesWritten += result;
	}
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
	char * users[MAX_USERS];

	for (int i = 0; i < MAX_USERS; i++) {
		users[i] = calloc(sizeof(char),
		MAX_PASSWORD_LENGTH + MAX_USERNAME_LENGTH + 1);
	}

	int port = 1337;
	if (argc == 4) {
		port = atoi(argv[3]); // get port from cmd arg
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
		char msgType, userID, loggedIn;
		char msg[BUFFER_SIZE];
		loggedIn = 0;
		msgType = -1;
		userID = -1;
		sendHello(userSocket, msg);
		while (msgType != 10) {
			getNextMsg(userSocket, msg);
			msgType = msg[2];
			printf("The current message type is:%d", msgType);
			switch (msgType) {
			case 1: // login message
				userID = checkLogin(msg); // the first user gets the id 0. the 2nd user in the file gets id 1 and so on.
				if (userID != -1) {
					loggedIn = 1;
					sendSuccess(userSocket, msg);
				} else
					sendFail(userSocket, msg, "login failed.");
				break;
			case 3: // list_of_files request
				if (loggedIn)
					sendFilesList(userSocket, msg, userID);
				else
					sendFail(userSocket, msg, "User not logged in yet.");
				break;
			case 5: // delete file request
				if (loggedIn)
					deleteFile(msg, userID);
				else
					sendFail(userSocket, msg, "User not logged in yet.");
				break;
			case 7: // file_transfer (to server)
				if (loggedIn)
					addFile(msg, userID);
				else
					sendFail(userSocket, msg, "User not logged in yet.");
				break;
			case 8: // file_request (from server)
				sendFail(userSocket, msg, "User not logged in yet.");
				break;
			case 10: // quit
				break;
			default: // bad type
				if(1==sendFail(userSocket, msg, "Bad message type."))
				{
					perror("ERROR writing to socket");
					exit(1);
				}
				break;
			}

		}
		sleep(2);
		printf("Waiting for a user to connect.\n");

	}
}
