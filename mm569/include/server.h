#include <vector>
#include<map>
#include<set>
#include <strings.h>
#include <string.h>
#include "../include/global.h"
class Server {
    public:
        Server();
        int InitServer(char *port);
        void AddToConnectedList(struct sockaddr_in& client_addr, int acceptedfd);
        void SendMessageToClient(std::string msg, int fromSocket);
        void ClientLogoutActions(int clientSocket, bool isExit);
        void ReconnectClient(int socketfd);
        void BlockClientActions(int fromSocket, std::string msg);
        void UnBlockClientActions(int fromSocket, std::string msg);
        void PrintBlockedClientsList(std::string msg);
    
    private:
        std::vector<ClientMetaInfo*> connected_clients;
	    std::map<std::string, ServerStatistics*> detailsMap;
	    std::map<int, std::string> fdVsIP;
        std::map<std::string, std::vector<ClientMetaInfo*> > blockInfo;
};