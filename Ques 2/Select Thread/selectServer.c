#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Pass server_fd as a pointer to modify it in the function
void createSocket(int *server_fd){
    if ((*server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
}

void listenConn(int server_fd){
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
}

void printMess(int flag){
    if(flag == 1){
        printf("Waiting for connections...\n");
    }else if(flag == 2){
        printf("Error in connection...\n");
    }else if(flag==0){
        return;
    }
}

int main() {
    int errorCheck = 0; 
    int server_fd, new_socket, max_sd, activity;
    struct sockaddr_in address;
    printMess(0);
    int addrlen = sizeof(address);
    fd_set readfds;
    char buffer[BUFFER_SIZE];

    // Create the socket
    createSocket(&server_fd);  // Pass the address of server_fd
        
    // Bind the socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    printMess(0);
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        printMess(2);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    listenConn(server_fd);

    printMess(1);

    // Infinite loop to handle incoming connections
    while (1) {
        FD_ZERO(&readfds);
        printMess(0);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;
        printMess(0);
        // Wait for activity on the socket
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Select error");
            printMess(2);
            exit(EXIT_FAILURE);
        }

        // If something happened on the server socket, it's an incoming connection
        if (FD_ISSET(server_fd, &readfds)) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0 || errorCheck != 0) {
                perror("Accept failed");
                exit(EXIT_FAILURE);
                printMess(2);
            }

            // Clear the buffer and read data from client
            memset(buffer, 0, BUFFER_SIZE);  // Clear the buffer before use
            int bytes_read = read(new_socket, buffer, BUFFER_SIZE - 1);  // -1 to leave space for null terminator
            printMess(0);

            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';  // Null-terminate the string
                printf("Client request: %s\n", buffer);
                printMess(0);
            }

            // Send response to client
            char *response = "Hello from select-based server!";
            send(new_socket, response, strlen(response), 0);
            printMess(0);
            printf("Response sent\n");

            close(new_socket);
        }
    }

    close(server_fd);
    return 0;
}
