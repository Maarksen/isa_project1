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
        static void fetch_messages(int sockfd, std::string out_dir, bool only_header);
        static void parse_response(int sockfd, std::string out_dir, bool only_header);
        static int get_message_count(int sockfd);
        static void save_message_to_file(std::string filena, std::string message);
        //static void print_fetched_messages(int sockfd);
        
    private:

};

#endif