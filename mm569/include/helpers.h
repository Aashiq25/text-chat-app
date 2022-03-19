#include "../include/global.h"
#include<vector>

void trim(std::string& input_str);

void PrintAuthor(std::string cmd);

void PrintPortNumber(std::string cmd, int server_socket);

void PrintIpAddress(std::string cmd);

std::string FetchMyIp();

void PrintEndCommand(bool isError, std::string cmd);

std::string FetchHostName(struct sockaddr_in& sa);

char* SerializeConnectedClients(std::vector<ClientMetaInfo>& connected_clients);

bool SortByPortNumber(const ClientMetaInfo& a, const ClientMetaInfo& b);

void PrintClientsList(std::vector<ClientMetaInfo>& clientsList, std::string cmd);