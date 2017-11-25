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
Message createStatusMessage(char * username) {
	int filesCount = 0;
	DIR * dirp;
	char * path = calloc(1, MAX_FILENAME);
	sprintf(path, "SERVER/DATA/%s/", username);
	//printf("trying to open %s",path);
	struct dirent * entry;
	dirp = opendir(path);
	while ((entry = readdir(dirp)) != NULL) {
		if (entry->d_type == DT_REG) {
			filesCount++;
		}
	}
	closedir(dirp);
	char * str = calloc(1, MAX_FILENAME);
	sprintf(str, "Hi %s, you have %d files stored.", username, filesCount);

	Message msg = createMessagefromString(statusMSG, str);
	return msg;
}
Message createFileListMessage(char * username) {
	char * strList = calloc(1, MAX_FILES_PER_USER * (2 + MAX_FILENAME));
	DIR * dirp;
	char * path = calloc(1, MAX_FILENAME);
	sprintf(path, "SERVER/DATA/%s/", username);
	struct dirent * entry;
	dirp = opendir(path);
	while ((entry = readdir(dirp)) != NULL) {
		if (entry->d_type == DT_REG) {
			strcat(strList, "\n");
			strcat(strList, entry->d_name);
		}
	}
	closedir(dirp);
	return createMessagefromString(list_of_files_resMSG, strList);
}
int deleteFile(char * username, char * filename) {
	char * fullPath = calloc(1, MAX_USERNAME_LENGTH + MAX_FILENAME);
	sprintf(fullPath, "SERVER/DATA/%s/%s", username, filename);
	return remove(fullPath);
}
int addFile(char * username, char * filename, char * fileContent) {
	char * fullPath = calloc(1, MAX_USERNAME_LENGTH + MAX_FILENAME);
	sprintf(fullPath, "SERVER/DATA/%s/%s", username, filename);

	FILE *file = fopen(fullPath, "w");
	int res = fputs(fileContent, file);
	fclose(file);
	return (res == EOF);
}
int getFile(char * username, char * filename, char * fileContent) {
	char * fullPath = calloc(1, MAX_USERNAME_LENGTH + MAX_FILENAME);
	sprintf(fullPath, "SERVER/DATA/%s/%s", username, filename);

	FILE *file = fopen(fullPath, "r");

	int i = 0;
	char cur;
	while ((cur = fgetc(file)) != EOF) {
		//printf("%s", line);
		fileContent[i] = cur;
		i += 1;
	}
	fclose(file);
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
char * formatLoginAttempt(char str[]) {
	char * login = calloc(1, MAX_PASSWORD_LENGTH + MAX_USERNAME_LENGTH + 1);
	int i = 0;
	while (str[i]) {
		login[i] = str[i];
		i = i + 1;
	}
	login[i] = '\t';
	i = i + 1;
	while (str[i] && (i < MAX_PASSWORD_LENGTH + MAX_USERNAME_LENGTH + 1)) {
		login[i] = str[i];
		i = i + 1;
	}
	return login;
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
		mkdir(folderName, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}
	int userSocket = -1;
	Message inMsg;
	Message outMsg;
	inMsg.msg_type = quitMSG; // we initiate to this value because after quitting we have the same state as in the beginning
	char userID = -1, loggedIn;
	char * loginAttempt;
	char * username;
	char * buffer[MAX_FILE_SIZE];
	while (1) { //server never stops
		switch (inMsg.msg_type) {
		case loginMSG: // login message
			loginAttempt = formatLoginAttempt(inMsg.value);
			for (j = 0; j < usersNum; j++) {
				if (0 == strcmp(loginAttempt, users[j]))
					userID = j;
			}
			if (userID != -1) {
				username = getUserName(users, userID);
				loggedIn = 1;
				outMsg = createStatusMessage(username);
			} else
				outMsg = createFailMessage();
			free(loginAttempt);
			break;
		case list_of_filesMSG: // list_of_files request
			if (loggedIn) {
				outMsg = createFileListMessage(username);
			} else
				outMsg = createFailMessage();
			break;
		case delete_fileMSG: // delete file from server request
			if (loggedIn && (0 == deleteFile(username, inMsg.value)))
				outMsg = createSuccessMessage();
			else
				outMsg = createFailMessage();
			break;
		case transfer_fileMSG: // file_transfer (to server)
			if (loggedIn && (0 == addFile(username, inMsg.value, buffer)))
				outMsg = createSuccessMessage();
			else
				outMsg = createFailMessage();
			break;
		case get_fileMSG: // file_request (from server)
			if (loggedIn && (0 == getFile(username, inMsg.value, buffer))) {
				outMsg = createMessagefromTwoStrings(transfer_fileMSG, inMsg.value,
						buffer);
			} else
				outMsg = createFailMessage();
			break;
		default:
			if (userSocket != -1) //need to close the socket
					{
				if (-1 == close(userSocket)) {
					perror("could not close socket.");
					exit(1);
				}
			}
			userSocket = waitForUser(port);
			outMsg = createHelloMessage();
			loggedIn = 0;
			userID = -1;
			username = "";
			break;
		}
		sendMessage(userSocket, outMsg); // reply to the user
		inMsg = receiveMessage(userSocket); //read the next message
	}
}
