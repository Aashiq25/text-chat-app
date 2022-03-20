#include <vector>
#include<map>
#include <strings.h>
#include <string.h>
#include "../include/global.h"
class Server {
    public:
        Server();
        int InitServer(char *port);

        void AddToConnectedList(struct sockaddr_in& client_addr, int acceptedfd);
        void SendMessageToClient(std::string msg, int fromSocket);
    
    private:
        std::vector<ClientMetaInfo> connected_clients;
	    std::map<std::string, ServerStatistics*> detailsMap;
};