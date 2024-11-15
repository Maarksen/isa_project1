#include <iostream>
#include <string>
#include <vector>
#include <getopt.h>

#include "Imapcl.hpp"

using namespace std;

void printUsage() {
    std::cout << "Usage: imapcl server [-p port] [-T [-c certfile] [-C certaddr]] [-n] [-h] -a auth_file [-b MAILBOX] -o out_dir\n"
              << "\nParameters:\n"
              << "  server       [Required] The IP address or domain name of the desired source.\n"
              << "  -p port      [Optional] Specifies the port number on the server. Default depends on whether -T is specified.\n"
              << "  -T           [Optional] Enables encryption (IMAPS). If not specified, the unencrypted version of the protocol is used.\n"
              << "  -c certfile  [Optional] File with certificates used to validate the server's SSL/TLS certificate.\n"
              << "  -C certaddr  [Optional] Directory to search for certificates used to validate the server's SSL/TLS certificate.\n"
              << "               Default value is /etc/ssl/certs.\n"
              << "  -n           [Optional] Only read new messages.\n"
              << "  -h           [Optional] Download only message headers.\n"
              << "  -a auth_file [Required] Specifies the authentication file (LOGIN command).\n"
              << "  -b MAILBOX   [Optional] Specifies the mailbox name to work with on the server. Default is INBOX.\n"
              << "  -o out_dir   [Required] Specifies the output directory where downloaded messages will be saved.\n";
}


int main(int argc, char *argv[]){

    if (argc < 4){
        printUsage();
        return 1;
    }

    string server;
    int port;
    string certfile;
    string certaddr = "/etc/ssl/certs";
    string auth_file;
    string MAILBOX = "INBOX";
    string out_dir;

    bool encryption = false;
    bool only_new = false;
    bool only_header = false;

    int opt;
    while ((opt = getopt(argc, argv, "p:Tc:C:nha:b:o:")) != -1) {
        switch(opt){
            case 'p':
                port = atoi(optarg);
                break;
            case 'T':
                encryption = true;
                break;
            case 'c':
                certfile = optarg;
                break;
            case 'C':
                certaddr = optarg;
                break;
            case 'n':
                only_new = true;
                break;
            case 'h':
                only_header = true;
                break;
            case 'a':
                auth_file = optarg;
                break;
            case 'b':
                MAILBOX = optarg;
                break;
            case 'o':
                out_dir = optarg;
                break;
            default:
                printUsage();
                return 1;
            
        }
    }

    //check non-optional arguments
    if (optind >= argc) {
        std::cerr << "[ERROR] server is required.\n";
        printUsage();
        return 1;
    }
    server = argv[optind];

    if (auth_file.empty()) {
        std::cerr << "[ERROR] argument -a auth_file is required.\n";
        printUsage();
        return 1;
    }
    if (out_dir.empty()) {
        std::cerr << "[ERROR] argument -o out_dir is required.\n";
        printUsage();
        return 1;
    }
    std::cout << "new flag a header flag " << only_new << " " << only_header << std::endl;
    Imapcl::run(server, port, certfile, certaddr, encryption, only_new, only_header, auth_file, MAILBOX, out_dir);
}