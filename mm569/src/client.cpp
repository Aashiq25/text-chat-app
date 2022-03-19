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
#define BUFFER_SIZE 1024

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */

Client::Client(int argc, char **argv) {
	client_port = std::string(argv[2]);
	isLoggedIn = false;
	server = -1;
}

int Client::InitClient()
{
	while (TRUE)
	{
		cse4589_print_and_log("[PA1-Client@CSE489/589]$ ");
		// fflush(stdout);

		char *msg = (char *)malloc(sizeof(char) * MSG_SIZE);
		memset(msg, '\0', MSG_SIZE);
		if (fgets(msg, MSG_SIZE - 1, stdin) == NULL) // Mind the newline character that will be written to msg
			exit(-1);

		printf("I got: %s(size:%lu chars)\n", msg, strlen(msg));
		
		bool requestServer;
		std::string input_command(msg);
		trim(input_command);
		if (input_command == "AUTHOR") {
			PrintAuthor(input_command);
		} else if (input_command == "IP") {
			PrintIpAddress(input_command);
		} else if (input_command == "PORT") {
			PrintClientPortNumber(input_command);
		} else if (input_command.substr(0, 5) == "LOGIN") {
			std::size_t ip_seperator = input_command.find(" "), port_seperator;
			port_seperator = input_command.find(" ", ip_seperator + 1);
  			std::string server_ip = input_command.substr(ip_seperator + 1, port_seperator - ip_seperator - 1);
			std::string server_port = input_command.substr(port_seperator + 1);
			server = ConnectToHost(server_ip, server_port);
			requestServer = true;
		}

		if (isLoggedIn) {
			if (input_command == "LIST") {
				PrintClientsList(availableClients, input_command);
			} else if (input_command == "REFRESH") {
				requestServer = true;
			} else if (input_command.substr(0, 4) == "SEND") {
				SendMessage(std::string(input_command));
			}
		}
		if (requestServer && server != -1 && send(server, msg, strlen(msg), 0) == strlen(msg)) {
			printf("Done!\n");
		}

		fflush(stdout);
		/* Initialize buffer to receieve response */
		char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
		memset(buffer, '\0', BUFFER_SIZE);
		if (requestServer && recv(server, buffer, BUFFER_SIZE, 0) != -1)
		{
			ParseAvailableClients(std::string(buffer));
			printf("Server responded: %s", buffer);
			requestServer = false;
			fflush(stdout);
		} else {
			perror("Error occurred while receiving");
		}
	}
}

int Client::ConnectToHost(std::string server_ip, std::string server_port)
{
	int fdsocket;
	struct addrinfo hints, *server_addr, *client_addr;

	/* Set up hints structure */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	// Fill Server Address
	if (getaddrinfo(server_ip.c_str(), server_port.c_str(), &hints, &server_addr) != 0)
		perror("getaddrinfo failed");

	// Fill Client Address
	std::string clientIp = FetchMyIp();
	if (getaddrinfo(clientIp.c_str(), client_port.c_str(), &hints, &client_addr) != 0)
		perror("getaddrinfo failed");

	// Create a socket for client
	fdsocket = socket(client_addr->ai_family, client_addr->ai_socktype, client_addr->ai_protocol);
	if (fdsocket < 0)
		perror("Failed to create socket");
	
	if (bind(fdsocket, client_addr->ai_addr, client_addr->ai_addrlen) < 0)
		perror("Bind failed");
	
	if (connect(fdsocket, server_addr->ai_addr, server_addr->ai_addrlen) < 0)
		perror("Connect failed");

	isLoggedIn = true;

	freeaddrinfo(client_addr);

	return fdsocket;
}


void Client::PrintClientPortNumber(std::string cmd) {
	cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
	cse4589_print_and_log("PORT:%d\n", client_port.c_str());
}


void Client::ParseAvailableClients(std::string msg) {
	if (msg.size() == 0) {
		return;
	}
	availableClients.clear();
	std::string connected_str = "Connected Clients:[";
    std::size_t startIndex = msg.find(connected_str);
    if (startIndex != -1) {
        startIndex += connected_str.size();
    }
    std::string availableClientsStr = msg.substr(startIndex, msg.find("]", startIndex) - startIndex);
    
    std::size_t strSeperator1 = 0;
    std::size_t strSeperator2;
    while (strSeperator1 != -1)
    {
        strSeperator2 = availableClientsStr.find("\n", strSeperator1);
        std::string clientStr = availableClientsStr.substr(strSeperator1,
                                                           (strSeperator2 != -1 ? strSeperator2 - strSeperator1
                                                                                : availableClientsStr.size() - strSeperator1));

        ClientMetaInfo ct;
        ct.stringToCMI(clientStr);
        availableClients.push_back(ct);

        strSeperator1 = (strSeperator2 != -1 ? strSeperator2 + 1 : -1);
    }
}


void Client::SendMessage(std::string msg) {
	std::string cmd = msg.substr(0, 4);
    std::size_t ipStart = msg.find(" ") + 1;
    std::size_t ipEnd = msg.find(" ", ipStart);
    std::string ipAddress = msg.substr(ipStart + 1, ipEnd - ipStart - 1), message = msg.substr(ipEnd + 1);
	bool didSend = false;
	if (ClientExists(ipAddress)) {
		if (server != -1 && send(server, msg.c_str(), msg.size(), 0) == msg.size()) {
			cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
			didSend = true;
		}
	}

	PrintEndCommand(!didSend, msg);

}

bool Client::ClientExists(std::string& ipAddress) {
	for (int i = 0; i < availableClients.size(); i++) {
		ClientMetaInfo client = availableClients[i];
		if (client.ipAddress == ipAddress) {
			return true;
		}
	}
	return false;
}