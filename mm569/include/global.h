#ifndef GLOBAL_H_
#define GLOBAL_H_

#define HOSTNAME_LEN 128
#define PATH_LEN 256
#include<string>

struct ClientMetaInfo
{
    std::string ipAddress, hostName, portNumber;
    bool isLoggedIn;

    std::string metaAsString()
    {
        return "ipAddress:" + ipAddress + ",hostName:" + hostName + ",isLoggedIn:" + (isLoggedIn ? "1" : "0") + ",portNumber:" + portNumber;
    }

    void stringToCMI(std::string metaStr)
    {
        std::size_t strSeperator1 = 0;
        std::size_t strSeperator2;
        while (strSeperator1 != -1)
        {
            strSeperator2 = metaStr.find(",", strSeperator1);
            std::string fieldWithValue = metaStr.substr(strSeperator1,
                                                        (strSeperator2 != -1 ? strSeperator2 - strSeperator1
                                                        : metaStr.size() - strSeperator1));

            std::size_t valueSeperator = fieldWithValue.find(":");
            std::string fieldName = fieldWithValue.substr(0, valueSeperator);
            std::string value = fieldWithValue.substr(valueSeperator + 1);

            if (fieldName == "ipAddress") {
                ipAddress = value;
            } else if (fieldName == "hostName") {
                hostName = value;
            } else if (fieldName == "portNumber") {
                portNumber = value;
            } else if (fieldName == "isLoggedIn") {
                if (value == "1") {
                    isLoggedIn = true;
                } else {
                    isLoggedIn = false;
                }
            }
            strSeperator1 = (strSeperator2 != -1 ? strSeperator2 + 1 : -1);
        }
    }
};

struct MessageCount {
    int received, sent;
};

#endif
