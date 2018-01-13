/*
 * main.h
 *
 *  Created on: Nov 20, 2017
 *      Author: itay
 */
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
#include <dirent.h>

#ifndef MAIN_H_
#define MAIN_H_

#define MAX_USERS 1024
#define MAX_FILES_PER_USER 15
#define MAX_FILE_SIZE 512
#define MAX_USERNAME_LENGTH 25
#define MAX_PASSWORD_LENGTH 25
#define MAX_FILENAME 256
#define BUFFER_SIZE 1024
#define FILE_BUFFER_SIZE 1000

typedef enum {
	helloMSG,
	loginMSG,
	statusMSG,
	list_of_filesMSG,
	list_of_files_resMSG,
	delete_fileMSG,
	successMSG,
	failureMSG,
	transfer_fileMSG,
	get_fileMSG,
	quitMSG,
	invalidMSG
	usersOnlineReqMsg
	usersOnlineResMsg
	sendMsg
	readMsg
} message_type;

typedef struct msg_t {
	char value[BUFFER_SIZE];
	short length;
	message_type msg_type;
	char protocol_id[2];
} Message;

void printMessage(Message msg);

int sendMessage(int userSocket, Message msg) {
	//printf("trying to send:");
	//printMessage(msg);

	char buf[BUFFER_SIZE];
	buf[0] = msg.protocol_id[0];
	buf[1] = msg.protocol_id[1];
	buf[2] = msg.msg_type;
	buf[3] = msg.length / 256;
	buf[4] = msg.length % 256;
	int i = 0;
	while (i <= msg.length) {
		buf[5 + i] = msg.value[i];
		i++;
	}	// message was written to buffer and is ready to be sent.
	int bytesWritten = 0;
	int result;
	while (bytesWritten < 5 + msg.length) {
		result = send(userSocket, &buf[bytesWritten],
				5 + msg.length - bytesWritten, 0);
		if (result < 1) {
			perror("ERROR writing to socket.");
			exit(1);
		}
		bytesWritten += result;
	}
	//printf("sent:");
	//printMessage(msg);

	return 0;
}

Message createMessagefromString(message_type t, char* str) {
	Message msg;
	msg.msg_type = t;
	int i = 0;
	while (str[i]) {
		msg.value[i] = str[i];
		i += 1;
	}
	msg.length = i + 1;
	while (i < BUFFER_SIZE) {
		msg.value[i] = 0;
		i += 1;
	}

	msg.protocol_id[0] = 0x22;
	msg.protocol_id[1] = 0x1e;
	return msg;
}
Message createMessagefromTwoStrings(message_type t, char* str1, char * str2) {
	Message msg;
	msg.msg_type = t;
	int i = 0;
	while (str1[i]) {
		msg.value[i] = str1[i];
		i += 1;
	}
	msg.value[i] = 0;
	i = i + 1;
	int j = 0;
	while (str2[j]) {
		msg.value[i + j] = str2[j];
		j += 1;
	}
	msg.length = i + j + 1;

	while (i + j < BUFFER_SIZE) {
		msg.value[i + j] = 0;
		i += 1;
	}
	msg.protocol_id[0] = 0x22;
	msg.protocol_id[1] = 0x1e;
	return msg;
}

// This assumes buffer is at least x bytes long,
// and that the socket is blocking.
Message receiveMessage(int socket) {
	//printf("receiving MSG now.\n");
	Message msg;
	int minLen = 5;
	int bytesRead = 0;
	int result;
	char buf[BUFFER_SIZE];
	while (bytesRead < minLen) {
		result = read(socket, buf + bytesRead, minLen - bytesRead);
		if (result < 0) {
			perror("read");
			msg.msg_type = invalidMSG;
			return msg;
		}

		bytesRead += result;
	}
	msg.protocol_id[0] = buf[0];
	msg.protocol_id[1] = buf[1];
	msg.msg_type = buf[2];
	msg.length = buf[3] * 256 + buf[4];
	if (msg.length > BUFFER_SIZE) {
		msg.msg_type = invalidMSG;
		return msg;
	}

	while (bytesRead < minLen + msg.length) {
		result = read(socket, buf + bytesRead, minLen + msg.length - bytesRead);
		if (result < 1) {
			perror("Could not read from socket.");
			exit(1);
		}
		bytesRead += result;
	}

	int i = 0;
	while (i + 5 < bytesRead) {
		msg.value[i] = buf[5 + i];
		i++;
	}
	while (i < BUFFER_SIZE) {
		msg.value[i] = 0;
		i += 1;
	}

	//printf("received:");
	//printMessage(msg);

	return msg;
}

void printMessage(Message msg) {
	printf("Protocol ID:%d, type:%d, length:%d\n",
			msg.protocol_id[0] * 256 + msg.protocol_id[1], msg.msg_type,
			msg.length);
}

#endif /* MAIN_H_ */

Message handleClientMsg(Message inMsg, int userID);
