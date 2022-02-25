#include "potato.hpp"

int main(int argc, char *argv[]) {
    /*-------------------------Initialization---------------------------*/

    if (argc != 4) {
        cerr << "Error: incorrect number of the input arguments" << endl;
        return -1;
    }
    int num_players = stoi(argv[2]);
    int num_hops = stoi(argv[3]);
    if (num_players <= 1) {
        cerr << "Error: incorrect number of players" << endl;
        return -1;
    }
    if (num_hops < 0 || num_hops > 512) {
        cerr << "Error: incorrect number of hops" << endl;
        return -1;
    }
    const char *port = argv[1];
    //Start the server for the ring master
    int socket_fd = startServer(port, false);
    cout << "Potato Ringmaster" << endl;
    cout << "Players = " << num_players << endl;
    cout << "Hops = " << num_hops << endl;

    /*---------------------Check every player-----------------------------*/

    int client_connection_fd[num_players];
    int port_players[num_players];
    vector<string> ip_players(num_players);
    for (int i = 0; i < num_players; i++) {
        char ip_p[200];
        struct sockaddr_storage socket_addr;
        socklen_t socket_addr_len = sizeof(socket_addr);
        client_connection_fd[i] =
                accept(socket_fd, (struct sockaddr *) &socket_addr, &socket_addr_len);
        if (client_connection_fd[i] == -1) {
            cerr << "Error: cannot accept connection on socket" << endl;
            return -1;
        }
//        struct sockaddr_in *sockaddr_ip = (struct sockaddr_in *) &socket_addr;
//        //Get the ip address for every player
//        char str[INET_ADDRSTRLEN];
//        ip_players[i] = inet_ntop(AF_INET, &sockaddr_ip->sin_addr, str, sizeof(str));
//        cout << "Player " << i << " is ready to play" << endl;
//        cout << "IP of this player: "<<ip_players[i]<<endl;

        //Send the information of the game to every player
        send(client_connection_fd[i], &i, sizeof(i), 0);
        send(client_connection_fd[i], &num_players, sizeof(num_players), 0);
        send(client_connection_fd[i], &num_hops, sizeof(num_hops), 0);
        //Get the port number from the player
        recv(client_connection_fd[i], &port_players[i], sizeof(port_players[i]), 0);
        recv(client_connection_fd[i], &ip_p, sizeof(ip_p), 0);
        ip_players[i] = ip_p;
        cout << "Player " << i << " is ready to play" << endl;
        //cout << "IP of this player: "<<ip_p<<endl;
    }
    /*------------------Send the next player's information-------------------*/

    for (int i = 0; i < num_players; i++) {
        int id_next = (i + 1) % num_players;
        int port_next = port_players[id_next];
        char ip_next[200];
        strcpy(ip_next, ip_players[id_next].c_str());
        //string ip_next = ip_players[id_next].c_str();
        send(client_connection_fd[i], &port_next, sizeof(port_next), 0);
        send(client_connection_fd[i], &ip_next, sizeof(ip_next), 0);
        //send(client_connection_fd[i], &ip_players[id_next], ip_players[id_next].size(), 0);
    }
    /*-------------------------Send the potato-----------------------------*/
    if(num_hops == 0) {
        //Close all file descriptors
        for (auto fd: client_connection_fd) {
            close(fd);
        }
        close(socket_fd);
        return 0;
    }
    Potato hot_potato;
    hot_potato.num_hops = num_hops;
    srand((unsigned int) time(nullptr));
    int ran_id = rand() % num_players;
    send(client_connection_fd[ran_id], &hot_potato, sizeof(hot_potato), 0);
    cout << "Ready to start the game, sending potato to player " << ran_id << endl;

    /*--------------------Receive the final potato-------------------------*/

    Potato final_potato;
    fd_set fds;
    //int end_flag = 0;
    while (true) {
        FD_ZERO(&fds);
        int max_fd;
        for (int i = 0; i < num_players; i++) {
            FD_SET(client_connection_fd[i], &fds);
            //Get the maximum file descriptor, prepares for the select() function.
            max_fd = max(client_connection_fd[i], max_fd);
        }
        int readable = ::select(max_fd + 1, &fds, nullptr, nullptr, nullptr);
        switch (readable) {
            case -1:
                cerr << "Error: cannot select a readable file descriptor" << endl;
                return -1;
                break;
            case 0:
                break;
            default:
                for (int i = 0; i < num_players; i++) {
                    if (FD_ISSET(client_connection_fd[i], &fds)) {
                        recv(client_connection_fd[i], &final_potato, sizeof(final_potato), 0);
                        final_potato.end_flag = 1;
                        break;
                    }
                }
        }
        if (final_potato.end_flag)
            break;
    }
    //Send the final potato to every player to inform that the game is over
    for (int i = 0; i < num_players; i++) {
        send(client_connection_fd[i], &final_potato, sizeof(final_potato), 0);
    }
    //Print the trace of potato
    cout << "Trace of potato:" << endl;

    for (int i = 0; i < num_hops; i++) {
        if (i != num_hops - 1)
            cout << final_potato.trace[i] << ",";
        else
            cout << final_potato.trace[num_hops - 1] << endl;
    }
    //Close all file descriptors
    for (auto fd: client_connection_fd) {
        close(fd);
    }
    close(socket_fd);
    return 0;
}


