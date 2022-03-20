/**
 * @server
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
 * This file contains the server init and main while loop for tha application.
 * Uses the select() API to multiplex between network I/O and STDIN.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <iostream>
#include <arpa/inet.h>

#include "../include/server.h"
#include "../include/logger.h"
#include "../include/helpers.h"

#define BACKLOG 5
#define STDIN 0
#define TRUE 1
#define CMD_SIZE 100
#define BUFFER_SIZE 256

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */

Server::Server()
{
}

int Server::InitServer(char *port)
{

	int server_socket, head_socket, selret, sock_index, fdaccept = 0, caddr_len;
	struct sockaddr_in client_addr;
	struct addrinfo hints, *res;
	fd_set master_list, watch_list;

	/* Set up hints structure */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	/* Fill up address structures */
	if (getaddrinfo(NULL, port, &hints, &res) != 0)
		perror("getaddrinfo failed");

	/* Socket */
	server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (server_socket < 0)
		perror("Cannot create socket");

	/* Bind */
	if (bind(server_socket, res->ai_addr, res->ai_addrlen) < 0)
		perror("Bind failed");

	freeaddrinfo(res);

	/* Listen */
	if (listen(server_socket, BACKLOG) < 0)
		perror("Unable to listen on port");

	/* ---------------------------------------------------------------------------- */

	/* Zero select FD sets */
	FD_ZERO(&master_list);
	FD_ZERO(&watch_list);

	/* Register the listening socket */
	FD_SET(server_socket, &master_list);
	/* Register STDIN */
	FD_SET(STDIN, &master_list);

	head_socket = server_socket;

	while (TRUE)
	{
		memcpy(&watch_list, &master_list, sizeof(master_list));

		cse4589_print_and_log("[PA1-Server@CSE489/589]$ ");
		fflush(stdout);

		/* select() system call. This will BLOCK */
		selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
		if (selret < 0)
			perror("select failed.");

		/* Check if we have sockets/STDIN to process */
		if (selret > 0)
		{
			/* Loop through socket descriptors to check which ones are ready */
			for (sock_index = 0; sock_index <= head_socket; sock_index += 1)
			{

				if (FD_ISSET(sock_index, &watch_list))
				{

					/* Check if new command on STDIN */
					if (sock_index == STDIN)
					{
						char *cmd = (char *)malloc(sizeof(char) * CMD_SIZE);

						memset(cmd, '\0', CMD_SIZE);
						if (fgets(cmd, CMD_SIZE - 1, stdin) == NULL) // Mind the newline character that will be written to cmd
							exit(-1);

						printf("I got: %s", cmd);

						// Process PA1 commands here ...

						// Get input command
						std::string input_command(cmd);
						trim(input_command);

						if (input_command == "AUTHOR")
						{
							PrintAuthor(input_command);
						}
						else if (input_command == "IP")
						{
							PrintIpAddress(input_command);
						}
						else if (input_command == "PORT")
						{
							PrintPortNumber(input_command, server_socket);
						}
						else if (input_command == "LIST")
						{
							PrintClientsList(connected_clients, input_command);
						} else if (input_command == "STATISTICS") {
							PrintClientStatistics(connected_clients, input_command, detailsMap);
						} else if (input_command.substr(0, 7) == "BLOCKED") {
							PrintBlockedClientsList(input_command);
						}

						free(cmd);
					}
					/* Check if new client is requesting connection */
					else if (sock_index == server_socket)
					{
						caddr_len = sizeof(client_addr);
						fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t *)&caddr_len);
						if (fdaccept < 0)
							perror("Accept failed.");

						printf("\nRemote Host connected!\n");
						AddToConnectedList(client_addr, fdaccept);
						char *serialized_data = SerializeConnectedClients(connected_clients);
						if (send(fdaccept, serialized_data, strlen(serialized_data), 0) == strlen(serialized_data))
						{
							printf("Sent login list to client!\n");
						}

						ProcessBufferMessages(fdaccept);

						/* Add to watched socket list */
						FD_SET(fdaccept, &master_list);
						if (fdaccept > head_socket)
							head_socket = fdaccept;
					}
					/* Read from existing clients */
					else
					{
						/* Initialize buffer to receieve response */
						char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
						memset(buffer, '\0', BUFFER_SIZE);

						if (recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0)
						{
							close(sock_index);
							printf("Remote Host terminated connection!\n");
							/* Remove from watched list */
							FD_CLR(sock_index, &master_list);
						}
						else
						{
							// Process incoming data from existing clients here ...

							std::string client_cmd(buffer);
							trim(client_cmd);
							printf("Client %s", client_cmd.c_str());
							if (client_cmd == "REFRESH")
							{
								char *serialized_data = SerializeConnectedClients(connected_clients);
								if (send(sock_index, serialized_data, strlen(serialized_data), 0) == strlen(serialized_data))
								{
									printf("\nSent updated list to client! %d\n", strlen(serialized_data));
								}
							}
							else if (client_cmd.substr(0, 4) == "SEND")
							{
								SendMessageToClient(std::string(client_cmd), sock_index);
							} else if (client_cmd == "LOGOUT") {
								ClientLogoutActions(sock_index, false);
							} else if (client_cmd == "EXIT") {
								ClientLogoutActions(sock_index, true);
								close(sock_index);
								FD_CLR(sock_index, &master_list);
							} else if (client_cmd.substr(0, 5) == "LOGIN") {
								ReconnectClient(sock_index);
							} else if (client_cmd.substr(0, 5) == "BLOCK") {
								BlockClientActions(sock_index, client_cmd);
							} else if (client_cmd.substr(0, 7) == "UNBLOCK") {
								UnBlockClientActions(sock_index, client_cmd);
							} else if (client_cmd.substr(0, 9) == "BROADCAST") {
								BroadCastMessage(client_cmd, sock_index);
							}

							printf("\nClient sent me: %s\n", buffer);

							fflush(stdout);
						}

						free(buffer);
					}
				}
			}
		}
	}

	return 0;
}

void Server::AddToConnectedList(struct sockaddr_in &client_addr, int acceptedfd)
{
	ClientMetaInfo* clientInfo = new ClientMetaInfo();
	char ipAddress[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(client_addr.sin_addr), ipAddress, INET_ADDRSTRLEN);
	clientInfo->ipAddress = ipAddress;
	clientInfo->hostName = FetchHostName(client_addr);
	clientInfo->isLoggedIn = true;
	char str[100];
	sprintf(str, "%d", ntohs(client_addr.sin_port));
	std::string portNumberOfClient(str);
	clientInfo->portNumber = portNumberOfClient;
	connected_clients.push_back(clientInfo);
	ServerStatistics *ss = new ServerStatistics();
	ss->socket = acceptedfd;
	detailsMap.insert(std::pair<std::string, ServerStatistics *>(clientInfo->ipAddress, ss));
	fdVsIP.insert(std::pair<int, std::string>(acceptedfd, clientInfo->ipAddress));
}

void Server::ReconnectClient(int socketfd) {
	std::string ipAddress = fdVsIP[socketfd];
	for (int i = 0; i < connected_clients.size(); i++) {
		ClientMetaInfo* client = connected_clients[i];
		if (ipAddress == client->ipAddress) {
			client->isLoggedIn = true;
			return;
		}
	}
	char *serialized_data = SerializeConnectedClients(connected_clients);
	if (send(socketfd, serialized_data, strlen(serialized_data), 0) == strlen(serialized_data))
	{
		printf("Sent login list to client!\n");
	}
	ProcessBufferMessages(socketfd);
}

void Server::SendMessageToClient(std::string msg, int fromSocket)
{
	std::string cmd(msg.substr(0, 4));

	std::string senderIp = fdVsIP[fromSocket];
	std::size_t ipStart = msg.find(" ") + 1;
	std::size_t ipEnd = msg.find(" ", ipStart);
	std::string receiverIpAddress = msg.substr(ipStart, ipEnd - ipStart), message = msg.substr(ipEnd + 1);

	
	cse4589_print_and_log("[RELAYED:SUCCESS]\n");
	cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", senderIp.c_str(), receiverIpAddress.c_str(), message.c_str());
	cse4589_print_and_log("[RELAYED:END]\n");
	std::string sendMessage = "From:" + senderIp + ",Message:" + message;

	int senderInMap = -1;
	if (blockInfo.find(receiverIpAddress) != blockInfo.end()) {
		senderInMap = FetchClientMetaIndex(blockInfo[receiverIpAddress], senderIp);
		if (senderInMap != -1) {
			// Blocked IP 
			// TODO check if we have to increment sent count
			return;
		}
	}


	ClientMetaInfo* receiverMeta = FetchClientMeta(connected_clients, receiverIpAddress);

	if (receiverMeta != NULL && receiverMeta->isLoggedIn) {
		if (send(detailsMap[receiverIpAddress]->socket, sendMessage.c_str(), sendMessage.size(), 0) == sendMessage.size())
		{
			detailsMap[senderIp]->sent++;
			detailsMap[receiverIpAddress]->received++;
		}
	} else {
		// Add to Buffer
		detailsMap[senderIp]->sent++;
		bufferMessages[receiverMeta->ipAddress].push_back(sendMessage);
	}

}

void Server::BroadCastMessage(std::string msg, int fromSocket)
{

	std::string senderIp = fdVsIP[fromSocket];
	std::size_t msgStart = msg.find(" ");
	std::string dummyReceiverIp = "255.255.255.255", message = msg.substr(msgStart + 1);

	
	cse4589_print_and_log("[RELAYED:SUCCESS]\n");
	cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", senderIp.c_str(), dummyReceiverIp.c_str(), message.c_str());
	cse4589_print_and_log("[RELAYED:END]\n");

	std::string sendMessage = "From:" + senderIp + ",Message:" + message;

	for (int i = 0; i < connected_clients.size(); i++) {
		ClientMetaInfo* receiverMeta = connected_clients[i];
		int senderInMap = -1;
		if (blockInfo.find(receiverMeta->ipAddress) != blockInfo.end()) {
			senderInMap = FetchClientMetaIndex(blockInfo[receiverMeta->ipAddress], senderIp);
			if (senderInMap != -1) {
				// Blocked IP 
				continue;
			}
		}

		if (receiverMeta->isLoggedIn) {
			if (send(detailsMap[receiverMeta->ipAddress]->socket, sendMessage.c_str(), sendMessage.size(), 0) == sendMessage.size())
			{
				detailsMap[senderIp]->sent++;
				detailsMap[receiverMeta->ipAddress]->received++;
			}

		} else {
			// Add to Buffer
			detailsMap[senderIp]->sent++;
			bufferMessages[receiverMeta->ipAddress].push_back(sendMessage);
		}
	}


}

void Server::ClientLogoutActions(int clientSocket, bool isExit) {
	std::string clientIp = fdVsIP[clientSocket];

	for (int i = 0; i < connected_clients.size(); i++) {
		ClientMetaInfo* client = connected_clients[i];
		if (clientIp == client->ipAddress) {
			if (isExit) {
				connected_clients.erase(connected_clients.begin() + i);
			} else {
				client->isLoggedIn = false;
			}
			break;
		}
	}
	if (isExit) {
		detailsMap.erase(clientIp);
		fdVsIP.erase(clientSocket);
	}
	
}

void Server::BlockClientActions(int fromSocket, std::string msg) {
	std::string cmd(msg.substr(0, 5));

	std::string senderIP = fdVsIP[fromSocket];

	std::size_t ipStart = msg.find(" ") + 1;
	std::string receiverIpAddress = msg.substr(ipStart);

	ClientMetaInfo* receiver = FetchClientMeta(connected_clients, receiverIpAddress);

	if (receiver != NULL) {
		int receiverInMap = -1;
		if (blockInfo.find(senderIP) != blockInfo.end()) {
			receiverInMap = FetchClientMetaIndex(blockInfo[senderIP], receiverIpAddress);
		}
		if (receiverInMap == -1) {
			blockInfo[senderIP].push_back(receiver);
		}
	}

}

void Server::UnBlockClientActions(int fromSocket, std::string msg) {
	std::string cmd(msg.substr(0, 7));

	std::string senderIP = fdVsIP[fromSocket];

	std::size_t ipStart = msg.find(" ") + 1;
	std::string receiverIpAddress = msg.substr(ipStart);

	int client = FetchClientMetaIndex(connected_clients, receiverIpAddress);

	if (client != -1) {
		int receiverIndex = -1;
		if (blockInfo.find(senderIP) != blockInfo.end()) {
			receiverIndex = FetchClientMetaIndex(blockInfo[senderIP], receiverIpAddress);
		}
		if (receiverIndex != -1) {
			blockInfo[senderIP].erase(blockInfo[senderIP].begin() + receiverIndex);
		}
	}
}

void Server::PrintBlockedClientsList(std::string msg) {
	std::string cmd(msg.substr(0, 7));


	std::size_t ipStart = msg.find(" ") + 1;
	std::string clientIpAddress = msg.substr(ipStart);
	bool didPrint = false;
	if (IsValidIpAddress(clientIpAddress)) { 
		int clientIndex = FetchClientMetaIndex(connected_clients, clientIpAddress);
		if (clientIndex != -1) {
			PrintClientsList(blockInfo[clientIpAddress], cmd);
			didPrint = true;
		}
	}
	if (!didPrint) {
		PrintEndCommand(!didPrint, cmd);
	}

}

void Server::ProcessBufferMessages(int socketfd) {
	std::string clientIp = fdVsIP[socketfd];
	for (int i = 0; i < bufferMessages[clientIp].size(); i++) {
		std::string sendMessage = bufferMessages[clientIp][i];
		if (send(socketfd, sendMessage.c_str(), sendMessage.size(), 0) == sendMessage.size())
		{
			sleep(1);
			detailsMap[clientIp]->received++;
		}
	}
	bufferMessages[clientIp].clear();
}