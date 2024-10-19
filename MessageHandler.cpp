#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

#include "MessageHandler.hpp"

void MH::read_response(int sockfd) {
    char buffer[1024];
    int n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        printf("Server response: %s\n", buffer);
    }
}

void MH::select_mailbox(int sockfd, const std::string mailbox) {
    std::string select_cmd = "A002 SELECT " + mailbox + "\r\n";
    
    write(sockfd, select_cmd.c_str(), select_cmd.length());
    read_response(sockfd);
}

void MH::create_output_dir(std::string out_dir) {
    struct stat info;

    if (stat(out_dir.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
        std::cout << "[INFO] Creating output directory: " << out_dir << std::endl;
        if (mkdir(out_dir.c_str(), 0777) == -1) {
            perror("[ERROR] Unable to create directory");
            exit(1);
        }
    }
}

void MH::fetch_messages(int sockfd, std::string out_dir) {
    std::string fetch_cmd = "A003 FETCH 1:* (BODY[])\r\n";
    
    write(sockfd, fetch_cmd.c_str(), fetch_cmd.length());

    save_messages(sockfd, out_dir);
}

void MH::save_messages(int sockfd, std::string out_dir) {
    create_output_dir(out_dir);

    char buffer[4096];
    int n;
    int msg_count = 1;

    while ((n = read(sockfd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';

        if (std::string(buffer).find("FETCH") != std::string::npos) {
            std::string filename = out_dir + "/message_" + std::to_string(msg_count++) + ".eml";
            
            std::ofstream outfile(filename);
            if (outfile.is_open()) {
                outfile << buffer;
                outfile.close();
                std::cout << "[INFO] Saved message to " << filename << std::endl;
            } else {
                std::cerr << "[ERROR] Unable to open file: " << filename << std::endl;
            }
        }
        if (std::string(buffer).find("A003 OK FETCH completed") != std::string::npos) {
            break;
        }
    }

    if (n < 0) {
        perror("Error reading server response");
    }
}

// void MH::print_fetched_messages(int sockfd) {
//     char buffer[4096];
//     int n;

//     while ((n = read(sockfd, buffer, sizeof(buffer) - 1)) > 0) {
//         buffer[n] = '\0';
//         std::cout << buffer;

//         if (std::string(buffer).find("A003 OK FETCH completed") != std::string::npos) {
//             break;
//         }
//     }

//     if (n < 0) {
//         perror("Error reading server response");
//     }
// }
