#include "../SERVER/aux.h"

#define DEFAULT_PORT		1337
#define DEFAULT_HOSTNAME	"localhost"
#define MAX_INPUT_MSG_LENGTH	50 //TODO Change?
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
	//printf("Port is %d",port);

	size_t addr_size = sizeof(serv_addr);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, addr_size) < 0) { //Error connecting
		perror("Error starting connection \n");
		error(1);
	}

	return sockfd;
}

void parseInputMsg(char* msg, int sockfd) { //Parse input msg and call appropriate function

	const char s[2] = " ";
	char *token;
	token = strtok(msg, s); //Get first word from input

	if (strcmp(token, "list_of_files") == 0) {
		//list_of_files(sockfd);
	} else if (strcmp(token, "delete_file") == 0) {
		//	token = strtok(NULL, s);
		//delete_file(sockfd, token);
	} else if (strcmp(token, "add_file") == 0) {
		//char* path = token = strtok(NULL, s);
		//	char* filename = token = strtok(NULL, s);
		//add_file(sockfd, path, filename);
	} else if (strcmp(token, "get_file") == 0) {
		//char* filename = token = strtok(NULL, s);
		//	char* path = token = strtok(NULL, s);
		//get_file(sockfd, filename, path);
	} else if (strcmp(token, "quit") == 0) {
		//quit(sockfd);
	}

	else {
		printf("Error: unexpected message\n");
	}

}

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
	int sockfd = initClient(hostname, port);
	if (sockfd == -1) {
		perror("Error starting connection \n");
	}
	char test[10];
	test[0] = 'a';
	int charsWritten = write(sockfd, test, 1);
	printf("%d\n", charsWritten);
	//After welcome message from server
	printf("User: ");
	scanf("%s", username);
	printf("Password: ");
	scanf("%s", password);
	//Msg response now

	//Get continuous input from user and call appropriate function
	char input[MAX_INPUT_MSG_LENGTH];
	fgets(input, MAX_INPUT_MSG_LENGTH, stdin);
	while (strcmp(input, "quit\n") != 0) { //Keep getting input until "quit" is received
		parseInputMsg(input, sockfd);
		fgets(input, MAX_INPUT_MSG_LENGTH, stdin);
	}

	void list_of_files(int Clientsocket) {
		int status;
		Message msg = createMessagefromString(list_of_filesMSG, NULL); //TODO check if I should replace NULL with an empty string

		//Send request to server
		status = sendMessage(Clientsocket, msg);
		if (status == 0) {
			perror("Error sending list_of_files msg");
		}

		//Recieve response
		Message responseMsg = receiveMessage(Clientsocket);
		printf(responseMsg.value);

	}

	void add_file(char* path_to_file, char* newfilename) {
		int status;
		FILE *fp;

		//Open the file and read its content into buffer
		fp = fopen(path_to_file, "r");
		char* buffer = malloc(sizeof(char) * FILE_BUFFER_SIZE);
		fread(buffer, sizeof(char), FILE_BUFFER_SIZE, fp);

		Message msg = createMessagefromString(transfer_fileMSG, newfilename,
				buffer); //TODO check if I should send two args or concatenate the name + file

		//Send request to server
		status = sendMessage(Clientsocket, msg);
		if (status == 0) {
			perror("Error sending transfer_fileMSG msg");
		}

		//Recieve response
		Message responseMsg = receiveMessage(Clientsocket);
		printf(responseMsg.value);

		//Check result of recieved msg from server
if	(responseMsg.msg_type == successMSG)) {
		printf("File added");
	} else {
		printf("Error adding file");
	}

}
}

void delete_file(int Clientsocket, char* filename) {
	int status;
	//Send delete filename msg to server
	Message msg = createMessagefromString(delete_fileMSG, filename);
	status = sendMessage(Clientsocket, msg);
	if (status == 0) {
		perror("Error sending delete_file msg");
	}

	//Recieve response
	Message responseMsg = receiveMessage(Clientsocket);

	//Check result of recieved msg from server
if (responseMsg.msg_type == successMSG)) {
	printf("File removed");
} else {
	printf("No such file exists!");
}
}

void get_file(char* file_name, char* path_to_save) {
	//Get the file from server

}

void quit(int socket) {
	if (close(socket) == -1) {
		printf("close failed \n");
	} else {
		printf("close succeeded \n");
	}
}
}
