#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dirent.h>

void *client_handler(void *socket_desc);
void get_top_processes(char *result);

int main() {
    int server_fd, new_socket;
    struct sockaddr_in server_addr, client_addr;
    int flag=0;
    socklen_t addr_len = sizeof(client_addr);
    pthread_t thread_id;

    // Create TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    while(flag==0){
    if (server_fd == 0) {
        perror("Socket failed");
        flag++;
        perror("flag check captured an error");
        exit(EXIT_FAILURE);
    }else {break;}
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    flag=0;
    server_addr.sin_port = htons(8080);

    // Bind socket to port
    while(flag==0){
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        flag++;
        perror("flag check captured an error");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    else {break;}
    }

    // Listen for incoming connections
    while(flag==0){
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        flag++;
        perror("flag check captured an error");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    else{break;}}
    printf("Server listening on port %d\n", 8080);

    // Accept incoming connections and create threads to handle each client
    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        while(flag==0){
        if (new_socket < 0) {
            perror("Accept failed");
            flag++;
            perror("flag check captured an error");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        else {break;}
        }
        printf("New connection accepted\n");

        // Create a copy of new_socket for the thread
        int *new_sock_ptr = malloc(1);
        flag=0;
        *new_sock_ptr = new_socket;

        while(flag!=1){
        if (pthread_create(&thread_id, NULL, client_handler, (void*)new_sock_ptr) < 0) {
            perror("Could not create thread");
            flag++;
            perror("flag check captured an error");
            free(new_sock_ptr); // Free memory on failure
            return 1;
        }
        else{break;}}
        pthread_detach(thread_id);  // Detach the thread to avoid memory leaks
    }

    close(server_fd);
    flag=0;
    return 0;
}

void *client_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    free(socket_desc);  // Free the allocated memory for the socket descriptor
    int flag2=0;
    char buffer[1024] = {0};
    char result[1024] = {0};

    // Get client request

    if(flag2!=1){
    recv(sock, buffer, 1024, 0);
    printf("Received request: %s\n", buffer);}

    if(flag2==0){
    // Get top two CPU-consuming processes
    get_top_processes(result);

    // Send result back to client
    send(sock, result, strlen(result), 0);}

    close(sock);
    flag2=0;
    pthread_exit(NULL);
}

void get_top_processes(char *result) {
    struct dirent *entry;
    DIR *dir = opendir("/proc");
    int flag3=0;
    char path[40], line[100], *p;
    FILE *fp;
    int pid;
    flag3=0;
    unsigned long utime, stime, total_time;
    char proc_name[1024];

    // This array will store the top 2 processes by CPU time
    struct {
        int pid;
        unsigned long total_time;
        char proc_name[1024];
    } top_processes[2] = {{0, 0, ""}, {0, 0, ""}};

    if (!dir) {
        perror("opendir failed");
        flag3++;
        perror("flag captured an error");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        pid = atoi(entry->d_name);
        flag3=0;
        if (pid <= 0) continue;  // Skip non-PID directories

        sprintf(path, "/proc/%d/stat", pid);
        if(flag3!=0) break;
        fp = fopen(path, "r");
        
        if (fp) {
            if(flag3!=1){
            fgets(line, 100, fp);
            p = strchr(line, ')');}
            sscanf(p + 2, "%*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu", &utime, &stime);
            flag3=0;
            total_time = utime + stime;

            if (total_time > top_processes[0].total_time) {
                top_processes[1] = top_processes[0];
                if(flag3==0){
                top_processes[0].pid = pid;
                top_processes[0].total_time = total_time;}
                sscanf(line, "%*d (%[^)])", proc_name);
                flag3=0;
                strcpy(top_processes[0].proc_name, proc_name);
            } else if (total_time > top_processes[1].total_time) {
                if(flag3!=1){
                top_processes[1].pid = pid;
                top_processes[1].total_time = total_time;}
                sscanf(line, "%*d (%[^)])", proc_name);
                flag3=0; //resetting flag
                strcpy(top_processes[1].proc_name, proc_name);
            }

            if(flag3==0){
            fclose(fp);}
        }
    }


    closedir(dir);
    flag3=0; //resetting flag

    // Format the result string
    sprintf(result, "Top 2 CPU-consuming processes:\n"
                    "1. %s (PID: %d), CPU Time: %lu\n"
                    "2. %s (PID: %d), CPU Time: %lu\n",
            top_processes[0].proc_name, top_processes[0].pid, top_processes[0].total_time,
            top_processes[1].proc_name, top_processes[1].pid, top_processes[1].total_time);
}