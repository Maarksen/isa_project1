/*
author: Marek Buch
login: xbuchm02
*/

#include "Encryption.hpp"
#include "MessageHandler.hpp"

//function to read the server response
void Encrypt::read_encrypted_response(SSL *ssl) {
    char buffer[1024];
    int n = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
    }
    std::string buffer_str = buffer;
    if(buffer_str.find(" BAD ") != std::string::npos || buffer_str.find(" NO ") != std::string::npos) {
        std::cerr << "[ERROR] Server response: " << buffer_str << std::endl;
        Encrypt::ssl_logout(ssl);
        exit(1);
    }
}

//function to initialize the SSL and verify certificates
SSL_CTX *Encrypt::initialize_openssl(std::string certaddr, std::string certfile) {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    //create SSL context
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        std::cerr << "[ERROR] Failed to create SSL context" << std::endl;
        ERR_print_errors_fp(stderr);
        exit(1);
    }

    //verify certificates
    if(certfile == ""){
        if(!SSL_CTX_load_verify_locations(ctx, NULL, certaddr.c_str())) {
            std::cerr << "[ERROR] Failed to load verify locations." << std::endl;
            ERR_print_errors_fp(stderr);
            SSL_CTX_free(ctx);
            exit(1);
        }
    }
    else {
        if(!SSL_CTX_load_verify_locations(ctx, certfile.c_str(), certaddr.c_str())) {
            std::cerr << "[ERROR] Failed to load verify locations." << std::endl;
            ERR_print_errors_fp(stderr);
            SSL_CTX_free(ctx);
            exit(1);
        }
    }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    return ctx;
}

//function to establish an SSL connection
SSL *Encrypt::ssl_connect_to_server(std::string certaddr, std::string certfile, std::string server, int port) {
    
    struct hostent *host;
    struct sockaddr_in server_addr;
    int sockfd;
    SSL *ssl = nullptr;

    SSL_CTX *ctx = Encrypt::initialize_openssl(certaddr, certfile);

    host = gethostbyname(server.c_str());
    if (host == NULL) {
        std::cerr << "[ERROR] Failed to resolve hostname." << std::endl;
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "[ERROR] Failed to create socket." << std::endl;
        cleanup(ssl, sockfd, ctx);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "[ERROR] Failed to connect." << std::endl;
        cleanup(ssl, sockfd, ctx);
    }

    ssl = SSL_new(ctx);
    if (!ssl) {
        std::cerr << "[ERROR] Failed to create SSL object" << std::endl;
        ERR_print_errors_fp(stderr);
        cleanup(ssl, sockfd, ctx);;
    }

    //connecting the ssl object with the socket
    if (SSL_set_fd(ssl, sockfd) == 0) {
        std::cerr << "[ERROR] Failed to connect SSL with socket." << std::endl;
        ERR_print_errors_fp(stderr);
        cleanup(ssl, sockfd, ctx);
    }

    //ssl handshake
    if (SSL_connect(ssl) <= 0) {
        std::cerr << "[ERROR] SSL handshake failed." << std::endl;
        ERR_print_errors_fp(stderr);
        cleanup(ssl, sockfd, ctx);
    }

    Encrypt::read_encrypted_response(ssl);

    return ssl;
}

//function to authenticate the user
void Encrypt::ssl_authenticate(SSL *ssl, std::string username, std::string password) {

    std::string login_command = "a001 LOGIN \"" + username + "\" \"" + password + "\"\r\n";
    
    SSL_write(ssl, login_command.c_str(), login_command.length());
    Encrypt::read_encrypted_response(ssl);
}

//function to cleanup the SSL connection after an error
void Encrypt::cleanup(SSL *ssl, int sockfd, SSL_CTX *ctx) {
    if (ssl) {
        SSL_free(ssl);
    }
    if (sockfd >= 0) {
        close(sockfd);
    }
    if (ctx) {
        SSL_CTX_free(ctx); 
    }
    EVP_cleanup();
    exit(1);
}

//function to logout from the server
void Encrypt::ssl_logout(SSL *ssl) {
    std::string logout_command = "a002 LOGOUT\r\n";
    SSL_write(ssl, logout_command.c_str(), logout_command.length());
    Encrypt::read_encrypted_response(ssl);
}