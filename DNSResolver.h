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

#define MAX_INPUT_SIZE 512

typedef std::map<std::string, std::string> RootServerMap_t;

class DNSResolver {
public:
    DNSResolver(void) {
        rootServers.clear();
    }

    RootServerMap_t & GetRootServers(void) { return rootServers; }
private:
    RootServerMap_t rootServers;
};

#endif//DNSRESOLVER_H
