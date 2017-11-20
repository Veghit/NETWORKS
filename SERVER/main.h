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


int msgWrite(int,char msg[], char type, char * str);
int writeMsgToSocket(int socket, char msg[], int length);

#endif /* MAIN_H_ */
