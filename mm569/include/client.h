#include<vector>
#include "../include/global.h"

int connect_to_host(std::string server_ip, std::string server_port, std::string client_port, bool& isLoggedIn);

int initClient(int argc, char **argv);

void PrintClientPortNumber(std::string cmd, char* port);

void ParseAvailableClients(std::string msg, std::vector<ClientMetaInfo>& availableClients);