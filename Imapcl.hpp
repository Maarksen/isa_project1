/*
author: Marek Buch
login: xbuchm02
*/

#ifndef IMAPCL_HPP
#define IMAPCL_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>

class Imapcl {
    public:
        static int run(std::string server, int port, std::string certfile, std::string certaddr,
                    bool encryption, bool only_new, bool only_header, std::string auth_file, std::string MAILBOX,
                    std::string out_dir);

        static bool get_credentials(std::string file_name);
        
        static int connect_to_server(std::string server, int port);
        static void authenticate(int sockfd, std::string auth_file);

    private:
        static std::string username;
        static std::string password;

};

#endif