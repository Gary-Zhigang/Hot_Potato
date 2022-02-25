#include "potato.hpp"
#include <climits>

int main(int argc, char *argv[]) {
    /*-------------------------Initialization---------------------------*/

    if (argc != 3) {
        cerr << "Error: incorrect number of the input arguments" << endl;
        return -1;
    }
    int master_fd;
    const char *hostname = argv[1];
    const char *port = argv[2];

    finishConnection(master_fd, hostname, port);
    //Start the server for the player
    int player_fd = startServer("", true);
    struct sockaddr_in sockaddr_port;
    socklen_t len = sizeof(sockaddr);
    if (getsockname(player_fd, (struct sockaddr *) &sockaddr_port, &len) == -1) {
        cerr << "Error: cannot get the socket name" << endl;
        exit(EXIT_FAILURE);
    }
    //Set the port of the player
    int port_players = ntohs(sockaddr_port.sin_port);
    /*---------------------Get the IP address of players---------------------*/

    char hostname_player[200];
    char ip_players[200];
    if (gethostname(hostname_player, sizeof(hostname_player)) == -1) {
            cerr << "Error: cannot get hostname" << endl;
            exit(EXIT_FAILURE);
    }
    struct hostent *hp;
    if ((hp=gethostbyname(hostname_player)) == NULL){
        exit(EXIT_FAILURE);
    }
    int i = 0;
    while(hp->h_addr_list[i] != NULL)
    {
        string temp;
        temp = inet_ntoa(*(struct in_addr*)hp->h_addr_list[i]);
        strcpy(ip_players, temp.c_str());
        //printf("hostname: %s\n",hp->h_name);
        //printf("      ip: %s\n",ip_players);
        i++;
    }


    /*-------------------Receive the information of players------------------*/

    int id;
    int num_players;
    int total_hops;
    recv(master_fd, &id, sizeof(id), 0);
    recv(master_fd, &num_players, sizeof(num_players), 0);
    recv(master_fd, &total_hops, sizeof(total_hops), 0);
    send(master_fd, &port_players, sizeof(port_players), 0);
    send(master_fd, &ip_players, sizeof(ip_players), 0);
    cout << "Connected as player " << id << " out of " << num_players << " total players"
         << endl;

    /*--------------Receive the IP and port of the next player--------------*/

    int port_next;
    char ip_next[200];
    recv(master_fd, &port_next, sizeof(port_next), 0);
    recv(master_fd, &ip_next, sizeof(ip_next), 0);
    const char *port_next_str = to_string(port_next).c_str();
    const char *ip_next_str(ip_next);


    /*----------Connect the next player and accept the last player----------*/

    int next_player_fd;
    //cout<<"Port: "<<port_players<<endl;
    //cout<<"Ip address of the next player: "<<ip_next_str<<endl;
    //cout<<"Port number of the next player: "<<port_next_str<<endl;
    finishConnection(next_player_fd, ip_next_str, port_next_str);
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int last_player_fd = accept(player_fd, (struct sockaddr *) &socket_addr, &socket_addr_len);
    if (last_player_fd == -1) {
        cerr << "Error: cannot accept connection on socket" << endl;
        return -1;
    }

    /*----------Start playing the potato until the num_hops is 0-----------*/

    if(total_hops == 0) {
        close(master_fd);
        close(next_player_fd);
        close(last_player_fd);
        return 0;
    }
    Potato potato;
    int set_fds[3] = {master_fd, next_player_fd, last_player_fd};
    fd_set fds;
    srand((unsigned int) time(nullptr));
    while (true) {
        FD_ZERO(&fds);
        int max_fd = INT_MIN;
        for (int i = 0; i < 3; i++) {
            FD_SET(set_fds[i], &fds);
            //Get the maximum file descriptor, prepares for the select() function.
            max_fd = max(set_fds[i], max_fd);
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
                for (int i = 0; i < 3; i++) {
                    if (FD_ISSET(set_fds[i], &fds)) {
                        recv(set_fds[i], &potato, sizeof(potato), 0);
                        break;
                    }
                }
        }
        //If the potato is the final potato, end the game.
        if (potato.end_flag && potato.num_hops == 0)
            break;
        int n = total_hops - potato.num_hops;
        //Record the player's id into the trace arrays
        potato.trace[n] = id;
        potato.num_hops--;
        if (potato.num_hops == 0) {
            //Send the final potato to the master.
            send(master_fd, &potato, sizeof(potato), 0);
            cout << "I'm it" << endl;
            continue;
        }
        //Randomly send the potato to last or next player
        int r_neighbour = rand() % 2;
        if (r_neighbour == 1) {
            send(next_player_fd, &potato, sizeof(potato), 0);
            cout << "Sending potato to " << (id + 1 + num_players) % num_players << endl;
        } else {
            send(last_player_fd, &potato, sizeof(potato), 0);
            cout << "Sending potato to " << (id - 1 + num_players) % num_players << endl;
        }
    }

    //close all file descriptors
    close(master_fd);
    close(next_player_fd);
    close(last_player_fd);
    return 0;
}
