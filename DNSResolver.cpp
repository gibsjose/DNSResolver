#include "DNSResolver.h"


int main(int argc, char * argv[]) {
    DNSResolver resolver;
    DNSCache cache;

    //Set the root servers for the resolver and
    // intitialize the config parser
    resolver.Initialize(argc, argv);

    //Create the client socket
    resolver.CreateClientSocket();
    DNSPacket request((std::string()));
    int flag = 1;
    while(1) {

        resolver.Initialize(argc,argv);
        if(flag != 1) {
            DNSPacket request((std::string()));
        }
        QuestionRecord lOriginalQuestionRecord;
        try {
            //Wait for client to make a request and store it
            request = resolver.GetClientRequest();
            lOriginalQuestionRecord.EncodeName(request.GetQuestionSection()[0].GetRawName());
            lOriginalQuestionRecord.SetType(request.GetQuestionSection()[0].GetType());
            lOriginalQuestionRecord.SetClass(request.GetQuestionSection()[0].GetClass());

            request.UnsetRecursionFlag();
            request.UnsetZFlags();  // Clear the Z flag bits (reserver for "future" use <-- "dig" sets these)
        } catch(const Exception & e) {
            std::cerr << "Error: " << e.what() << std::endl;
            continue;
        }

        //DEBUG
        // std::cout << "Request:\n-----------------------\n" << std::endl;
        // request.Print();

        // DNSPacket * cache.GetPacket(string)   //ex: www.facebook.com  --> Response Packet
        // std::string & cache.GetAddress(string)  //ex: ns1.cr0.facebook.com --> x.x.x.x
        // std::string & cache.GetAlias(string)    //ex: www.facebook.com --> ns1.cr0.facebook.com
        // void cache.AddPacket(string key, DNSPacket &)
        // void cache.AddAddress(key string, value string, uint32_t ttl);
        // void cache.AddAlias(key string, value string, uint32_t ttl);

        //Response packet
        DNSPacket response((std::string()));
        //@TODO CHECK CACHE

        try {
            flag = 1;
            cache.Print();
            response = *cache.GetPacket(request.GetDomain());
            response.SetID(request.GetID());
            resolver.SendClientResponse(response);
            continue;
        } catch(const Exception e) {
            flag = 0;
        }


        while(true) {
            //Create the server socket based on the configuration (starts at ROOT)
            try {
                resolver.CreateServerSocket();
                response = resolver.SendServerRequest(request);
            } catch(const Exception & e) {
                std::cerr << e.what() << std::endl;
                exit(-1);
            }

            // std::cout << "RESPONSE FROM SERVER:\n------------------------\n" << std::endl;
            // response.Print();

            //Answer received
            if(response.GetAnswerCount()) {
                bool lBreak = false;

                //Loop through answers looking for Type A (IPv4)
                for(int i = 0; i < response.GetAnswerCount(); i++) {
                    if(response.GetAnswerSection().at(i).GetType() == TYPE_A) {
                        // Replace the question section of the current response packet with the original question section
                        // that the client provided in its initial request.  This needs to be done because a new
                        // request packet is constructed when a CNAME record is encountered and therefore the question
                        // section is changed in the final response packet that we want to send back to the client.
                        std::vector<QuestionRecord> lQuestionRecords;
                        lQuestionRecords.push_back(lOriginalQuestionRecord);
                        response.setQuestionSection(lQuestionRecords);

                        //@TODO Add to cache
                        cache.AddPacket(response.GetDomain(), response);
                        //Forward packet to client
                        try {
                            resolver.SendClientResponse(response);
                        } catch(const Exception & e) {
                            std::cerr << e.what() << std::endl;
                        }

                        //Stop the recursion since an answer was obtained.
                        lBreak = true;
                        break;
                    }
                }

                if(lBreak) break;

                //Loop through answers looking for CNAMEs
                for(int i = 0; i < response.GetAnswerCount(); i++) {
                    if(response.GetAnswerSection().at(i).GetType() == TYPE_CNAME) {
                        //Update the request to use the true name (CNAME)
                        const char * lRecordData = response.GetAnswerSection().at(i).GetRecordData();
                        // std::string lCNameString(response.GetAnswerSection().at(i).GetRecordData(),
                        //                          response.GetAnswerSection().at(i).GetRecordDataLength());
                        // lCNameString.erase(lCNameString.size() - 1);
                        std::string lCNameString = Record::DecodeString(lRecordData, &lRecordData);
                        std::cout << "CNAME: " << lCNameString << std::endl;
                        request = DNSPacket(lCNameString, request.GetID());

                        //@TODO Add to cache
                        break;
                    }
                }
            }

            //No answer: send to the next server up
            //Send to a new server (name server/new root)
            try {
                //@TODO Add to cache
                resolver.UpdateServer(response);
            } catch(const Exception &e) {
                //Send response back to client
                std::cerr << e.what() << std::endl;

                try {
                    cache.Print();
                    resolver.SendClientResponse(response);
                } catch(const Exception & e) {
                    std::cerr << e.what() << std::endl;
                }

                break;
            }

        }
    }
}

void DNSResolver::Initialize(int argc, char ** argv) {
    rootServerIndex = 0;
    rootServers.clear();
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

    clientPort = configManager.getListeningPort();
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

    //DEBUG
    std::cout << "Client port: " << clientPort << std::endl;
    std::cout << "Client socket: " << clientSocket << std::endl;
}

DNSPacket DNSResolver::GetClientRequest(void) {
    unsigned int addressLength = sizeof(clientAddress);

    char data[MAX_DNS_LEN];
    int bytesReceived;

    while(true)
    {
        memset(&data, 0, MAX_DNS_LEN);
        bytesReceived = recvfrom(clientSocket, data, MAX_DNS_LEN, 0,
                                 (struct sockaddr *)&clientAddress,
                                 &addressLength
                                );
        if(bytesReceived < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            //The socket timed out from reading data since it is a non-blocking read attempt.
            //Another attempt has to be made.
            continue;
        }
        else if(bytesReceived < 0) {
            throw SocketException("Error receiving bytes from client: recvfrom()");
        }
        else
        {
            // DEBUG
            std::cout << "Received " << bytesReceived << " byte request from client." << std::endl;

            DNSPacket request(data, bytesReceived);

            // For some reason "dig" sends extra data in the Answer Section when it makes a DNS request.
            // Since if this extra data is sent as-is to the root DNS server a response is obtained saying
            // the server was unable to parse the packet, remove the answer section from the request packet.
            request.removeAnswerSection();

            return request;
        }
    }
}

void DNSResolver::CreateServerSocket(void) {

    //Create a socket for the server
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);

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
    std::cout << "Querying server IP: " << serverIP << std::endl;
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
        std::cout << strerror(errno) << std::endl;
        throw SocketException("Error receiving bytes from server: recvfrom()");
    }

    try
    {
        DNSPacket response(responseData, bytesReceived);
        return response;
    }
    catch(const Exception & e)
    {
        std::cerr << "Error parsing response from server: " << e.what() << std::endl;
        exit(-1);
    }
}

void DNSResolver::SendClientResponse(DNSPacket & response) {

    //DEBUG
    std::cout << "Sending client response...\n" << std::endl;

    // response.Print();

    char * responseData = response.GetData();

    int bytesSent = 0;

    bytesSent = sendto(clientSocket, responseData, response.Size(), 0, (struct sockaddr *)&clientAddress, sizeof(clientAddress));

    if(bytesSent < 0) {
        throw SocketException("Error sending bytes to client: sendto()");
    }
}

void DNSResolver::UpdateServer(DNSPacket & response) {

    //Check for name servers
    if(response.GetNameServerCount()) {
        std::string nameServer;
        std::string address;

        for(int i = 0; i < response.GetNameServerCount(); i++ ) {
            nameServer = std::string(response.GetNameServerSection().at(i).GetRecordData(),
                                     response.GetNameServerSection().at(i).GetRecordDataLength());
            nameServer.erase(nameServer.size() - 1);

            //Look up the IP of the name server in the additional records
            for(int j = 0; j < response.GetAdditionalRecordCount(); j++) {
                std::string lRawName = response.GetAdditionalSection().at(j).GetRawName();
                lRawName = response.GetAdditionalSection().at(j).EncodeString(lRawName);

                if(nameServer == lRawName) {
                    address = ExtendedRecord::getIPFromBytes(
                        response.GetAdditionalSection().at(j).GetRecordData(),
                        response.GetAdditionalSection().at(j).GetRecordDataLength()
                    );

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
        if((rootServerIndex + 1) >= rootServers.size()) {
            throw GeneralException("Root servers exhausted: DNS Lookup failed");
        }
        else
        {
            // Look at the next root server.
            rootServerIndex++;
        }

        serverIP = rootServers.at(rootServerIndex);
    }
}
