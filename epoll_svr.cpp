#include <iostream>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <stdio.h>
#include <map>

void SetNonBlocking(int fd)
{
    int opts = fcntl(fd, F_GETFL);
    if (opts < 0)
    {
        std::cout<<"get fcntl failed"<<std::endl;
        return;
    }
    opts |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, opts) < 0)
    {
        std::cout<<"set fcntl O_NONBLOCK failed"<<std::endl;
        return;
    }
}

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr, client_addr;
    socklen_t sock_len = sizeof(client_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(8989);
    SetNonBlocking(sockfd);
    if (bind(sockfd, reinterpret_cast<struct sockaddr*>(&server_addr), sock_len) < 0)
    {
        std::cout<<"bind error"<<std::endl;
        return -1;
    }
    if (listen(sockfd, 5) < 0)
    {
        std::cout<<"listen error"<<std::endl;
        return -1;
    }
    int epfd = epoll_create(1024);
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT;
    ev.data.fd = sockfd;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
    if (ret < 0)
    {
        std::cout<<"epoll_ctl error"<<std::endl;
        return -1;
    }
    struct epoll_event events[1024];
    for (;;)
    {
        usleep(20);
        int num = epoll_wait(epfd, events, 1024, -1);
        for (int i = 0; i < num; ++i)
        {
            epoll_event& event = events[i];
            int fd = event.data.fd;
            if (fd == sockfd && event.events & EPOLLIN)
            {
                int clientfd = accept(sockfd, (struct sockaddr*)(&client_addr), &sock_len);
                if (clientfd <= 0)
                {
                    std::cout<<"accept failed"<<std::endl;
                    continue;
                }
                std::cout<<"client connected,fd:"<<clientfd<<std::endl;
                SetNonBlocking(clientfd);
                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLOUT;
                ev.data.fd = clientfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);
                continue;
            }
            else if (event.events & EPOLLIN)
            {
                // can read
                char buf[1024];
                int bytes = recv(event.data.fd, buf, sizeof(buf) - 1, 0);
                buf[bytes] = 0;
                if (bytes < 0)
                {
                    if (errno == EAGAIN)
                    {
                        std::cout<<"read eagain error"<<std::endl;
                        continue;
                    }
                    std::cout<<"close "<<event.data.fd<<std::endl;
                    close(event.data.fd);
                    struct epoll_event ev;
                    ev.data.fd = event.data.fd;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, event.data.fd, &ev);
                }
                else if (bytes == 0)
                {
                    std::cout<<"close "<<event.data.fd<<std::endl;
                    close(event.data.fd);
                    struct epoll_event ev;
                    ev.data.fd = event.data.fd;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, event.data.fd, &ev);
                }
                else
                {
                    //std::cout<<"recv from client:"<<buf<<std::endl;
                }
            }
            else if (event.events & EPOLLOUT)
            {
                // can write
                char buf[1024];
                int num = snprintf(buf, sizeof(buf), "this message is from server");
                int bytes = send(event.data.fd, buf, num, 0);
                if (bytes < 0)
                {
                    if (errno == EAGAIN)
                    {
                        std::cout<<"write eagain error"<<std::endl;
                        continue;
                    }
                    std::cout<<"close "<<event.data.fd<<std::endl;
                    close(event.data.fd);
                    struct epoll_event ev;
                    ev.data.fd = event.data.fd;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, event.data.fd, &ev);
                }
                else if (bytes == 0)
                {
                    std::cout<<"close "<<event.data.fd<<std::endl;
                    close(event.data.fd);
                    struct epoll_event ev;
                    ev.data.fd = event.data.fd;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, event.data.fd, &ev);
                }
                else
                {
                    //std::cout<<"send succeed"<<std::endl;
                }
            }
        }
    }
}
