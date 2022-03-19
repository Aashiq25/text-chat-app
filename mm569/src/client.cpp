/**
 * @client
 * @author  Swetank Kumar Saha <swetankk@buffalo.edu>, Shivang Aggarwal <shivanga@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This file contains the client.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include<iostream>

#include "../include/client.h"
#include "../include/logger.h"
#include "../include/helpers.h"

#define TRUE 1
#define MSG_SIZE 256
#define BUFFER_SIZE 256

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int initClient(int argc, char **argv)
{
	// if (argc != 3)
	// {
	// 	printf("Usage:%s [ip] [port]\n", argv[0]);
	// 	exit(-1);
	// }

	

	while (TRUE)
	{
		cse4589_print_and_log("[PA1-Client@CSE489/589]$ ");
		fflush(stdout);

		char *msg = (char *)malloc(sizeof(char) * MSG_SIZE);
		memset(msg, '\0', MSG_SIZE);
		if (fgets(msg, MSG_SIZE - 1, stdin) == NULL) // Mind the newline character that will be written to msg
			exit(-1);

		printf("I got: %s(size:%lu chars)", msg, strlen(msg));
		int server = -1;
		std::string input_command(msg);
		trim(input_command);
		if (input_command == "AUTHOR") {
			PrintAuthor(input_command);
		} else if (input_command == "IP") {
			PrintIpAddress(input_command);
		} else if (input_command == "PORT") {
			PrintClientPortNumber(input_command, argv[2]);
		} else if (input_command.substr(0, 5) == "LOGIN") {
			std::cout<<"Received Login ";
			std::size_t ip_seperator = input_command.find(" "), port_seperator;
			port_seperator = input_command.find(" ", ip_seperator + 1);
  			std::string server_ip = input_command.substr(ip_seperator + 1, port_seperator - ip_seperator - 1);
			std::string port = input_command.substr(port_seperator + 1);


			server = connect_to_host(server_ip, port);
		}
		if (server != -1 && send(server, msg, strlen(msg), 0) == strlen(msg))
			printf("Done!\n");
		fflush(stdout);

		/* Initialize buffer to receieve response */
		char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
		memset(buffer, '\0', BUFFER_SIZE);

		if (recv(server, buffer, BUFFER_SIZE, 0) >= 0)
		{
			printf("Server responded: %s", buffer);
			fflush(stdout);
		}
	}
}

int connect_to_host(std::string server_ip, std::string server_port)
{
	cse4589_print_and_log("Server IP:%s Port:%s", server_ip.c_str(), server_port.c_str());
	int fdsocket;
	struct addrinfo hints, *res;

	/* Set up hints structure */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	/* Fill up address structures */
	if (getaddrinfo(server_ip.c_str(), server_port.c_str(), &hints, &res) != 0)
		perror("getaddrinfo failed");

	/* Socket */
	fdsocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (fdsocket < 0)
		perror("Failed to create socket");

	/* Connect */
	if (connect(fdsocket, res->ai_addr, res->ai_addrlen) < 0)
		perror("Connect failed");

	freeaddrinfo(res);

	return fdsocket;
}


void PrintClientPortNumber(std::string cmd, char* port) {
	cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
	cse4589_print_and_log("PORT:%d\n", port);
}
