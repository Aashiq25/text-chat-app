#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include<string>
#include <stdio.h>

#include "../include/helpers.h"
#include "../include/logger.h"

void trim(std::string& input_str) {
    input_str.erase(input_str.find_last_not_of(' ') + 1);
    input_str.erase(0, input_str.find_first_not_of(' '));
    input_str.erase(input_str.find_last_not_of("\n") + 1);
}


void PrintAuthor(std::string cmd) {
    cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
    cse4589_print_and_log("I, mm569, have read and understood the course academic integrity policy.\n");
    cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}

void PrintEndCommand(bool isError, std::string cmd) {
    if (isError) {
        cse4589_print_and_log("[%s:ERROR]\n", cmd.c_str());
    }
    cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}

void PrintPortNumber(std::string cmd, int server_socket) {
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    bool didPrint = false;
    if (getsockname(server_socket, (struct sockaddr *)&sin, &len) != -1) {
        char *ip;
        uint16_t port = ntohs(sin.sin_port);
        if (port > 0) {
            didPrint = true;
            cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
            cse4589_print_and_log("PORT:%d\n", port);
        }
    }
    PrintEndCommand(!didPrint, cmd);

}


void PrintIpAddress(std::string cmd) {
    bool didPrint = false;
    struct addrinfo *address_info;
    int sockfd;
    struct sockaddr_in google_address;

    std::string googleDNS = "8.8.8.8";
    
    // Creating socket connection
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
    }

    google_address.sin_family = AF_INET;
    google_address.sin_addr.s_addr = inet_addr(googleDNS.c_str());
    google_address.sin_port = htons(53);
 
    // Connect to google DNS 
    if (connect(sockfd , (struct sockaddr*) &google_address,  sizeof(google_address)) == 0) {
        struct sockaddr_in sin;
        socklen_t len = sizeof(sin);
        // Get Sock name to fetch IP address
        if (getsockname(sockfd, (struct sockaddr *)&sin, &len) != -1) {
            char ipAddress[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &sin.sin_addr, ipAddress, INET_ADDRSTRLEN);
            cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
            cse4589_print_and_log("IP:%s\n", ipAddress);
            didPrint = true;
        }
    }
    PrintEndCommand(!didPrint, cmd);

 
}