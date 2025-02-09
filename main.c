#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

// Function to perform service detection
void service_detection(int sockfd) {
    char buffer[1024] = {0};
    const char *http_request = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    
    // Send a simple HTTP request
    send(sockfd, http_request, strlen(http_request), 0);
    
    // Receive and analyze the response
    int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        if (strstr(buffer, "HTTP")) {
            printf("Service detected: HTTP\n");
        } else if (strstr(buffer, "SSH")) {
            printf("Service detected: SSH\n");
        } else if (strstr(buffer, "FTP")) {
            printf("Service detected: FTP\n");
        } else {
            printf("Unknown service detected: %s\n", buffer);
        }
    } else {
        printf("No banner or response received. Service detection failed.\n");
    }
}

// Function to scan a single port
int scan_port(const char *ip, int port, int detect_service) {
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", port);

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(ip, port_str, &hints, &res) != 0) {
        fprintf(stderr, "Error resolving address for port %d.\n", port);
        return 0;
    }

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("Socket creation failed");
        freeaddrinfo(res);
        return 0;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0) {
        printf("Port %d is open.\n", port);
        if (detect_service) {
            service_detection(sockfd);
        }
        close(sockfd);
        freeaddrinfo(res);
        return port;
    } else {
        printf("Port %d is NOT open.\n", port);
    }

    close(sockfd);
    freeaddrinfo(res);
    return 0;
}

// Function to scan a range of ports
void scan_ports_range(const char *ip, int start_port, int end_port) {
    printf("Scanning ports from %d to %d...\n", start_port, end_port);
    int open_ports[65535];
    int count = 0;

    for (int port = start_port; port <= end_port; port++) {
        if (scan_port(ip, port, 0)) {
            open_ports[count++] = port;
        }
        usleep(10000); // Small delay to avoid overwhelming the target
    }

    printf("\nOpen Ports:\n");
    for (int i = 0; i < count; i++) {
        printf("[PORT %d]\n", open_ports[i]);
    }
}

// Function to perform basic OS detection using TTL
void detect_os(const char *ip) {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("Raw socket creation failed");
        return;
    }

    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &dest.sin_addr);

    // Send an empty ICMP packet
    if (sendto(sockfd, NULL, 0, 0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
        perror("ICMP send failed");
        close(sockfd);
        return;
    }

    char buffer[64];
    socklen_t len = sizeof(dest);
    if (recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&dest, &len) > 0) {
        struct ip *ip_hdr = (struct ip *)buffer;
        int ttl = ip_hdr->ip_ttl;
        printf("TTL Value: %d\n", ttl);

        if (ttl >= 64 && ttl <= 128) {
            printf("Operating System: Likely Linux/Unix-based\n");
        } else if (ttl > 128) {
            printf("Operating System: Likely Windows\n");
        } else {
            printf("Operating System: Unknown\n");
        }
    } else {
        printf("No ICMP response received. OS detection failed.\n");
    }

    close(sockfd);
}

int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s <IP Address> <Port> || <IP Address> <First Port> <Last Port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *ip = argv[1];

    if (argc == 3) {
        int port = atoi(argv[2]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port number. Must be between 1 and 65535.\n");
            exit(EXIT_FAILURE);
        }
        scan_port(ip, port, 1);
    } else if (argc == 4) {
        int start_port = atoi(argv[2]);
        int end_port = atoi(argv[3]);
        if (start_port <= 0 || start_port > 65535 || end_port <= 0 || end_port > 65535 || start_port > end_port) {
            fprintf(stderr, "Invalid port range. Ports must be between 1 and 65535, and start port must be <= end port.\n");
            exit(EXIT_FAILURE);
        }
        scan_ports_range(ip, start_port, end_port);
    }

    return 0;
}