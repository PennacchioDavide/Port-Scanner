#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

void port_scanner(int argc, char *argv[])
{
    char tIP[16] = {0};
    strcpy(tIP, argv[1]);
    char first_port[6] = {0};
    strcpy(first_port, argv[2]);
    char last_port[6] = {0};
    strcpy(last_port, argv[3]);

    
}

int main(int argc, char *argv[])
{
    if (argc < 4 && argc > 4)
    {
        printf("Please input IP address and ports\n");
        exit(1);
    }

    port_scanner(argc, argv);

    return 0;
}