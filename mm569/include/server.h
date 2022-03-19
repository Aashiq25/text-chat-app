#include <vector>
#include "../include/global.h"

int initServer(char *port);

void AddToConnectedList(struct sockaddr_in& client_addr, std::vector<ClientMetaInfo>& connected_clients);
