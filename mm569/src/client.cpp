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
#include <iostream>
#include <unistd.h>

#include "../include/client.h"
#include "../include/logger.h"
#include "../include/helpers.h"

#define TRUE 1
#define MSG_SIZE 256
#define BUFFER_SIZE 1024
#define STDIN 0

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */

Client::Client(int argc, char **argv)
{
	client_port = std::string(argv[2]);
	isLoggedIn = false;
	server = -1;
}

int Client::InitClient()
{
	int head_socket, selret, sock_index;
	fd_set master_list, watch_list;

	/* Zero select FD sets */
	FD_ZERO(&master_list);
	FD_ZERO(&watch_list);

	/* Register STDIN */
	FD_SET(STDIN, &master_list);

	head_socket = 0;
	while (true)
	{
		memcpy(&watch_list, &master_list, sizeof(master_list));
		cse4589_print_and_log("[PA1-Client@CSE489/589]$ ");
		fflush(stdout);

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

						char *msg = (char *)malloc(sizeof(char) * MSG_SIZE);
						memset(msg, '\0', MSG_SIZE);
						if (fgets(msg, MSG_SIZE - 1, stdin) == NULL) // Mind the newline character that will be written to msg
							exit(-1);

						printf("I got: %s(size:%lu chars)\n", msg, strlen(msg));

						std::string input_command(msg);
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
							PrintClientPortNumber(input_command);
						}
						else if (input_command.substr(0, 5) == "LOGIN")
						{
							if (server != -1 && send(server, input_command.c_str(), input_command.size(), 0) == input_command.size())
							{
								isLoggedIn = true;
							} else {
								std::size_t ip_seperator = input_command.find(" "), port_seperator;
								port_seperator = input_command.find(" ", ip_seperator + 1);
								std::string server_ip = input_command.substr(ip_seperator + 1, port_seperator - ip_seperator - 1);
								std::string server_port = input_command.substr(port_seperator + 1);
								server = ConnectToHost(server_ip, server_port);
								FD_SET(server, &master_list);
								head_socket = server;
							}

						}
						else if (input_command == "EXIT") {
							LogoutClient(input_command);
							close(server);
							FD_CLR(server, &master_list);
							exit(0);
						}
						if (isLoggedIn)
						{
							if (input_command == "LIST")
							{
								PrintClientsList(availableClients, input_command);
							}
							else if (input_command == "REFRESH")
							{
								if (send(server, msg, strlen(msg), 0) == strlen(msg))
								{
									printf("Refresh requested!\n");
								}
							}
							else if (input_command.substr(0, 4) == "SEND")
							{
								SendMessage(input_command);
							} else if (input_command == "LOGOUT") {
								LogoutClient(input_command);
							} else if (input_command.substr(0, 5) == "BLOCK") {
								BlockOrUnblockClient(input_command, input_command.substr(0, 5));
							} else if (input_command.substr(0, 7) == "UNBLOCK") {
								BlockOrUnblockClient(input_command, input_command.substr(0, 7));
							}
						}
					}
					else if (sock_index == server)
					{
						// Incoming messages

						char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
						memset(buffer, '\0', BUFFER_SIZE);
						int recv_status;
						if ((recv_status = recv(server, buffer, BUFFER_SIZE, 0)) > 0)
						{
							
							printf("Server responded: %s\n", buffer);
							ParseAvailableClients(std::string(buffer));

							PrintReceivedMessage(std::string(buffer));
							fflush(stdout);
						}
						else if (recv_status == -1)
						{
							perror("Error occurred while receiving");
						}
						
						free(buffer);
					}
					else
					{
						// P2P new connections
					}
				}
			}
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

void Client::PrintClientPortNumber(std::string cmd)
{
	cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
	cse4589_print_and_log("PORT:%d\n", client_port.c_str());
}

void Client::ParseAvailableClients(std::string msg)
{
	if (msg.size() == 0)
	{
		return;
	}
	availableClients.clear();
	std::string connected_str = "Connected Clients:[";
	std::size_t startIndex = msg.find(connected_str);
	if (startIndex != -1)
	{
		startIndex += connected_str.size();
	} else {
		return;
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

		ClientMetaInfo* ct = new ClientMetaInfo;
		ct->stringToCMI(clientStr);
		availableClients.push_back(ct);

		strSeperator1 = (strSeperator2 != -1 ? strSeperator2 + 1 : -1);
	}
}

void Client::SendMessage(std::string msg)
{
	std::string cmd(msg.substr(0, 4));
	std::size_t ipStart = msg.find(" ") + 1;
	std::size_t ipEnd = msg.find(" ", ipStart);
	std::string ipAddress = msg.substr(ipStart, ipEnd - ipStart);
	bool didSend = false;
	printf("Client ip %s\n", ipAddress.c_str());
	if (ClientExists(ipAddress))
	{
		printf("Client exists\n");
		if (send(server, msg.c_str(), msg.size(), 0) == msg.size())
		{
			cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
			didSend = true;
		}
	}

	PrintEndCommand(!didSend, cmd);
}

bool Client::ClientExists(std::string ipAddress)
{
	for (int i = 0; i < availableClients.size(); i++)
	{
		ClientMetaInfo* client = availableClients[i];
		if (client->ipAddress == ipAddress)
		{
			return true;
		}
	}
	return false;
}

void Client::PrintReceivedMessage(std::string incomingMsg) {
	std::string fromStr = "From:", messageStr = ",Message:";
    std::size_t ipStart = incomingMsg.find(fromStr), msgStart = incomingMsg.find(messageStr);

    if (ipStart != -1 && msgStart != -1)
    {
        ipStart += fromStr.size();
        msgStart += messageStr.size();
    } else {
        return;
    }

    std::size_t senderIpEnd = incomingMsg.find(",");

    std::string senderIp = incomingMsg.substr(ipStart, senderIpEnd-ipStart);
    std::string message = incomingMsg.substr(msgStart);
	cse4589_print_and_log("[RECEIVED:SUCCESS]\n");
	cse4589_print_and_log("msg from:%s\n[msg]:%s\n", senderIp.c_str(), message.c_str());
	cse4589_print_and_log("[RECEIVED:END]\n");

}

void Client::LogoutClient(std::string cmd) {
	if (send(server, cmd.c_str(), cmd.size(), 0) == cmd.size())
	{
		isLoggedIn = false;
		if (cmd == "EXIT") {
			server = -1;
		}
		cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
		cse4589_print_and_log("[%s:END]\n", cmd.c_str());
		
	}
}

void Client::BlockOrUnblockClient(std::string msg, std::string cmd) {
	std::size_t ipStart = msg.find(" ") + 1;
	std::string ipAddress = msg.substr(ipStart);
	bool didInform;
	if (ClientExists(ipAddress))
	{
		if (send(server, msg.c_str(), msg.size(), 0) == msg.size())
		{
			cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
			didInform = true;
		}
	}
	PrintEndCommand(!didInform, cmd);
}