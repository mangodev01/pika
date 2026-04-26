#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    bool verbose   = false;
    bool dryRun    = false;
    char* host     = "localhost";
    uint64_t port  = 8080;
    char* username = "root";

    static struct option long_options[] = {
        {"verbose", no_argument,       0, 'v'},
        {"dryRun",  no_argument,       0, 'd'},
        {"dry",     no_argument,       0, 'd'},
        {"host",    required_argument, 0, 'H'},
        {"port",    required_argument, 0, 'p'},
        {"username",required_argument, 0, 'u'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "vdH:p:u:n:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'v': verbose = true;           break;
            case 'd': dryRun  = true;           break;
            case 'H': host     = optarg;        break;
            case 'p': port     = strtoull(optarg, NULL, 0); break;
            case 'u':
            case 'n': username = optarg;        break;
        }
    }

    printf("verbose:  %d\n", verbose);
    printf("dryRun:   %d\n", dryRun);
    printf("host:     %s\n", host);
    printf("port:     %lu\n", port);
    printf("username: %s\n", username);
}
