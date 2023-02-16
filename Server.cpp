#include "netinet/in.h"
#include "stdio.h"
#include "sys/socket.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include "sys/select.h"
#include "errno.h"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int new_socket;
    fd_set readfds;
    int opt = 1;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Create socket failed");
        return 0;
    }

    const int port = 8080;

    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Set socket options failed");
        return 0;
    }

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return 0;
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        return 0;
    }

    int max_sd;
    int clients[MAX_CLIENTS] = {0};

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i];
            if (sd > 0) {
                FD_SET(sd, &readfds);
                if (sd > max_sd) {
                    max_sd = sd;
                }
            }
        }

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno!=EINTR)) {
            printf("Select error");
        }

        if (FD_ISSET(server_fd, &readfds)) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                perror("Accept failed");
                return 0;
            }

            printf("New client connected\n");

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] == 0) {
                    clients[i] = new_socket;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i];
            if (FD_ISSET(sd, &readfds)) {
                char buffer[BUFFER_SIZE] = { 0 };

                int valread = read(sd, buffer, BUFFER_SIZE);

                if (valread == 0) {
                    printf("Client disconnected\n");
                    close(sd);
                    clients[i] = 0;
                } else {
                    printf("%s\n", buffer);
                    char* greetings = "Hello from the server";
                    write(sd, greetings, strlen(greetings));
                }
            }
        }
    }

    return 0;
}
