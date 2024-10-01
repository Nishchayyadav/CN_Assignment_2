#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

void listenConn(int server_fd){
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
}

void bindSocket(int server_fd, struct sockaddr_in address){
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
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
    int server_fd, new_socket;
    struct sockaddr_in address;
    printMess(0);
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        errorCheck = 1;
        printMess(2);
        exit(EXIT_FAILURE);
    }

    // Set up address struct
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // Bind the socket
    bindSocket(server_fd, address);

    // Listen for connections
    listenConn(server_fd);

    //Logging Connection Message
    printMess(1);

    // Loop to accept multiple clients
    while (1) {
        // Accept a new client
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0 || errorCheck != 0) {
            perror("Accept failed");
            printMess(2);
            continue; // Continue to next iteration on failure
        }

        // Read client request
        read(new_socket, buffer, 1024);
        printMess(0);
        printf("Client request: %s\n", buffer);

        // Respond to client
        char *response = "Hello from server!";
        send(new_socket, response, strlen(response), 0);
        printMess(0);
        printf("Response sent\n");

        // Clear the buffer for the next request
        memset(buffer, 0, 1024);

        // Close the connection to this client
        printMess(0);
        close(new_socket);
    }

    // Close the server socket (unreachable code in this case)
    close(server_fd);
    return 0;
}