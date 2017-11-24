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

#ifndef MAIN_H_
#define MAIN_H_

#define MAX_USERS 15
#define MAX_FILES_PER_USER 15
#define MAX_FILE_SIZE 512
#define MAX_USERNAME_LENGTH 25
#define MAX_PASSWORD_LENGTH 25
#define BUFFER_SIZE 1000

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
} message_type;

typedef struct msg_t {
	char value[1000];
	short length;
	message_type msg_type;
	char protocol_id[2];
} Message;

int sendMessage(int userSocket, Message msg) {
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
	while (bytesWritten < msg.length) {
		result = write(socket, msg.value + bytesWritten,
				msg.length - bytesWritten);
		if (result < 1) {
			return 1;
		}
		bytesWritten += result;
	}
	printMessage(msg);

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
	msg.length = i;
	msg.protocol_id[0] = 0x22;
	msg.protocol_id[1] = 0x1e;
	return msg;
}

// This assumes buffer is at least x bytes long,
// and that the socket is blocking.
Message receiveMessage(int socket) {
	Message msg;
	int minLen = 5;
	int bytesRead = 0;
	int result;
	char buf[BUFFER_SIZE];
	while (bytesRead < minLen) {
		result = read(socket, buf + bytesRead, minLen - bytesRead);
		if (result < 1) {
			perror("ERROR reading from socket.");
			error(1);
		}

		bytesRead += result;
	}
	msg.protocol_id[0] = buf[0];
	msg.protocol_id[1] = buf[1];
	msg.msg_type = buf[2];
	msg.length = buf[3] * 256 + buf[4];
	if (msg.length>BUFFER_SIZE){
		//perror("invalid message received.");
		msg.msg_type = invalidMSG;
		printMessage(msg);
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

	int i = 5;
	while (i < bytesRead) {
		msg.value[i] = buf[i];
		i++;
	}
	printMessage(msg);
	return msg;
}

void printMessage(Message msg) {
	printf("Protocol ID:%s, type:%c, length:%d, value:%s\n", msg.protocol_id,
			msg.msg_type, msg.length, msg.value);
}

#endif /* MAIN_H_ */
