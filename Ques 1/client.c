#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

typedef struct {
    int thread_id;
} thread_arg_t;


void printMess(int flag){
    if(flag == 1){
        printf("Waiting for connections...\n");
    }else if(flag == 2){
        printf("Error in connection...\n");
    }else if(flag==0){
        return;
    }
}

void recServerResp(int sock, char* buffer, thread_arg_t *t_arg){
    int bytes_read = read(sock, buffer, 1024);
    printMess(0);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("Thread %d: Server response:\n%s\n", t_arg->thread_id, buffer);
    }
}

void connServer(int sock, struct sockaddr_in serv_addr, thread_arg_t *t_arg){
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Thread %d: Connection failed\n", t_arg->thread_id);
        printMess(0);
        pthread_exit(NULL);
    }

    // Send request to the server
    char request[] = "GET CPU INFO";
    send(sock, request, strlen(request), 0);
}


void *handle_client(void *arg) {
    thread_arg_t *t_arg = (thread_arg_t *)arg;
    int sock = 0;
    struct sockaddr_in serv_addr;
    printMess(0);
    char buffer[1024];

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Thread %d: Socket creation error\n", t_arg->thread_id);
        printMess(2);
        pthread_exit(NULL);
    }

    serv_addr.sin_family = AF_INET;
    printMess(0);
    serv_addr.sin_port = htons(8080);

    // Convert IPv4 and IPv6 addresses from text to binary
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Thread %d: Invalid address/ Address not supported\n", t_arg->thread_id);
        printMess(2);
        pthread_exit(NULL);
    }

    // Connect to server
    connServer(sock, serv_addr, t_arg);

    // Receive server response
    recServerResp(sock, buffer, t_arg);

    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int errorMainCheck = 0;
    int n = 1;
    printMess(0);
    if (argc > 1) n = atoi(argv[1]);

    pthread_t threads[n];
    printMess(0);
    thread_arg_t thread_args[n];

    for (int i = 0; i < n; i++) {
        thread_args[i].thread_id = i + 1;
        printMess(0);
        if (pthread_create(&threads[i], NULL, handle_client, (void *)&thread_args[i]) != 0 || errorMainCheck != 0) {
            printf("Error creating thread %d\n", i + 1);
            printMess(2);
            return -1;
        }
    }

    for (int i = 0; i < n; i++) {
        printMess(0);
        pthread_join(threads[i], NULL);
    }

    return 0;
}