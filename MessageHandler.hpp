/*
author: Marek Buch
login: xbuchm02
*/

#ifndef MESSAGEHANDLER_HPP
#define MESSAGEHANDLER_HPP

#include <iostream>
#include <unistd.h>
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <filesystem>
#include <sstream>
#include <openssl/ssl.h>

class MH {
    public:
        static void read_response(int sockfd);
        static void select_mailbox(int sockfd, SSL *ssl, const std::string mailbox, bool encryption);

        static void create_output_dir(std::string out_dir);
        static void fetch_messages(int sockfd, SSL *ssl, std::string out_dir, bool only_header, std::string server,
                                   std::string mailbox, bool encryption);
        static void fetch_new_messages(int sockfd, SSL *ssl, std::string out_dir, bool only_header, std::string server,
                                       std::string mailbox, bool encryption);
        
        static void parse_fetch_response(int sockfd,SSL *ssl, std::string out_dir, bool only_header,
                                         bool only_new, bool encryption, std::string server, std::string mailbox);
        static std::string parse_search_response(int sockfd, SSL *ssl, bool encryption);

        static bool save_message_to_file(std::string filena, std::string message);
        static void logout(int sockfd);
        
    private:

};

#endif