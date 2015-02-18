#include "DNSResolver.h"

int main(int argc, char * argv[]) {

    DNSResolver resolver;

    resolver.Initialize();

    //Initialization
    ConfigManager lConfigManager;
    try
    {
        lConfigManager.parseArgs(argc, argv);
    }
    catch(ParseException & e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return -2;
    }
    catch(FileIOException & e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return -3;
    }
    catch(...)
    {
        std::cerr << "Error: unhandled exception.\n";
        return -1;
    }

    std::cout << "Resolver info:" << std::endl;
    std::cout << " --> IP: " << lConfigManager.getResolverIPString() << std::endl;
    std::cout << " --> Port: " << lConfigManager.getResolverPort() << std::endl;


    //Create a socket:
    //socket() system call creates a socket, returning a socket descriptor
    //  AF_INET specifies the address family for internet
    //  SOCK_DGRAM says we want UDP as our transport layer
    //  0 is a parameter used for some options for certain types of sockets, unused for INET sockets
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    //Create timeval struct with 2s timeout.
    struct timeval to;
    to.tv_sec = 2;
    to.tv_usec = 0;

    //Make socket timeout after certain time with no data w/ setsockopt
    //  socket descriptor
    //  socket level (internet sockets, local sockets, etc.)
    //  option we want (SO_RCVTIMEO = Receive timeout)
    //  timeout structure
    //  size of structure
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));

    if(sockfd < 0) {
        std::cerr << "Could not open socket\n";
        return -1;
    }

    //inet_addr() converts a string-address into the proper type
    //Specify the address for the socket
    //Create the socket address structure and populate it's fields
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;                            //Specify the family again (AF_INET = internet family)
    serveraddr.sin_port = htons(lConfigManager.getResolverPort());                        //Specify the port on which to send data (16-bit) (# < 1024 is off-limits)
    serveraddr.sin_addr.s_addr = lConfigManager.getResolverIPInetAddr();        //Specify the IP address of the server with which to communicate

    fd_set sockets;

    // Clear the fd set
    FD_ZERO(&sockets);

    //Add server socket to the file descriptor set
    FD_SET(sockfd, &sockets);
    //FD_SET(STDIN_FILENO, &sockets);

    std::cout << std::endl;

    char * response = (char*)malloc(MAX_INPUT_SIZE);

    std::string domain;
    while(1) {
        memset(response, 0, MAX_INPUT_SIZE);
        domain.clear();
        //Get domain from user
        std::cout << "Enter a domain name: ";
        std::cin >> domain;

        resolver.SetDomain(domain);

        //Create a request DNS packets
        DNSPacket requestPacket(domain);

        std::cout << "REQUEST PACKET" << std::endl;
        requestPacket.Print();

        //Send packet to server
        char *packetData = requestPacket.GetData();
        if(-1 == sendto(sockfd, packetData, requestPacket.Size(), 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) {
            perror(strerror(errno));
            return 0;
        }

        unsigned int len = sizeof(serveraddr);
        int n = recvfrom(sockfd, response, MAX_INPUT_SIZE, 0, (struct sockaddr *)&serveraddr, &len);

        if(n <= 0) {
            std::cerr << "Error receiving packet: recvfrom(): " << strerror(errno) << std::endl;
        }
        else
        {
            //Create a packet for the response packet
            DNSPacket responsePacket(response, n);

            //Print response
            std::cout << "\nRESPONSE PACKET" << std::endl;
            responsePacket.Print();
        }
    }

    //Close
    free(response);
}

void DNSResolver::Initialize(void) {
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
}
