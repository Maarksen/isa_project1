/*
author: Marek Buch
login: xbuchm02
*/

#include "Imapcl.hpp"
#include "MessageHandler.hpp"
#include "Encryption.hpp"

std::string Imapcl::username;
std::string Imapcl::password;

using namespace std;

int Imapcl::run(std::string server, int port, std::string certfile, std::string certaddr,
                bool encryption, bool only_new, bool only_header, std::string auth_file,
                std::string MAILBOX, std::string out_dir) {

    int sockfd;
    SSL *ssl = nullptr;
    if (encryption) {
        ssl = Encrypt::ssl_connect_to_server(certaddr, certfile, server, port);
        get_credentials(auth_file);
        Encrypt::ssl_authenticate(ssl, username, password);
    }
    else{
        sockfd = connect_to_server(server, port);
        authenticate(sockfd, auth_file);
    }

    MH::select_mailbox(sockfd, ssl, MAILBOX, encryption);

    if (only_new) {
        MH::fetch_new_messages(sockfd, ssl, out_dir, only_header, server, MAILBOX, encryption);
    } else {
        MH::fetch_messages(sockfd, ssl, out_dir, only_header, server, MAILBOX, encryption);
    }

    return 0;
}

// int Imapcl::interactive(std::string server, int port, std::string certfile, std::string certaddr,
//                 bool encryption, bool only_new, bool only_header, std::string auth_file,
//                 std::string MAILBOX, std::string out_dir) {
//     std::cout << "[INFO] Interactive mode started. Type 'EXIT' to quit." << std::endl;

//     std::string command;

//     int sockfd;
//     SSL *ssl = nullptr;
//     if (encryption) {
//         ssl = Encrypt::ssl_connect_to_server(certaddr, certfile, server, port);
//         get_credentials(auth_file);
//         Encrypt::ssl_authenticate(ssl, username, password);
//     }
//     else{
//         sockfd = connect_to_server(server, port);
//         authenticate(sockfd, auth_file);
//     }

//     MH::select_mailbox(sockfd, ssl, MAILBOX, encryption);

//     while (true) {
//         std::getline(std::cin, command);

//         std::istringstream iss(command);
//         std::string main_command, argument;
//         iss >> main_command >> argument;

//         std::cout << "[INFO] Command: " << main_command << " Argument: " << argument << std::endl;

//         if (main_command == "DOWNLOADNEW") {
//             std::cout << "[INFO] Downloading new messages." << std::endl;
//             if(!argument.empty()){
//                 std::cout << "[INFO] Downloading new messages from mailbox " << argument << "." << std::endl;
//                 MH::select_mailbox(sockfd, ssl, argument, encryption);
//                 MH::fetch_new_messages(sockfd, ssl, out_dir, only_header, server, argument, encryption);
//             }
//             else{
//                 std::cout << "[INFO] Downloading new messages from mailbox " << MAILBOX << "." << std::endl;
//                 MH::fetch_new_messages(sockfd, ssl, out_dir, only_header, server, MAILBOX, encryption);
//             }

//         }
//         else if (main_command == "DOWNLOADALL") {
//             std::cout << "[INFO] Downloading all messages." << std::endl;
//             if(!argument.empty()){
//                 std::cout << "[INFO] Downloading all messages from mailbox " << argument << "." << std::endl;
//                 MH::select_mailbox(sockfd, ssl, argument, encryption);
//                 MH::fetch_messages(sockfd, ssl, out_dir, only_header, server, argument, encryption);
//             }
//             else{
//                 std::cout << "[INFO] Downloading all messages from mailbox " << MAILBOX << "." << std::endl;
//                 MH::fetch_messages(sockfd, ssl, out_dir, only_header, server, MAILBOX, encryption);
//             }
//         }
//         else if (main_command == "QUIT") {
//             break;
//         } 
//         else {
//             std::cout << "[INFO] Unknown command: " << command << std::endl;
//         }
//     }

//     return 0;
// }

//function to get the credentials from the auth_file
bool Imapcl::get_credentials(std::string file_name) {
    FILE* file = fopen(file_name.c_str(), "r");
    if (file == NULL) {
        std::cerr << "[ERROR] Cannot open file." << std::endl;
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

    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *host;

    host = gethostbyname(server.c_str());
    if (host == NULL) {
        std::cerr << "[ERROR] Failed to resolve hostname." << std::endl;
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "[ERROR] Failed to create socket." << std::endl;
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "[ERROR] Failed to connect." << std::endl;
        close(sockfd);
        return -1;
    }
    MH::read_response(sockfd);

    return sockfd;
}

//function to authenticate the user
void Imapcl::authenticate(int sockfd, std::string auth_file) {
    
    if(get_credentials(auth_file) == false) return;

    std::string login_command = "a001 LOGIN \"" + username + "\" \"" + password + "\"\r\n";
    
    write(sockfd, login_command.c_str(), login_command.length());
    MH::read_response(sockfd);
}
