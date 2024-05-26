#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_EVENTS 10
#define BUFFER_SIZE 1024





int create_and_bind(const char *port) {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }
    int optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(port));
    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind error");
        close(listenfd);
        exit(EXIT_FAILURE);
    }
    return listenfd;
}
void set_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl getfl error");
        exit(EXIT_FAILURE);
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl setfl error");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int listenfd = create_and_bind(argv[1]);
    if (listen(listenfd, 1024) == -1) {
        perror("listen error");
        close(listenfd);
        exit(EXIT_FAILURE);
    }
    set_nonblock(listenfd);
    int epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1 error");
        close(listenfd);
        exit(EXIT_FAILURE);
    }
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = listenfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event) == -1) {
        perror("epoll_ctl add error");
        close(listenfd);
        close(epollfd);
        exit(EXIT_FAILURE);
    }
    while (1) {
        struct epoll_event events[MAX_EVENTS];
        int num_events = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (num_events == -1) {
            perror("epoll_wait error");
            continue;
        }
        for (int i = 0; i < num_events; i++) {
            if (events[i].data.fd == listenfd) {
                struct sockaddr_in clientaddr;
                socklen_t clientlen = sizeof(clientaddr);
                int connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
                //创建一个协程来处理accept.
                if (connfd == -1) {
                    perror("accept error");
                    continue;
                }
                set_nonblock(connfd);
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = connfd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event) == -1) {
                    perror("epoll_ctl add error");
                    close(connfd);
                    continue;
                }
            } else {
                char buffer[BUFFER_SIZE];
                int n = read(events[i].data.fd, buffer, BUFFER_SIZE);
                if (n == -1 && errno != EAGAIN) {
                    perror("read error");
                    close(events[i].data.fd);
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                } else if (n > 0) {
                    write(events[i].data.fd, buffer, n);
                } else {  // n == 0, client closed the connection
                    close(events[i].data.fd);
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                }
            }
        }
    }
    close(listenfd);
    close(epollfd);
    return 0;
}