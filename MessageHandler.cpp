/*
author: Marek Buch
login: xbuchm02
*/

#include "MessageHandler.hpp"
#include "Encryption.hpp"

//function to read the server response
void MH::read_response(int sockfd) {
    char buffer[1024];
    int n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
    }
    std::string buffer_str = buffer;
    if(buffer_str.find(" BAD ") != std::string::npos || buffer_str.find(" NO ") != std::string::npos) {
        std::cerr << "[ERROR] Server response: " << buffer_str << std::endl;
        exit(1);
    }
}

//function to select the mailbox
void MH::select_mailbox(int sockfd, SSL *ssl, const std::string mailbox, bool encryption) {
    std::string select_cmd = "A002 SELECT " + mailbox + "\r\n";
    
    if (encryption) {
        std::cout << "toto je command " << select_cmd << std::endl;
        SSL_write(ssl, select_cmd.c_str(), select_cmd.length());
        Encrypt::read_encrypted_response(ssl);
    }
    else {
        write(sockfd, select_cmd.c_str(), select_cmd.length());
        read_response(sockfd);
    }
}

//function to create the output directory
void MH::create_output_dir(std::string out_dir) {
    if(!std::filesystem::exists(out_dir)) {
        std::filesystem::create_directory(out_dir);
        std::cout << "Created output directory: " << out_dir << std::endl;
    }
    std::cout << "Directory already exists." << std::endl;
}


//function to fetch all messages
void MH::fetch_messages(int sockfd, SSL *ssl, std::string out_dir, bool only_header, std::string server,
                        std::string mailbox, bool encryption) {
    std::string fetch_command;

    if (only_header) {
        fetch_command = "A003 FETCH 1:* (BODY.PEEK[HEADER])\r\n";
    }
    else {
        fetch_command = "A003 FETCH 1:* (BODY[])\r\n";
    }

    if (encryption) {
        SSL_write(ssl, fetch_command.c_str(), fetch_command.length());
        //Encrypt::read_encrypted_response(ssl);
    }
    else {
        write(sockfd, fetch_command.c_str(), fetch_command.length());
        //MH::read_response(sockfd);
    }

    std::cout << "fetchol som spravy, volam parse" << std::endl;
    parse_fetch_response(sockfd, ssl, out_dir, only_header, false, encryption, server, mailbox);
}

//function to fetch only new messages
void MH::fetch_new_messages(int sockfd, SSL *ssl, std::string out_dir, bool only_header, std::string server,
                            std::string mailbox, bool encryption) {
    //looking for only previously unseen messages
    std::string search_command = "A003 SEARCH UNSEEN\r\n";
    std::string fetch_command;

    if (encryption) {
        SSL_write(ssl, search_command.c_str(), search_command.length());
        //Encrypt::read_encrypted_response(ssl);
    }
    else {
        write(sockfd, search_command.c_str(), search_command.length());
        //MH::read_response(sockfd);
    }

    //get the uids of the unseen messages
    std::string unseen_uids = parse_search_response(sockfd, ssl, encryption);
    std::cout << "Unseen UIDs: " << unseen_uids << std::endl;

    //no new messages available
    if (unseen_uids.empty()) {
        std::cout << "[INFO] No new messages available in mailbox " << mailbox << "." << std::endl;
        return;
    }

    if (only_header) {
        fetch_command = "A003 FETCH " + unseen_uids + " (BODY.PEEK[HEADER])\r\n";
    }
    else {
        fetch_command = "A003 FETCH " + unseen_uids + " (BODY[])\r\n";
    }

    write(sockfd, fetch_command.c_str(), fetch_command.length());

    parse_fetch_response(sockfd, ssl, out_dir, only_header, true, encryption, server, mailbox);
}

//helper function to trim whitespaces from the beginning and end of a string
std::string trim(const std::string &str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    size_t last = str.find_last_not_of(" \t\r\n");

    if(first == std::string::npos || last == std::string::npos){
        return "";
    }
    else{
        return str.substr(first, last - first + 1);
    }
}

//function to parse the response from the FETCH command
void MH::parse_fetch_response(int sockfd, SSL *ssl, std::string out_dir, bool only_header, bool only_new,
                              bool encryption, std::string server, std::string mailbox) {

    create_output_dir(out_dir);

    char buffer[4096];
    int n;
    int msg_count = 0;
    std::string current_message;
    unsigned int expected_length = 0;
    bool parsing_message = false;

    while (true) {
        std::cout << "parsing fetch" << std::endl;
        if (encryption) {
            n = SSL_read(ssl, buffer, sizeof(buffer) - 1);
        }
        else {
            n = read(sockfd, buffer, sizeof(buffer) -1);
        }
        buffer[n] = '\0';
        std::istringstream response(buffer);
        std::string line;

        std::cout << "Server response:\n" << buffer << std::endl;

        while (std::getline(response, line)) {
            if (!parsing_message) {
                size_t keyword_fetch = line.find("FETCH");
                size_t start_len = line.find("{");
                size_t end_len = line.find("}");

                //if we found the FETCH substring which comes at the beggining of each fetched message
                //start parsing the message body
                if (keyword_fetch != std::string::npos && start_len != std::string::npos && end_len != std::string::npos) {
                    std::string length_str = line.substr(start_len + 1, end_len - start_len - 1);
                    expected_length = std::stoi(length_str);
                    std::cout << "spadol som do ifu" << std::endl;

                    current_message = "";
                    parsing_message = true;
                    continue;
                }
                
                //when Fetch completed is detected outside of the message body, print the number of messages fetched
                if (line.find("A003 OK Fetch completed") != std::string::npos) {
                    if (only_header) {
                        std::cout << "[INFO] " << msg_count << " header messages fetched from mailbox " << mailbox << "." << std::endl;
                    }
                    else if (only_new) {
                        std::cout << "[INFO] " << msg_count << " new messages fetched from mailbox " << mailbox << "." << std::endl;
                    }
                    else {
                        std::cout << "[INFO] " << msg_count << " messages fetched from mailbox " << mailbox << "." << std::endl;
                    }
                    return;
                }

                //if the server response contains NO or BAD, print error message and exit
                if (line.find("NO") != std::string::npos || line.find("BAD") != std::string::npos){
                    std::cerr << "[ERROR] Server response: " << line << std::endl;
                    exit(1);
                }
            } 
            else {
                current_message += line + "\n";

                if(only_header) {
                    //if we are only fetching headers and the line is empty, which means the header part is over,
                    //or if we reached the expected length of the message save the message to a file
                    if(trim(line).empty() || current_message.length() >= expected_length) {
                        std::string filename;
                        if(only_new) {
                            filename = out_dir + "/new_h_" + server + "_" + mailbox + "_" + std::to_string(++msg_count) + ".eml";
                        }
                        else {
                            filename = out_dir + "/h_" + server + "_" + mailbox + "_" + std::to_string(++msg_count) + ".eml";
                        }
                        save_message_to_file(filename, current_message);

                        parsing_message = false;
                        expected_length = 0;
                        current_message = "";
                        continue;
                    }

                }
                //if we reached the expected length of the message, save it to a file
                else if (current_message.length() >= expected_length) {
                    std::string filename;
                    if (only_new) {
                        filename = out_dir + "/new_m_" + server + "_" + mailbox + "_" + std::to_string(++msg_count) + ".eml";
                    }
                    else {
                        filename = out_dir + "/m_" + server + "_" + mailbox + "_" + std::to_string(++msg_count) + ".eml";
                    }

                    save_message_to_file(filename, current_message);

                    parsing_message = false;
                    expected_length = 0;
                    current_message = "";
                }
            }
        }
    }

    if (n < 0) {
        std::cerr << "[ERROR] Error reading server response." << std::endl;
    }
}

//function to parse the response from the SEARCH command and return the uids
std::string MH::parse_search_response(int sockfd, SSL *ssl, bool encryption) {
    int n;
    char buffer[1024];
    std::string uids;
    std::string keyword_search = "SEARCH";

    while (true) {

        std::cout << "parsing search" << std::endl;
        if (encryption) {
            std::cout << "mam ssl" << std::endl;
            n = SSL_read(ssl, buffer, sizeof(buffer) - 1);
        }
        else {
            n = read(sockfd, buffer, sizeof(buffer) -1);
        }
        std::cout << "mam nko" << std::endl;

        buffer[n] = '\0';
        std::istringstream response(buffer);
        std::string line;

        while (std::getline(response, line)) {

            //if the server response contains NO or BAD, print error message and exit
            if (line.find("NO") != std::string::npos || line.find("BAD") != std::string::npos){
                std::cerr << "[ERROR] Server response: " << line << std::endl;
                exit(1);
            }

            size_t search_uids = line.find(keyword_search);

            if (search_uids != std::string::npos) {
                //take the substring after the word SEARCH, which contains the uids
                std::istringstream uid_stream(line.substr(search_uids + 7));
                std::string uid;

                while (uid_stream >> uid) {
                    uids += uid + ",";
                }

                return uids;
            }
        }
    }

    if (n < 0) {
        std::cerr << "[ERROR] Error reading server response" << std::endl;
    }
    return "";
}

//function to save the message to a file
void MH::save_message_to_file(std::string filename, std::string message) {
    if (std::filesystem::exists(filename)) {
        std::filesystem::remove(filename);
        std::cout << "Deleted existing file: " << filename << std::endl;
    }

    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        outfile << message;
        outfile.close();
        std::cout << "Saved message to " << filename << std::endl;
    } 
    else {
        std::cerr << "[ERROR] Unable to open file: " << filename << "." << std::endl;
    }
}