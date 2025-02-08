#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

int port_scanner(char *ip, int port)
{
    char port_str[6] = {0};
    snprintf(port_str, sizeof(port_str), "%d", port);

    struct addrinfo hints, *serv_addr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(ip, port_str, &hints, &serv_addr);
    if (status != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(serv_addr->ai_family, serv_addr->ai_socktype, serv_addr->ai_protocol);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        freeaddrinfo(serv_addr);
        return 0;
    }

    status = connect(sockfd, serv_addr->ai_addr, serv_addr->ai_addrlen);
    if (status < 0)
    {
        printf("Port %d is NOT open.\n", port);
    }
    else
    {
        printf("Port %d is open.\n", port);
        return port;

    }

    close(sockfd);
    freeaddrinfo(serv_addr);
    return 0;
}

void loop_port_scanner(char *ip, int start_port, int end_port)
{
    int open_ports[65535];
    int n = 0;
    int j = 0;
    for (int i = start_port; i <= end_port; i++)
    {
        n = port_scanner(ip, i);
        if (n != 0 && n >= 1)
        {
            open_ports[j] = n;
            n = 0;
            j++;
        }
        usleep(10000);
    }
    int k = 0;
    printf("\n\nOpen Ports:\n");
    while (k < j)
    {
        printf("[PORT %d]\n", open_ports[k]);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3 || argc > 4)
    {
        fprintf(stderr, "Usage: %s <IP Address> <Port> || <IP Address> <First Port> <Last Port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    else if (argc == 3)
    {
        int port = atoi(argv[2]);
        if (port <= 0 || port > 65535)
        {
            fprintf(stderr, "Invalid port number. Must be between 1 and 65535.\n");
            exit(EXIT_FAILURE);
        }
        port_scanner(argv[1], port);
    }
    else
    {
        int start_port = atoi(argv[2]);
        int end_port = atoi(argv[3]);
        if (start_port <= 0 || start_port > 65535 || end_port <= 0 || end_port > 65535 || start_port > end_port)
        {
            fprintf(stderr, "Invalid port range. Ports must be between 1 and 65535, and the start port must be <= end port.\n");
            exit(EXIT_FAILURE);
        }
        loop_port_scanner(argv[1], start_port, end_port);
    }

    return 0;
}
