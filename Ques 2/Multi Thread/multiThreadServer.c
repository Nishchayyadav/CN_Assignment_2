#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void printMess(int flag){
    if(flag == 1){
        printf("Waiting for connections...\n");
    }else if(flag == 2){
        printf("Error in connection...\n");
    }else if(flag==0){
        return;
    }
}

// Function to handle communication with a client
void *handle_client(void *arg) {
    int new_socket = *(int *)arg;
    free(arg);  // Free the dynamically allocated memory

    char buffer[BUFFER_SIZE] = {0};
    printMess(0);
    // Read the client request
    int bytes_read = read(new_socket, buffer, BUFFER_SIZE);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';  // Null-terminate the buffer
        printMess(0);
        printf("Client request: %s\n", buffer);

        // Prepare a response based on the client's request
        char *response = "Hello from multi-threaded server!";
        printMess(0);
        // Send the response back to the client
        send(new_socket, response, strlen(response), 0);
        printf("Response sent\n");
    }

    // Close the socket for this client
    close(new_socket);
    return NULL;
}

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

int main() {
    int errorCheck = 0;
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create the socket (pass the address of server_fd)
    createSocket(&server_fd);
    
    // Configure the address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    printMess(0);
    address.sin_port = htons(PORT);

    // Bind the socket to the address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        printMess(2);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    listenConn(server_fd);
   
    printMess(1);

    // Handle incoming connections in a loop
    while (1) {
        // Allocate memory for the new socket
        int *new_socket = malloc(sizeof(int));
        if ((*new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0 || errorCheck != 0) {
            perror("Accept failed");
            free(new_socket);  // Free memory if accept fails
            continue;
        }

        pthread_t tid;
        
        // Create a new thread for each client
        pthread_create(&tid, NULL, handle_client, (void *)new_socket);
        pthread_detach(tid);  // Detach thread to avoid memory leaks
    }

    // Close the server socket (this won't be reached in an infinite loop)
    close(server_fd);
    return 0;
}
