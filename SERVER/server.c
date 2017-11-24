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

Message createHelloMessage() {
	char * str = "Welcome! Please log in.";
	Message msg = createMessagefromString(helloMSG, str);
	return msg;
}

Message createStatusMessage(int userID) {
	//int filesCount = 0;
	char * str = "Hi Bob, you have 6 files stored.";
	Message msg = createMessagefromString(statusMSG, str);
	return msg;
}

Message createFileListMessage() {
	return createMessagefromString(list_of_files_resMSG,
			"Welcome! Please log in.");
}

int deleteFile() {
	return 0;
}
int addFile() {
	return 0;
}
int getFile() {
	return 0;
}
void userQuit() {
}

int checkLogin(char str[]) {
	return 1;
}

int isUserConnected() {
	return 0;
}

int waitForUser(int port) {
	printf("***Waiting for a user to connect***\n");
	// connect TCP
	int newsockfd, clilen;
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
	return newsockfd;
}

Message createFailMessage() {
	Message msg = createMessagefromString(failureMSG, "");
	return msg;
}

Message createSuccessMessage() {
	Message msg = createMessagefromString(successMSG, "");
	return msg;
}

int main(int argc, char *argv[]) {

	if ((argc != 3) && (argc != 4)) {
		printf("should receive 3 or 4 cmd args. Received %d args", argc);
		return 1;
	}
	// get other vars from command
	char * users_file = argv[1];
	//char * dir_path = argv[2];
	char * users[MAX_USERS];

	for (int i = 0; i < MAX_USERS; i++) {
		users[i] = calloc(sizeof(char),
		MAX_PASSWORD_LENGTH + MAX_USERNAME_LENGTH + 1);
	}

	int port = 1337;
	if (argc == 4) {
		port = atoi(argv[3]); // get port from cmd arg
	}

	// read users file
	int usersNum = parseUsersFile(users_file, users);

	// open a folder for each user

	int j;

	for (j = 0; j < usersNum; j++) {
		char * folderName = calloc(sizeof(char), 256);
		strcat(folderName, "SERVER/DATA/");
		strcat(folderName, getUserName(users, j));
		mkdir(folderName, 777);
	}
	int userSocket = -1;
	Message inMsg;
	Message outMsg;
	inMsg.msg_type = quitMSG; // we initiate to this value because after quitting we have the same state as in the beginning
	char userID, loggedIn;

	while (1) { //server never stops
		switch (inMsg.msg_type) {
		case loginMSG: // login message
			userID = checkLogin(inMsg.value); // the first user gets the id 0. the 2nd user in the file gets id 1 and so on.
			if (userID != -1) {
				loggedIn = 1;
				outMsg = createSuccessMessage();
			} else
				outMsg = createFailMessage();
			break;
		case list_of_filesMSG: // list_of_files request
			if (loggedIn)
				outMsg = createFileListMessage(userID);
			else
				outMsg = createFailMessage();
			break;
		case delete_fileMSG: // delete file request
			if (loggedIn && (0 == deleteFile(userID)))
				outMsg = createSuccessMessage();
			else
				outMsg = createFailMessage();
			break;
		case transfer_fileMSG: // file_transfer (to server)
			if (loggedIn && (0 == addFile(userID)))
				outMsg = createSuccessMessage();
			else
				outMsg = createFailMessage();
			break;
		case get_fileMSG: // file_request (from server)
			if (loggedIn && (0 == getFile(userID)))
				outMsg = createSuccessMessage();
			else
				outMsg = createFailMessage();
			break;
		default:
			if (userSocket != -1) //need to close the socket
					{
				if (-1 == close(userSocket)) {
					perror("could not close socket.");
					error(1);
				}
			}
			userSocket = waitForUser(port);
			outMsg = createHelloMessage();
			loggedIn = 0;
			userID = -1;
			break;
		}
		sendMessage(userSocket, outMsg); // reply to the user
		inMsg = receiveMessage(userSocket); //read the next message
	}
}
