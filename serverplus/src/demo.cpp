#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#define ERR_EXIT(m) \
        do {\
            perror(m); \
            exit(EXIT_FAILURE); \
        } while(0)

using EventList = std::vector<epoll_event>;
int main()
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    //创建一个监听套接字(非阻塞套接字)
    int listenfd;
    listenfd = socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(listenfd < 0){
        ERR_EXIT("socket");
    }
    //设置地址
    struct sockaddr_in srvAddr;
    memset(&srvAddr,0,sizeof(srvAddr));//初始化
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(8888);
    srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //设置地址的重复利用
    int on = 1;
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0){
        ERR_EXIT("setsockopt");
    }
    //绑定
    if(bind(listenfd, (sockaddr *)&srvAddr, sizeof(srvAddr)) < 0){
        ERR_EXIT("bind");
    }
    //监听
    if(listen(listenfd, SOMAXCONN) < 0){
        ERR_EXIT("listen");
    }
    std::vector<int> clients;
    int epollfd = epoll_create(EPOLL_CLOEXEC);
    //使用EPOLLIN并关注EPOLLIN事件
    struct epoll_event event;
    event.data.fd = listenfd;
    event.events = EPOLLIN;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event);
    //储存epoll的描述符
    EventList events(16);
    struct sockaddr_in peerAddr;
    socklen_t peerlen;
    int connfd;
    int idlefd;//空闲描述符
    int nready;
    //循环处理
    while (true) {
        //取事件
        nready = epoll_wait(epollfd, events.data(), static_cast<int>(events.size()), -1);
        if(nready == -1){
            if(errno == EINTR){
                continue;
            }
            ERR_EXIT("epoll_wait");
        }
        else if(nready == 0){
            continue;
        }
        if (static_cast<size_t>(nready) == events.size()) {//强制类型转换
            events.resize(events.size() * 2);
        }   
        for (auto e : events) {
            if (e.data.fd == listenfd) {
                peerlen = sizeof(peerAddr);
                connfd = ::accept4(listenfd, (sockaddr *)&peerAddr, &peerlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
                if (connfd == -1) {
                    if (errno == EMFILE) {
                        close(idlefd);
                        idlefd = accept(listenfd, NULL, NULL);
                        close(idlefd);
                        idlefd = open("/dev/null", O_RDONLY | O_CLOEXEC);
                        continue;
                    }
                    else 
                        ERR_EXIT("accept4");
                }
                clients.push_back(connfd);
                event.data.fd = connfd;
                event.events = EPOLLIN;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event);
                std::cout << " connection from ip = " << inet_ntoa(peerAddr.sin_addr)
                          << " port = " << ntohs(peerAddr.sin_port) << std::endl;
              //将这两段代码放到请求队列.
            }
            else if (e.events & EPOLLIN){
                connfd = e.data.fd;
                char buf[1024] = {0};
                int ret = read(connfd, buf, 1024);
                if (ret == -1)
                    ERR_EXIT("read");
                if (ret == 0) {
                    std::cout << "client closed" << std::endl;
                    event = e; 
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, &event);
                    clients.erase(std::remove(clients.begin(), clients.end(), connfd), clients.end());
                    close(connfd);
                    continue;
                }
                std::cout << "msg: " << buf << std::endl;
                write(connfd, buf, strlen(buf));
            }
        }
    }
    return 0;
}
