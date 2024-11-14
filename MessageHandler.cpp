#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
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
        std::cout << "Creating output directory: " << out_dir << std::endl;
        if (mkdir(out_dir.c_str(), 0777) == -1) {
            perror("[ERROR] Unable to create directory");
            exit(1);
        }
    }
}

void MH::fetch_messages(int sockfd, std::string out_dir, bool only_header) {
    std::string fetch_command;

    if (only_header) {
        fetch_command = "A003 FETCH 1:* (BODY.PEEK[HEADER])\r\n";
    }
    else {
        fetch_command = "A003 FETCH 1:* (BODY[])\r\n";
    }

    write(sockfd, fetch_command.c_str(), fetch_command.length());
    parse_response(sockfd, out_dir, only_header);
}

//function to trim whitespaces from the beginning and end of a string
std::string trim(const std::string &str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    size_t last = str.find_last_not_of(" \t\r\n");
    return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
}

void MH::parse_response(int sockfd, std::string out_dir, bool only_header) {
    create_output_dir(out_dir);

    char buffer[4096];
    int n;
    int msg_count = 1;
    std::string current_message;
    int expected_length = 0;
    bool reading_message = false;

    while ((n = read(sockfd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        std::istringstream response_stream(buffer);
        std::string line;

        std::cout << "Server response:\n" << buffer << std::endl;

        while (std::getline(response_stream, line)) {
            if (!reading_message) {
                size_t fetch_pos = line.find("FETCH");
                size_t length_start = line.find("{");
                size_t length_end = line.find("}");

                if (fetch_pos != std::string::npos && length_start != std::string::npos && length_end != std::string::npos) {
                    std::string length_str = line.substr(length_start + 1, length_end - length_start - 1);
                    expected_length = std::stoi(length_str);

                    current_message = "";
                    reading_message = true;
                    continue;
                }

                if (line.find("A003 OK FETCH completed") != std::string::npos) {
                    std::cout << "All messages fetched" << std::endl;
                    return;
                }
            } 
            else {
                current_message += line + "\n";

                if(only_header) {
                    if(trim(line).empty()){
                        std::string filename = out_dir + "/header_message_" + std::to_string(msg_count++) + ".eml";
                        save_message_to_file(filename, current_message);

                        reading_message = false;
                        expected_length = 0;
                        current_message = "";
                        continue;
                    }

                }
                else if (current_message.length() >= expected_length) {
                    std::string filename = out_dir + "/message_" + std::to_string(msg_count++) + ".eml";
                    save_message_to_file(filename, current_message);

                    reading_message = false;
                    expected_length = 0;
                    current_message = "";
                }
            }
        }
    }

    if (n < 0) {
        perror("Error reading server response");
    }
}

void MH::save_message_to_file(std::string filename, std::string message) {
    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        outfile << message;
        outfile.close();
        std::cout << "Saved message to " << filename << std::endl;
    } else {
        std::cerr << "[ERROR] Unable to open file: " << filename << std::endl;
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
