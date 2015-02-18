#ifndef DNSRESOLVER_H
#define DNSRESOLVER_H

#include <iostream>
#include <sys/socket.h> //Socket features
#include <netinet/in.h> //Internet-specific features of sockets
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <map>

#include "DNSPacket.h"
#include "ConfigManager.hpp"
#include "Exception.h"

#define MAX_INPUT_SIZE  512
#define DNS_PORT        53

typedef std::vector<std::string> RootServers_t;

class DNSResolver {
public:
    DNSResolver(void) {
        rootServers.clear();
        domain.clear();
    }

    void Initialize(int, char **);

    std::string & GetDomain(void) { return domain; }
    void SetDomain(const std::string & domain) { this->domain = domain; }

    RootServers_t & GetRootServers(void) { return rootServers; }

    std::string & GetRootServer(unsigned int index) {
        return (index >= rootServers.size() ? rootServers.at(0) : rootServers.at(index));
    }

    void PrintClientInfo(Void) {
        std::cout << "Client Info:" << std::endl;
        //std::cout << " --> IP: " << serverIP << std::endl;
        std::cout << " --> Port: " << serverPort << std::endl;
    }

    void PrintServerInfo(void) {
        std::cout << "Server Info:" << std::endl;
        std::cout << " --> IP: " << serverIP << std::endl;
        std::cout << " --> Port: " << serverPort << std::endl;
    }

private:
    ConfigManager configManager;
    RootServers_t rootServers;
    std::string domain;

    unsigned clientPort;                    //Client port
    unsigned serverPort = DNS_PORT;         //Port 53
    std::string serverIP;                   //Server IP
};

#endif//DNSRESOLVER_H
