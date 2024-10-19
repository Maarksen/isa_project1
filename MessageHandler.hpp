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
        static void fetch_messages(int sockfd, std::string out_dir);
        static void save_messages(int sockfd, std::string out_dir);
        //static void print_fetched_messages(int sockfd);
        
    private:

};

#endif