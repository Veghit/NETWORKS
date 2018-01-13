#include "header.h"
fd_set read_fds;
char * AUTH[MAX_USERS];
char * DATA_PATH;
char LOGIN[MAX_USERS];
char BUFFER[MAX_FILE_SIZE];

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
	if (bind(sock, (struct sockaddr *) &name, sizeof(name)) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
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
	int usersNum = parseUsersFile(users_file, AUTH);
	// open a folder for each user
	int j;
	for (j = 0; j < usersNum; j++) {
		char * folderName = calloc(sizeof(char), 256);
		strcat(folderName, DATA_PATH);
		strcat(folderName, "/");
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
		printf("server loop \n");
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
					printf("new socket:%d\n", i);
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

Message handleClientMsg(Message inMsg, int socket) {
	Message outMsg;
	char * loginAttempt;
	int j, usersNum, userID = LOGIN[socket];

	switch (inMsg.msg_type) {
	case loginMSG: // login message
		loginAttempt = formatLoginAttempt(inMsg.value);
		for (j = 0; j < usersNum; j++) {
			if (0 == strcmp(loginAttempt, AUTH[j]))
				LOGIN[socket] = j;
		}
		if (LOGIN[socket] != -1) {
			outMsg = createStatusMessage(getUserName(AUTH, userID), DATA_PATH);
		} else
			outMsg = createFailMessage();
		break;
	case list_of_filesMSG: // list_of_files request
		if (LOGIN[socket] != -1) {
			outMsg = createFileListMessage(getUserName(AUTH, userID),
					DATA_PATH);
		} else
			outMsg = createFailMessage();
		break;
	case delete_fileMSG: // delete file from server request
		if ((LOGIN[socket] != -1)
				&& (0
						== deleteFile(getUserName(AUTH, userID), inMsg.value,
								DATA_PATH)))
			outMsg = createSuccessMessage();
		else
			outMsg = createFailMessage();
		break;
	case transfer_fileMSG: // file_transfer (to server)
		if ((LOGIN[socket] != -1)
				&& (0
						== addFile(getUserName(AUTH, userID), inMsg.value,
								DATA_PATH)))
			outMsg = createSuccessMessage();
		else
			outMsg = createFailMessage();
		break;
	case get_fileMSG: // file_request (from server)
		if ((LOGIN[socket] != -1)
				&& (0
						== getFile(getUserName(AUTH, userID), inMsg.value,
								BUFFER, DATA_PATH))) {
			outMsg = createMessagefromString(transfer_fileMSG, BUFFER);
		} else
			outMsg = createFailMessage();
		break;
	default:
		outMsg = createHelloMessage();
	}
	printMessage(inMsg);
	return outMsg;
}
