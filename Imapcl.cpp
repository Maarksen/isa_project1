#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

#include "Imapcl.hpp"
#include "MessageHandler.hpp"

std::string Imapcl::username;
std::string Imapcl::password;

using namespace std;

int Imapcl::run(std::string server, int port, std::string certfile, std::string certaddr,
                bool encryption, bool only_new, bool only_header, std::string auth_file,
                std::string MAILBOX, std::string out_dir) {

    int sockfd = connect_to_server(server, port);
    if (sockfd < 0) {
        return -1;
    }
    authenticate(sockfd, auth_file);
    MH::select_mailbox(sockfd, MAILBOX);

    if (only_new) {
        MH::fetch_new_messages(sockfd, out_dir, only_header, MAILBOX);
    } else {
        MH::fetch_messages(sockfd, out_dir, only_header, MAILBOX);
    }

    return 1;
}

//function to get the credentials from the auth_file
bool Imapcl::get_credentials(std::string file_name) {
    FILE* file = fopen(file_name.c_str(), "r");
    if (file == NULL) {
        perror("[ERROR] Cannot open file");
        return false;
    }

    char line[512];
    char user[256] = {0};
    char pass[256] = {0};

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "username =", 10) == 0) {
            sscanf(line, "username = %255s", user);
        } else if (strncmp(line, "password =", 10) == 0) {
            sscanf(line, "password = %255s", pass);
        }
    }

    fclose(file);

    if (strlen(user) == 0 || strlen(pass) == 0) {
        std::cerr << "[ERROR] Missing username or password in credentials file." << std::endl;
        return false;
    }

    username = std::string(user);
    password = std::string(pass);
    return true;
}

//function to connect to the server
int Imapcl::connect_to_server(std::string server, int port) {
    cout << "[CONNECTING]" << endl;

    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *host;

    host = gethostbyname(server.c_str());
    if (host == NULL) {
        perror("[ERROR] Failed to resolve hostname.");
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("[ERROR] Failed to create socket.");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[ERROR] Failed to connect.");
        close(sockfd);
        return -1;
    }
    cout << "Connected to " << server << " on port " << port << endl;
    MH::read_response(sockfd);

    return sockfd;
}

//function to authenticate the user
void Imapcl::authenticate(int sockfd, std::string auth_file) {
    cout << "[AUTHENTICATING]" << endl;
    
    if(get_credentials(auth_file) == false) return;

    std::string login_command = "a001 LOGIN \"" + username + "\" \"" + password + "\"\r\n";
    cout << "Sending: " << login_command << endl;
    
    write(sockfd, login_command.c_str(), login_command.length());
    MH::read_response(sockfd);
}