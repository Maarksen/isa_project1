#ifndef MESSAGEHANDLER_HPP
#define MESSAGEHANDLER_HPP

#include <iostream>
#include <unistd.h>
#include <string.h>
#include <string>

class MH {
    public:
        static void read_response(int sockfd);
        static void select_mailbox(int sockfd, const std::string mailbox);

        static void create_output_dir(std::string out_dir);
        static void fetch_messages(int sockfd, std::string out_dir, bool only_header, std::string mailbox);
        static void fetch_new_messages(int sockfd, std::string out_dir, bool only_header, std::string mailbox);
        
        static void parse_fetch_response(int sockfd, std::string out_dir, bool only_header, bool only_new, std::string mailbox);
        static std::string parse_search_response(int sockfd);

        static void save_message_to_file(std::string filena, std::string message);
        
    private:

};

#endif