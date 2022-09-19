#include "netinet/in.h"
#include "stdio.h"
#include "sys/socket.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"

int main(int argc, char *argv[]) {

    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int new_socket;
    
    int opt{1};


    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Create socket failed");
        return 0;
    }

    const int port{8080};

    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return 0;
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        return 0;
    }

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            return 0;
        }

        char buffer[1024] = { 0 };

        int valread = read(new_socket, buffer, 1024);

        printf("%s\n", buffer);
        if (valread < 0) {
            printf("Nothing to read");
        }

        char* greetings = "Hello from the server";
        write(new_socket, greetings, strlen(greetings));

        close(new_socket);
    }
    return 0;
}