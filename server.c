#include "aux.h"
int parseUsersFile(char * users_file, char * users[]) {

	int n = 0;
	char const* const fileName = users_file;
	FILE *file = fopen(fileName, "r");
	if (file < 0) {
		perror("ERROR on opening users file.");
		exit(1);
	}
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
	username[i] = 0;
	return username;
}
Message createHelloMessage() {
	char * str = "Welcome! Please log in.";
	Message msg = createMessagefromString(helloMSG, str);
	return msg;
}
Message createStatusMessage(char * username, char * dataPath) {
	int filesCount = 0;
	DIR * dirp;
	char * path = calloc(1, MAX_FILENAME);
	sprintf(path, "%s/%s/", dataPath, username);
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
	sprintf(str, "Hi %s, you have %d files stored.\n", username, filesCount);

	Message msg = createMessagefromString(statusMSG, str);
	free(path);
	free(str);
	return msg;
}
Message createFileListMessage(char * username, char * dataPath) {
	char * strList = calloc(1, MAX_FILES_PER_USER * (2 + MAX_FILENAME));
	DIR * dirp;
	char * path = calloc(1, MAX_FILENAME);
	sprintf(path, "%s/%s/", dataPath, username);
	struct dirent * entry;
	dirp = opendir(path);
	while ((entry = readdir(dirp)) != NULL) {
		if (entry->d_type == DT_REG) {
			strcat(strList, entry->d_name);
			strcat(strList, "\n");
		}
	}
	closedir(dirp);
	Message msg = createMessagefromString(list_of_files_resMSG, strList);
	free(strList);
	free(path);
	return msg;
}
int deleteFile(char * username, char * filename, char * dataPath) {
	char * fullPath = calloc(1, MAX_USERNAME_LENGTH + MAX_FILENAME);
	sprintf(fullPath, "%s/%s/%s", dataPath, username, filename);
	int ret = remove(fullPath);
	free(fullPath);
	return ret;
}
int addFile(char * username, char * fileNameAndContent, char * dataPath) {
	char * fullPath = calloc(1, MAX_USERNAME_LENGTH + MAX_FILENAME);
	char fileName[MAX_FILENAME];
	char fileContent[MAX_FILE_SIZE];
	int i = 0;
	while (fileNameAndContent[i] != 0) {
		fileName[i] = fileNameAndContent[i];
		i += 1;
	}
	fileName[i] = 0;
	int j = 1;
	while (fileNameAndContent[i + j] != 0) {
		fileContent[j - 1] = fileNameAndContent[i + j];
		j += 1;
	}
	fileContent[j - 1] = 0;
	sprintf(fullPath, "%s/%s/%s", dataPath, username, fileName);
	printf("%s", fileContent);
	FILE *file = fopen(fullPath, "w");
	int res = fputs(fileContent, file);
	fclose(file);
	free(fullPath);
	return (res == EOF);
}
int getFile(char * username, char * filename, char * fileContent,
		char * dataPath) {
	char * fullPath = calloc(1, MAX_USERNAME_LENGTH + MAX_FILENAME);
	sprintf(fullPath, "%s/%s/%s", dataPath, username, filename);

	FILE *file = fopen(fullPath, "r");

	int i = 0;
	char cur;
	while ((cur = fgetc(file)) != EOF) {
		//printf("%s", line);
		fileContent[i] = cur;
		i += 1;
	}
	fclose(file);
	free(fullPath);
	return 0;
}
int connectServer(int port) {
	// connect TCP
	int welcomeSocket;
	struct sockaddr_in serverAddr;

	welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);

	if (welcomeSocket < 0) {
		perror("ERROR on accept");
		exit(1);
	}
	if (setsockopt(welcomeSocket, SOL_SOCKET, SO_REUSEADDR, &(int ) { 1 },
			sizeof(int)) < 0) {
		perror("ERROR on accept");
		exit(1);
	}

	//if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) // fix to bind problem.
	//   error("setsockopt(SO_REUSEADDR) failed");
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	if (bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr))
			< 0) {
		perror("ERROR on binding");
		exit(1);
	}
	return welcomeSocket;
}

int waitForUser(int welcomeSocket) {
	int newSocket;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;

	if (listen(welcomeSocket, 5) == 0)
		printf("Listening\n");
	else {
		perror("ERROR on listen");
		exit(1);
	}

	addr_size = sizeof serverStorage;
	newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage,
			&addr_size);
	if (newSocket < 0) {
		perror("ERROR on accept");
		exit(1);
	}
	return newSocket;
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
	login[i] = ' ';
	i = i + 1;
	while (str[i] && (i < MAX_PASSWORD_LENGTH + MAX_USERNAME_LENGTH + 1)) {
		login[i] = str[i];
		i = i + 1;
	}
	login[i] = '\n';
	login[i + 1] = 0;
	return login;
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
	int i;
	for (i = 0; i < MAX_USERS; i++) {
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
		strcat(folderName, dir_path);
		strcat(folderName, "/");
		strcat(folderName, getUserName(users, j));
		int res = mkdir(folderName, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		if (res < 0) {
			printf("%s", folderName);
			perror("MKDIR failed.");
			//exit(1);
		}
	}
	int userSocket = -1;
	Message inMsg;
	Message outMsg;
	inMsg.msg_type = quitMSG; // we initiate to this value because after quitting we have the same state as in the beginning
	char userID = -1, loggedIn;
	char * loginAttempt;
	char * username;
	char buffer[MAX_FILE_SIZE];
	int serverSocket = connectServer(port);

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
				outMsg = createStatusMessage(username, dir_path);
			} else
				outMsg = createFailMessage();
			free(loginAttempt);
			break;
		case list_of_filesMSG: // list_of_files request
			if (loggedIn) {
				outMsg = createFileListMessage(username, dir_path);
			} else
				outMsg = createFailMessage();
			break;
		case delete_fileMSG: // delete file from server request
			if (loggedIn && (0 == deleteFile(username, inMsg.value, dir_path)))
				outMsg = createSuccessMessage();
			else
				outMsg = createFailMessage();
			break;
		case transfer_fileMSG: // file_transfer (to server)
			if (loggedIn && (0 == addFile(username, inMsg.value, dir_path)))

				outMsg = createSuccessMessage();
			else
				outMsg = createFailMessage();
			break;
		case get_fileMSG: // file_request (from server)
			if (loggedIn
					&& (0 == getFile(username, inMsg.value, buffer, dir_path))) {
				outMsg = createMessagefromString(transfer_fileMSG, buffer);
			} else
				outMsg = createFailMessage();
			break;
		default:
			if (userSocket != -1) //need to close the socket
					{
				if (-1 == close(userSocket)) {
					perror("could not close socket.");
					exit(1);
				} else
					printf("user socket closed,\n");
			}
			userSocket = waitForUser(serverSocket);
			printf("a user was connected.\n");
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
