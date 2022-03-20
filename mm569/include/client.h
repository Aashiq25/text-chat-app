#include<vector>
#include<set>
#include "../include/global.h"
class Client {
    public:

        Client(int argc, char **argv);

        int ConnectToHost(std::string server_ip, std::string server_port);
        int InitClient();
        void PrintClientPortNumber(std::string cmd);
        void ParseAvailableClients(std::string msg);
        void SendMessage(std::string msg);
        void BroadCastMessage(std::string msg);
        bool ClientExists(std::string ipAddress);
        void PrintReceivedMessage(std::string incomingMsg);
        void LogoutClient(std::string cmd);
        void BlockClient(std::string msg, std::string cmd);
        void UnBlockClient(std::string msg, std::string cmd);
    private:
        std::string client_port;
        bool isLoggedIn;
        int server;
        std::vector<ClientMetaInfo*> availableClients;
        std::set<std::string> blockedIps;
};