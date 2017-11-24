#include "aux.h"

int parseUsersFile(char * users_file, char * users[]) {

	int n = 0;
	char const* const fileName = users_file;
	FILE *file = fopen(fileName, "r");
	char line[MAX_USERNAME_LENGTH + MAX_PASSWORD_LENGTH + 1];
	while (fgets(line, sizeof(line), file)) {
		//printf("%s", line);
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

int sendHello(int socket) {
	char * str = "Welcome! Please log in.";
	Message msg = createMessagefromString(helloMSG, str);
	sendMessage(socket, msg);
	return strlen(str) + 5;
}

int sendStatus(int socket, int userID) {
	//int filesCount = 0;
	char * str = "Hi Bob, you have 6 files stored.";
	Message msg = createMessagefromString(statusMSG, str);
	sendMessage(socket, msg);
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

int waitForUser(int port) {
	// connect TCP
	int newsockfd, clilen;
	//char buffer[256];
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
		perror("ERROR on accept");
		exit(1);
	}

	struct sockaddr_in my_addr, cli_addr;

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
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

	//bzero(buffer, 256);
	//n = read(newsockfd, buffer, 255);

	//if (n < 0) {
	//	perror("ERROR reading from socket");
	//	exit(1);

	//}

	//printf("Here is the message: %s\n", buffer);

	return newsockfd;
}

int getValueLength(char msg[]) {
	return msg[3] * 256 + msg[4];
}



int sendFail(int userSocket) {
	Message msg = createMessagefromString(failureMSG, "");
	return msgWrite(socket, msg);
}

int sendSuccess(int userSocket) {
	Message msg = createMessagefromString(successMSG, "");
	return msgWrite(userSocket, msg);
}

int main(int argc, char *argv[]) {

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

	//printf("ALL GOOD\n\t users_file:%s\n\t dir_path:%s\n\t port:%d\n",
	//		users_file, dir_path, port);

	// read users file
	int usersNum = parseUsersFile(users_file, users);
	//printf("%d users were found.\n", usersNum);
	// open a folder for each user

	int j;
	//char * username = getUserName(users, 0);
	//printf("%s",username);

	for (j = 0; j < usersNum; j++) {
		char * folderName = calloc(sizeof(char), 256);
		strcat(folderName, "SERVER/DATA/");
		strcat(folderName, getUserName(users, j));
		//printf("%s\n", folderName);
		mkdir(folderName, 777);
	}

	printf("\n***Waiting for a user to connect***\n");

	while (1) {
		int userSocket = waitForUser(port);

		char userID, loggedIn;
		message_type t;
		loggedIn = 0;
		t = -1;
		userID = -1;
		sendHello(userSocket);

		while (t != quitMSG) { //quit message
			Message msg = receiveMessage(userSocket);
			t = msg.msg_type;
			printf("\n<==TYPE:%d\n", t);
			switch (t) {
			case loginMSG: // login message
				userID = checkLogin(msg.value); // the first user gets the id 0. the 2nd user in the file gets id 1 and so on.
				if (userID != -1) {
					loggedIn = 1;
					sendSuccess(userSocket);
				} else
					sendFail(userSocket);
				break;
			case list_of_filesMSG: // list_of_files request
				if (loggedIn)
					sendFilesList(userSocket, msg, userID);
				else
					sendFail(userSocket);
				break;
			case delete_fileMSG: // delete file request
				if (loggedIn)
					deleteFile(msg, userID);
				else
					sendFail(userSocket);
				break;
			case transfer_fileMSG: // file_transfer (to server)
				if (loggedIn)
					addFile(msg, userID);
				else
					sendFail(userSocket);
				break;
			case get_fileMSG: // file_request (from server)
				sendFail(userSocket);
				break;
			case quitMSG: // quit
				break;
			default: // bad type
				perror("an unknown message type was received.\n");
				if (1 == sendFail(userSocket)) {
					perror("ERROR writing to socket");
					exit(1);
				}
				exit(1);
				break;
			}

		}
		sleep(2);
		printf("Waiting for a user to connect.\n");

	}
}
