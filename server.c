#include "header.h"
fd_set read_fds;
char * AUTH[MAX_USERS];
char * DATA_PATH;
int LOGIN[MAX_USERS];
char BUFFER[MAX_FILE_SIZE];
int USERS_NUM = 0;

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
		strcpy(users[n], line);
		n += 1;
	}
	fclose(file);
	return n;
}
char * getUserName(char * users[], int j) {
	if (j > USERS_NUM)
		return "";
	char * username = calloc(sizeof(char), 1 + MAX_USERNAME_LENGTH);
	int i = 0;
	while (i < MAX_USERNAME_LENGTH) {
		if (users[j][i] == '	')
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

Message createUsersOnlineMessage() {
	char * strList = calloc(1, BUFFER_SIZE);
	strcat(strList, "online users: ");
	int i, j, first;
	first = 1;
	for (i = 0; i < USERS_NUM; i++) {
		for (j = 0; j < MAX_USERS; j++) {
			if (LOGIN[j] == i) {
				if (!first)
					strcat(strList, ",");
				strcat(strList, getUserName(AUTH, i));
				first = 0; // to correctly handle ","
			}
		}

	}
	strcat(strList, "\n");
	Message msg = createMessagefromString(usersOnlineResMsg, strList);
	free(strList);
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
	//printf("%s", fileContent);
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
	if (file <= 0) {
		printf("%s - bad path.\n", fullPath);
		return 1;
	}
	int i = 0;
	char cur;
	while ((cur = fgetc(file)) != EOF) {
		fileContent[i] = cur;
		i += 1;
	}
	fclose(file);
	free(fullPath);
	return 0;
}
int make_socket(uint16_t port) {
	int sock;
	struct sockaddr_in name;

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	while (bind(sock, (struct sockaddr *) &name, sizeof(name)) < 0) {
		//perror("bind");
		//exit(EXIT_FAILURE);
	}
	return sock;
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
// str is the received NULL separated string from the user. It returns the properly formated string.
char * formatLoginAttempt(char str[]) {
	//printf("LOGIN ATTEMPT:  %s\n" ,str);
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
	login[i] = '\n';
	login[i + 1] = 0;
	return login;
}
int main(int argc, char *argv[]) {
	fd_set active_fd_set, read_fd_set;
	size_t size;
	struct sockaddr_in clientname;
	int sock;
	bzero(BUFFER, MAX_FILE_SIZE);
	memset(LOGIN, -1, sizeof(int) * MAX_USERS);

	if ((argc != 3) && (argc != 4)) {
		printf("should receive [users-file] [dir] ?[port]. Received %d args",
				argc);
		return 1;
	}
	// get other vars from command
	char * users_file = argv[1];
	DATA_PATH = argv[2];
	int i;
	for (i = 0; i < MAX_USERS; i++) {
		AUTH[i] = calloc(sizeof(char),
		MAX_PASSWORD_LENGTH + MAX_USERNAME_LENGTH + 1);
	}
	int port = 1337;
	if (argc == 4) {
		port = atoi(argv[3]); // get port from cmd arg
	}
	// read users file
	USERS_NUM = parseUsersFile(users_file, AUTH);
	// open a folder for each user
	int j;
	for (j = 0; j < USERS_NUM; j++) {
		char * folderName = calloc(sizeof(char), 256);

		char* fileName = calloc(sizeof(char), 256);
		char* jChar = calloc(sizeof(char), 256);
		strcat(folderName, DATA_PATH);
		strcat(folderName, "/");
		strcpy(fileName, folderName);
		sprintf(jChar, "%d", j);
		strcat(fileName, jChar);
		fopen(fileName, "w+"); //Create file for msgs
		char * temp = getUserName(AUTH, j);
		strcat(folderName, temp);
		free(temp);
		mkdir(folderName, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		free(folderName);
	}
	Message inMsg;
	Message outMsg;
	sock = make_socket(port);
	if (listen(sock, 1) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	FD_ZERO(&active_fd_set);
	FD_SET(sock, &active_fd_set);

	while (1) { //server never stops
		printf("server loop: %s \n ", createUsersOnlineMessage().value);
		/* Block until input arrives on one or more active sockets. */
		read_fd_set = active_fd_set;
		if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
			perror("select");
			exit(EXIT_FAILURE);
		}

		/* Service all the sockets with input pending. */
		for (i = 0; i < FD_SETSIZE; ++i)
			if (FD_ISSET(i, &read_fd_set)) {
				if (i == sock) {
					//printf("new socket:%d\n", i);
					LOGIN[i] = -1;
					/* Connection request on original socket. */
					int new;
					size = sizeof(clientname);
					new = accept(sock, (struct sockaddr *) &clientname, &size);
					if (new < 0) {
						perror("accept");
						exit(EXIT_FAILURE);
					}
					fprintf(stderr, "Server: connect from host %s, port %d.\n",
							inet_ntoa(clientname.sin_addr),
							clientname.sin_port);
					FD_SET(new, &active_fd_set);
					sendMessage(new, createHelloMessage());
				} else {
					printf("serving socket:%d\n", i);
					/* Data arriving on an already-connected socket. */
					inMsg = receiveMessage(i);
					if ((inMsg.msg_type == invalidMSG)
							|| (inMsg.msg_type == quitMSG)) {
						printf("closing socket:%d\n", i);
						LOGIN[i] = 0;
						close(i);
						FD_CLR(i, &active_fd_set);
					} else {
						outMsg = handleClientMsg(inMsg, i);

						if (outMsg.msg_type == invalidMSG) {
							printf("closing socket:%d\n", i);
							LOGIN[i] = 0;
							close(i);
							FD_CLR(i, &active_fd_set);
						}
						sendMessage(i, outMsg);
					}
				}
			}
	}
}

void writeToMyMessages(int userID, int from, char * text) {
	char * str = calloc(BUFFER_SIZE, 1);
	strcat(str, "Message received from ");
	strcat(str, getUserName(AUTH, from));
	strcat(str, ": ");
	strcat(str, text);
	strcat(str, "\n");
	char * filePath = calloc(BUFFER_SIZE, 1);
	char idStr[5];
	sprintf(idStr, "%d", userID);
	strcat(filePath, DATA_PATH);
	strcat(filePath, "/");
	strcat(filePath, idStr);
	printf("wrote:%s to file:%s", str, filePath);
	int fd = open(filePath, O_WRONLY);
	write(fd, str, strlen(str));
	close(fd);
	free(str);
	free(filePath);
	return;
}
int sendChat(int socket, char* fromUser, char* theMsg) {
	char * str = calloc(BUFFER_SIZE, 1);
	strcat(str, "New Message from ");
	strcat(str, fromUser);
	strcat(str, ": ");
	strcat(str, theMsg);
	strcat(str, "\n");
	//printf("sending socket:%d message:%s \n", socket, str);
	Message outMsg = createMessagefromString(sendMsg, str);
	sendMessage(socket, outMsg);
	free(str);
	return 0;
}

Message createReadMsg(int userID) {
	//FILE *fp = MsgsFiles[i] ;
	char * filePath = calloc(BUFFER_SIZE, 1);
	char idStr[5];
	sprintf(idStr, "%d", userID);
	strcat(filePath, DATA_PATH);
	strcat(filePath, "/");
	strcat(filePath, idStr);
	int fp = open(filePath, O_RDONLY);
	if (fp <= 0) {
		perror("File wasn't open\n");
	}
	printf("File opened\n");
	read(fp, BUFFER, BUFFER_SIZE);
	close(fp);
	if (truncate(filePath, 0) == -1) { //Delete file content
		perror("Could not truncate");
	}
	Message msg = createMessagefromString(transfer_fileMSG, BUFFER);
	return msg;
}

Message handleClientMsg(Message inMsg, int socket) {
	Message outMsg;
	char * loginAttempt;
	int j, i;

	switch (inMsg.msg_type) {
	case loginMSG: // login message
		LOGIN[socket] = -1;
		loginAttempt = formatLoginAttempt(inMsg.value);
		for (j = 0; j < USERS_NUM; j++) {
			// check if credentials are correct for one of the users.
			if (0 == strcmp(loginAttempt, AUTH[j])) {
				LOGIN[socket] = j;
				//printf("user ID:%d\n", j);
			}
		}
		if (LOGIN[socket] != -1) {
			return createStatusMessage(getUserName(AUTH, LOGIN[socket]),
					DATA_PATH);
		} else
			return createFailMessage();
		break;
	case list_of_filesMSG: // list_of_files request
		if (LOGIN[socket] != -1) {
			return createFileListMessage(getUserName(AUTH, LOGIN[socket]),
					DATA_PATH);
		} else
			return createFailMessage();
		break;
	case delete_fileMSG: // delete file from server request
		if ((LOGIN[socket] != -1)
				&& (0
						== deleteFile(getUserName(AUTH, LOGIN[socket]),
								inMsg.value, DATA_PATH)))
			return createSuccessMessage();
		else
			return createFailMessage();
		break;
	case transfer_fileMSG: // file_transfer (to server)
		if ((LOGIN[socket] != -1)
				&& (0
						== addFile(getUserName(AUTH, LOGIN[socket]),
								inMsg.value, DATA_PATH)))
			return createSuccessMessage();
		else
			return createFailMessage();
		break;
	case get_fileMSG: // file_request (from server)
		if ((LOGIN[socket] != -1)
				&& (0
						== getFile(getUserName(AUTH, LOGIN[socket]),
								inMsg.value, BUFFER, DATA_PATH))) {
			return createMessagefromString(transfer_fileMSG, BUFFER);
		} else
			return createFailMessage();
		break;
	case usersOnlineReqMsg:
		if (LOGIN[socket] != -1)
			return createUsersOnlineMessage();
		else
			return createFailMessage();
		break;
	case sendMsg:
		i = 0;
		while (inMsg.value[i])
			i++; //skip to user name and get to the message itself;
		i++; //skip the null separator

		// is recipient connected?
		for (j = 0; j < MAX_USERS; j++) {
			if ((LOGIN[j] != -1)
					&& strcmp(getUserName(AUTH, LOGIN[j]), inMsg.value) == 0) {
				sendChat(j, getUserName(AUTH, LOGIN[socket]), inMsg.value + i);
				return createSuccessMessage();
			}
		}

		// is recipient a known user?
		for (j = 0; j < USERS_NUM; j++) {
			if (strcmp(getUserName(AUTH, j), inMsg.value) == 0) {
				writeToMyMessages(j, LOGIN[socket], inMsg.value + i);
				return createSuccessMessage();
			}
		}
		return createFailMessage();
		break;
	case readMsg:
		return createReadMsg(LOGIN[socket]);
		break;
	default:
		return createHelloMessage();
	}
	printMessage(inMsg);
	return outMsg;
}
