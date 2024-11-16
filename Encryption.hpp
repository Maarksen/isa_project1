/*
author: Marek Buch
login: xbuchm02
*/

#ifndef ENCRYPTION_HPP
#define ENCRYPTION_HPP

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

#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

class Encrypt {
    public:
        static void read_encrypted_response(SSL *ssl);

        static SSL_CTX *initialize_openssl(std::string certaddr);
        static SSL *ssl_connect_to_server(std::string certaddr, std::string hostname, int port);

        static void cleanup(SSL *ssl, int sockfd, SSL_CTX *ctx);

    private:

};

#endif