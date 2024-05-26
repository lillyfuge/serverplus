#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8888
#define BUFFER_SIZE 1024
int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return -1;
    }

    printf("Connected to the server. Type your message, press Enter to send, 'exit' to quit.\n");

    char buffer[BUFFER_SIZE];
    while (1) {
        fgets(buffer, BUFFER_SIZE, stdin);
        if (!strcmp(buffer, "exit\n")) {
            break;
        }

        // Send data to server
        if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
            perror("Send failed");
            break;
        }

        // Receive echo from server
        ssize_t bytes_received = recv(sockfd, buffer, BUFFER_SIZE-1, 0);
        if (bytes_received <= 0) {
            perror("Receive failed");
            break;
        }
        buffer[bytes_received] = '\0'; // Null-terminate received data
        printf("Server echoed: %s", buffer);
    }

    close(sockfd);
    return 0;
}