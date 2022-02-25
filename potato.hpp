#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Potato {
public:
    int end_flag;
    int num_hops;
    int trace[512]{};

    Potato() : num_hops(0), end_flag(0) { memset(trace, 0, sizeof(trace)); }
};

int startServer(const char *port, bool is_player) {
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    char *hostname = nullptr;
    //const char *hostname = "0.0.0.0";

    memset(&host_info, 0, sizeof(host_info));

    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags = AI_PASSIVE;
    //Get the host name of the machine.
//    if (is_player) {
//        char hostname_player[200];
//        if (gethostname(hostname_player, sizeof(hostname_player)) == -1) {
//            cerr << "Error: cannot get hostname" << endl;
//            exit(EXIT_FAILURE);
//        }
//        hostname = hostname_player;
//        cout<<"Host name: "<<hostname<<endl;
//    }

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        cerr << "Error: " << gai_strerror(status) << endl;
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    }

    socket_fd = socket(host_info_list->ai_family,
                       host_info_list->ai_socktype,
                       host_info_list->ai_protocol);
    if (socket_fd == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    }

    int yes = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("Error");
    }

    if (is_player) {
        //Assign the port if it is the player's server
        struct sockaddr_in *assign_addr = (struct sockaddr_in *) (host_info_list->ai_addr);
        assign_addr->sin_port = 0; //For the future bind
    }

    status = ::bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        //cerr << "Error: "<< strerror(status) << endl;
        perror("Error");
        cerr << "Error: cannot bind socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    }

    status = listen(socket_fd, 100);
    if (status == -1) {
        cerr << "Error: cannot listen on socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(host_info_list);
    return socket_fd;
}

void finishConnection(int &socket_fd, const char *hostname, const char *port) {
    int status;
    struct addrinfo host_info{};
    struct addrinfo *host_info_list;

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        //cerr << "Error: "<< strerror(status) << endl;
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    }
    socket_fd = socket(host_info_list->ai_family,
                       host_info_list->ai_socktype,
                       host_info_list->ai_protocol);
    if (socket_fd == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    }

    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        //cerr << "Error: "<< strerror(status) << endl;
        perror("Error");
        cerr << "Error: cannot connect to socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(host_info_list);
}
