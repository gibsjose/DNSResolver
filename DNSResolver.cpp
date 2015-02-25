#include "DNSResolver.h"

int main(int argc, char * argv[]) {
    DNSResolver resolver;

    //Set the root servers for the resolver and
    // intitialize the config parser
    resolver.Initialize(argc, argv);

    //Create the client socket
    resolver.CreateClientSocket();

    while(1) {
        DNSPacket request((std::string()));

        try {
            //Wait for client to make a request and store it
            request = resolver.GetClientRequest();
        } catch(const Exception & e) {
            std::cerr << "Error: " << e.what() << std::endl;
            continue;
        }

        //DEBUG
        request.Print();

        //DNSPacket * cache.GetPacket(string)   //ex: www.facebook.com  --> Response Packet
        //std::string & cache.GetAddress(string)  //ex: ns1.cr0.facebook.com --> x.x.x.x
        //std::string & cache.GetAlias(string)    //ex: www.facebook.com --> ns1.cr0.facebook.com
        //void cache.AddPacket(string key, DNSPacket &)
        //void cache.AddAddress(key string, value string, uint32_t ttl);
        //void cache.AddAlias(key string, value string, uint32_t ttl);

        //@TODO CHECK CACHE
        // if(cache.PacketExists(request) != NULL) {
        //     resolver.SendClientResponse(request);
        //     continue;
        // } // ... same for address and alias

        //Response packet
        DNSPacket response((std::string()));

        do {
            //Create the server socket based on the configuration (starts at ROOT)
            try {
                resolver.CreateServerSocket();
                response = resolver.SendServerRequest(request);
            } catch(const Exception & e) {
                std::cerr << e.what() << std::endl;
            }

            //Answer received
            if(response.GetAnswerCount()) {

                //Loop through answers looking for Type A (IPv4)
                for(int i = 0; i < response.GetAnswerCount(); i++) {
                    if(response.GetAnswerSection().at(i).GetType() == TYPE_A) {
                        //@TODO Add to cache

                        //Forward packet to client
                        try {
                            resolver.SendClientResponse(response);
                        } catch(const Exception & e) {
                            std::cerr << e.what() << std::endl;
                        }

                        break;
                    }
                }

                //Loop through answers looking for CNAMEs
                // for(int i = 0; i < response.GetAnswerCount(); i++) {
                //     if(response.GetAnswerSection().at(i).GetType() == TYPE_CNAME) {
                //         //Update the request to use the true name (CNAME)
                //         //...
                //         //@TODO Add to cache
                //         break;
                //     }
                // }
            }

            //No answer: send to the next server up
            else {
                //Send to a new server (name server/new root)
                try {
                    //@TODO Add to cache
                    resolver.UpdateServer(response);
                } catch(const Exception &e) {
                    //Send response back to client
                    std::cerr << e.what() << std::endl;

                    try {
                        resolver.SendClientResponse(response);
                    } catch(const Exception & e) {
                        std::cerr << e.what() << std::endl;
                    }

                    break;
                }
            }

        } while(!response.GetAnswerCount());
    }
}

void DNSResolver::Initialize(int argc, char ** argv) {
    //Create root server address array
    rootServers.push_back("198.41.0.4");            //A.ROOT-SERVERS.NET
    rootServers.push_back("192.228.79.201");        //B.ROOT-SERVERS.NET
    rootServers.push_back("192.33.4.12");           //C.ROOT-SERVERS.NET
    rootServers.push_back("199.7.91.13");           //D.ROOT-SERVERS.NET
    rootServers.push_back("192.203.230.10");        //E.ROOT-SERVERS.NET
    rootServers.push_back("192.5.5.241");           //F.ROOT-SERVERS.NET
    rootServers.push_back("192.112.36.4");          //G.ROOT-SERVERS.NET
    rootServers.push_back("128.63.2.53");           //H.ROOT-SERVERS.NET
    rootServers.push_back("192.36.148.17");         //I.ROOT-SERVERS.NET
    rootServers.push_back("192.58.128.30");         //J.ROOT-SERVERS.NET
    rootServers.push_back("193.0.14.129");          //K.ROOT-SERVERS.NET
    rootServers.push_back("199.7.83.42");           //L.ROOT-SERVERS.NET
    rootServers.push_back("202.12.27.33");          //M.ROOT-SERVERS.NET

    //Initialize to first ROOT server
    serverIP = rootServers.at(0);

    //Config manager: parse arguments
    try {
        configManager.parseArgs(argc, argv);
    }
    catch(Exception & e) {
        std::cerr << "Error: ConfigManager::parseArgs(): " << e.what() << "\n";
        exit(-1);
    }

    clientPort = configManager.getClientPort();
}

void DNSResolver::CreateClientSocket(void) {

    //Create a socket for the client
    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);

    //5 second timeout
    struct timeval to;
    to.tv_sec = 5;
    to.tv_usec = 0;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));

    if(clientSocket < 0) {
        throw SocketException("Unable to create client socket");
    }

    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(clientPort);
    clientAddress.sin_addr.s_addr = INADDR_ANY;

    bind(clientSocket, (struct sockaddr *) &clientAddress, sizeof(clientAddress));
}

DNSPacket DNSResolver::GetClientRequest(void) {
    unsigned int addressLength = sizeof(clientAddress);

    char data[MAX_DNS_LEN];
    memset(&data, 0, MAX_DNS_LEN);

    int bytesReceived = recvfrom(clientSocket, data, MAX_DNS_LEN, 0, (struct sockaddr *)&clientAddress, &addressLength);

    if(bytesReceived < 0) {
        throw SocketException("Error receiving bytes from client: recvfrom()");
    } //ALSO CHECK FOR TIMEOUT

    DNSPacket request(data, bytesReceived);

    return request;
}

void DNSResolver::CreateServerSocket(void) {

    //Create a socket for the server
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);

    //std::cout << "Server socket: " << serverSocket << std::endl;

    //5 second timeout
    struct timeval to;
    to.tv_sec = 5;
    to.tv_usec = 0;
    setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));

    if(serverSocket < 0) {
        throw SocketException("Unable to create server socket");
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    serverAddress.sin_addr.s_addr = inet_addr(serverIP.c_str());

    //DEBUG
    std::cout << "serverSocket = " << serverSocket << std::endl;
    std::cout << "serverPort = " << serverPort << std::endl;
    std::cout << "serverIP = " << serverIP << std::endl;
}

DNSPacket DNSResolver::SendServerRequest(DNSPacket & request) {
    char * requestData = request.GetData();

    int bytesSent = 0;

    bytesSent = sendto(serverSocket, requestData, request.Size(), 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

    if(bytesSent < 0) {
        throw SocketException("Error sending bytes to server: sendto()");
    }

    char responseData[MAX_DNS_LEN];
    memset(&responseData, 0, MAX_DNS_LEN);

    unsigned int addressLength = sizeof(serverAddress);
    int bytesReceived = recvfrom(serverSocket, responseData, MAX_DNS_LEN, 0, (struct sockaddr *)&serverAddress, &addressLength);

    if(bytesReceived < 0) {
        throw SocketException("Error receiving bytes from server: recvfrom()");
    }

    DNSPacket response(responseData, bytesReceived);

    return response;
}

void DNSResolver::SendClientResponse(DNSPacket & response) {
    char * responseData = response.GetData();

    int bytesSent = 0;

    bytesSent = sendto(clientSocket, responseData, response.Size(), 0, (struct sockaddr *)&clientAddress, sizeof(clientAddress));

    if(bytesSent < 0) {
        throw SocketException("Error sending bytes to client: sendto()");
    }
}

void DNSResolver::UpdateServer(DNSPacket & response) {
    static unsigned int rootServerIndex = 0;

    //Check for name servers
    if(response.GetNameServerCount()) {
        std::string nameServer;
        std::string address;

        for(int i = 0; i < response.GetNameServerCount(); i++ ) {
            nameServer = response.GetNameServerSection().at(i).GetRecordData();

            //Look up the IP of the name server in the additional records
            for(int j = 0; j < response.GetAdditionalRecordCount(); j++) {
                if(nameServer == response.GetAdditionalSection().at(j).GetRawName()) {
                    address = response.GetAdditionalSection().at(j).GetRecordData();
                    std::cout << "Found name server address: " << address << std::endl;
                    break;
                } else {
                    address.clear();
                }
            }

            if(!address.empty()) {
                serverIP = address;
                break;
            }
        }

        if(address.empty()) {
            std::cout << "Could not find address for any name server" << std::endl;
            throw GeneralException("Dead end: Could not resolve IP addresses for name servers: DNS Lookup failed");
        }

    } else {
        if(rootServerIndex >= rootServers.size()) {
            throw GeneralException("Root servers exhausted: DNS Lookup failed");
        }

        serverIP = rootServers.at(rootServerIndex);
    }
}
