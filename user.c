#include "header.h"

#define DEFAULT_PORT		1337
#define DEFAULT_HOSTNAME	"localhost"
#define MAX_INPUT_MSG_LENGTH	60 

//Function declaration 
void quit(int clientSocket);
int initClient(char* ip, int port);
void parseInputMsg(char* msg, int sockfd);
void list_of_files(int clientSocket);
void add_file(int clientSocket, char* path_to_file, char* newFileName);
void delete_file(int clientSocket, char* filename);
void get_file(int clientSocket, char* file_name, char* path_to_save);
void users_online(int clientSocket);
void msg(int clientSocket, char* user_name_we_send_to, char* the_message);
void read_msgs(int clientSocket);


int main(int argc, char *argv[]) {
	if ((argc != 3) && (argc != 1)) {
		printf("should receive 2 or 0 cmd args. Received %d args", argc);
	}

	char* hostname = DEFAULT_HOSTNAME;
	int port = DEFAULT_PORT;
	char username[MAX_USERNAME_LENGTH] = "";
	char password[MAX_PASSWORD_LENGTH] = "";

	if (argc == 3) {
		hostname = argv[1];
		port = atoi(argv[2]);
	}
	int clientSocket = initClient(hostname, port);
	if (clientSocket == -1) {
		perror("Error starting connection \n");
	}

	//recieve hello msg from server
	Message responseMsg = receiveMessage(clientSocket);
	if (responseMsg.msg_type == helloMSG) {
		printf("Welcome! Please log in.\n");
	}
	if (responseMsg.msg_type != helloMSG) {
		printf("Error receiving hello msg \n");
	}

	int successLogin = 0;

	//Keep getting username and password until authentication succeeds 
	while (successLogin == 0) {
		//Get login input from user and send login msg to server

		//printf("User: ");
		//scanf("%s", username);
		//printf("Password: ");
		//scanf("%s", password);
		char * token;
		fgets(username, MAX_INPUT_MSG_LENGTH, stdin);
		token = strtok(username, " ");
		if (strcmp(token, "User:") == 0) {
			token = strtok(NULL, " ");
			strcpy(username, token);
			fgets(password, MAX_INPUT_MSG_LENGTH, stdin);
			token = strtok(password, " ");
			if (strcmp(token, "Password:") == 0) {
				token = strtok(NULL, " ");
				strcpy(password, token);
				if ((strlen(username) > 0) && strlen(username) > 0) {
					username[strlen(username) - 1] = 0;
					password[strlen(password) - 1] = 0;
				}
			}
		}
		//printf("(%s,%s)", username, password);

		int status;
		Message msg = createMessagefromTwoStrings(loginMSG, username, password);
		//Send request to server
		status = sendMessage(clientSocket, msg);
		if (status != 0) {
			perror("Error sending login msg");
		}

		//Recieve login result from server
		responseMsg = receiveMessage(clientSocket);
		if ((responseMsg.msg_type == statusMSG)) {
			printf("%s", responseMsg.value);
			successLogin = 1;
		} else {
			printf("Authentication failed\n");

		}
	}
	//Get continuous input from user and call appropriate function
	char input[MAX_INPUT_MSG_LENGTH];
	//fgets(input, MAX_INPUT_MSG_LENGTH, stdin);
	while (strcmp(input, "quit") != 0) { //Keep getting input until "quit" is received
		fgets(input, MAX_INPUT_MSG_LENGTH, stdin);
		if (strcmp(input, "\n") != 0)
			parseInputMsg(input, clientSocket);
	}

	return 0;
}

int initClient(char* ip, int port) { //initialize connection, returns -1 on errors, otherwise socket
	int sockfd;
	struct sockaddr_in serv_addr;
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) //Error creating socket
			{
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port); // change?
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //change?
	bzero(&(serv_addr.sin_zero), 8);

	size_t addr_size = sizeof(serv_addr);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, addr_size) < 0) { //Error connecting
		perror("Error starting connection \n");
		exit(1);
	}

	return sockfd;
}

void parseInputMsg(char msg[], int sockfd) { //Parse input msg and call appropriate function

	char *token;
	msg[strlen(msg) - 1] = 0;
	token = strtok(msg, " "); //Get first word from input

	if (strcmp(token, "list_of_files") == 0) {
		list_of_files(sockfd);
	} else if (strcmp(token, "delete_file") == 0) {
		token = strtok(NULL, " ");
		delete_file(sockfd, token);
	} else if (strcmp(token, "add_file") == 0) {
		char* path;
		char* filename;
		path = strtok(NULL, " ");
		filename = strtok(NULL, " ");
		add_file(sockfd, path, filename);
	} else if (strcmp(token, "get_file") == 0) {
		char* filename = strtok(NULL, " ");
		char* path = strtok(NULL, " ");
		get_file(sockfd, filename, path);
	} else if (strcmp(token, "quit") == 0) {
		quit(sockfd);
	}

	else {
		printf("Error: unexpected message\n");
	}

}

void users_online(int clientSocket){
	int status; 
	Message msg = createMessagefromString(usersOnlineReqMsg, "");
	
	//Send request to server
	status = sendMessage(clientSocket, msg);
	if (status != 0) {
		perror("Error sending usersOnlineReqMsg msg");
	}

	//Recieve response
	Message responseMsg = receiveMessage(clientSocket);
	printf("%s", responseMsg.value);

	
}

void msg(int clientSocket, char* user_name_we_send_to, char* the_message){
	int status; 
	Message msg = createMessagefromString(sendMsg, the_message);
	
	//Send request to server
	status = sendMessage(clientSocket, msg);
	if (status != 0) {
		perror("Error sending sendMsg msg");
	}

	//Recieve response
	Message responseMsg = receiveMessage(clientSocket);
	printf("%s", responseMsg.value);
}

void read_msgs(int clientSocket){
	int status; 
	Message msg = createMessagefromString(readMsg,"");
	
	//Send request to server
	status = sendMessage(clientSocket, msg);
	if (status != 0) {
		perror("Error sending readMsg msg");
	}

	//Recieve response
	Message responseMsg = receiveMessage(clientSocket);
	printf("%s", responseMsg.value);
}

void list_of_files(int clientSocket) {
	int status;
	Message msg = createMessagefromString(list_of_filesMSG, "");

	//Send request to server
	status = sendMessage(clientSocket, msg);
	if (status != 0) {
		perror("Error sending list_of_files msg");
	}

	//Recieve response
	Message responseMsg = receiveMessage(clientSocket);
	printf("%s", responseMsg.value);

}

void add_file(int clientSocket, char* path_to_file, char* newFileName) {
	int status;
	FILE *fp;

	//Open the file and read its content into buffer
	fp = fopen(path_to_file, "r");
	if (fp <= 0) {
		printf("%s", path_to_file);
		perror("can't find entered path.\n");
		return;
	}
	char* buffer = malloc(sizeof(char) * BUFFER_SIZE);
	fread(buffer, sizeof(char), BUFFER_SIZE, fp);
	fclose(fp);

	Message msg = createMessagefromTwoStrings(transfer_fileMSG, newFileName,
			buffer);

	//Send request to server
	status = sendMessage(clientSocket, msg);
	if (status != 0) {
		perror("Error sending transfer_fileMSG msg");
	}

	//Recieve response
	Message responseMsg = receiveMessage(clientSocket);
	printf("%s", responseMsg.value);

	//Check result of recieved msg from server
	if (responseMsg.msg_type == successMSG) {
		printf("File added\n");
	} else {
		printf("Error adding file.\n");
	}
	free(buffer);
}

void delete_file(int clientSocket, char* filename) {
	int status;
	//Send delete filename msg to server
	Message msg = createMessagefromString(delete_fileMSG, filename);
	status = sendMessage(clientSocket, msg);
	if (status != 0) {
		perror("Error sending delete_file msg");
	}

	//Recieve response
	Message responseMsg = receiveMessage(clientSocket);

	//Check result of recieved msg from server
	if (responseMsg.msg_type == successMSG) {
		printf("File removed\n");
	} else {
		printf("No such file exists!\n");
	}
}

void get_file(int clientSocket, char* file_name, char* path_to_save) {

	int status;
	//Send get file request to server
	Message msg = createMessagefromString(get_fileMSG, file_name);
	status = sendMessage(clientSocket, msg);
	if (status != 0) {
		perror("Error sending get_fileMSG msg");
	}

	//Recieve response from server
	Message responseMsg = receiveMessage(clientSocket);

	//Check result of recieved msg from server
	if (responseMsg.msg_type == failureMSG) {
		printf("Error getting file.\n");
	} else {
		//Get file content from server and save it
		char* buffer = malloc(sizeof(char) * BUFFER_SIZE);
		strcpy(buffer, responseMsg.value); //Get file content into buffer
		FILE *fp;
		char full_path[MAX_FILENAME];
		sprintf(full_path, "%s/%s", path_to_save, file_name);
		fp = fopen(full_path, "w");
		if (fp <= 0) {
			printf("%s - bad path.\n", full_path);
			return;
		}
		fwrite(buffer, sizeof(char), responseMsg.length - 1, fp);
		fclose(fp);
		free(buffer);
		printf("File saved\n");
	}

}

void quit(int clientSocket) {
	int status;
	//Send quit msg to server
	Message msg = createMessagefromString(quitMSG, "");
	status = sendMessage(clientSocket, msg);
	if (status != 0) {
		perror("Error sending quitMSG msg");
	}
	if (close(clientSocket) == -1) {
		printf("close failed.\n");
	} else {
		//printf("close succeeded.\n");
	}
}
