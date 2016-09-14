#include <iostream>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
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

static int g_idx = 0;
static const char* kServAddr = "192.168.232.128";
static int kPort = 8989;
static uint32_t kConnected = 0;
static uint32_t kConnecting = 1;
static uint32_t kDisconnected = 2;

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        std::cout<<"parameter invalid"<<std::endl;
        return -1;
    }
    int count = atoi(argv[1]);
    int epfd = epoll_create(1024);
    for (int i = 0; i < count; ++i)
    {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            std::cout<<"create socket error"<<std::endl;
            return -1;
        }
        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(kServAddr);
        serv_addr.sin_port = htons(kPort);
        uint32_t status = kDisconnected;
        SetNonBlocking(sockfd);
        int ret = connect(sockfd, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr));
        if ( ret < 0)
        {
            if (errno != EINPROGRESS)
            {
                std::cout<<"connect to server error, errno:"<<errno<<std::endl;
                continue;
            }
            std::cout<<"connect to server connecting, errno:"<<errno<<std::endl;
            status = kConnecting;
        }
        else
        {
            status = kConnected;
        }
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLOUT;
        ev.data.fd = sockfd;
        ev.data.u32 = status;
        epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
    }
    struct epoll_event events[1024];
    for (;;)
    {
        usleep(20);
        int num = epoll_wait(epfd, events, 1024, -1);
        for (int i = 0; i < num; ++i)
        {
            epoll_event& event = events[i];
            if (event.events & EPOLLIN)
            {
                uint32_t status = event.data.u32;
                if (status == kConnecting)
                {
                    event.data.u32 = kConnected;
                    continue;
                }
                // can read
                char buf[1024];
                int bytes = recv(event.data.fd, buf, sizeof(buf) - 1, 0);
                if (bytes < 0)
                {
                    if (errno == EAGAIN)
                    {
                        std::cout<<"read eagain error"<<std::endl;
                        continue;
                    }
                }
                else if (bytes == 0)
                {
                    close(event.data.fd);
                    std::cout<<"close "<<event.data.fd<<std::endl;
                    struct epoll_event ev;
                    ev.data.fd = event.data.fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, event.data.fd, &ev);
                }
                else
                {
                    //std::cout<<"recv from server:"<<buf<<std::endl;
                }
            }
            else if (event.events & EPOLLOUT)
            {
                // can write
                char buf[1024];
                int count = snprintf(buf, sizeof(buf), "this message is from client,idx:%d", g_idx);
                int bytes = send(event.data.fd, buf, count, 0);
                if (bytes < 0)
                {
                    if (errno == EAGAIN)
                    {
                        std::cout<<"write eagain error"<<std::endl;
                        continue;
                    }
                }
                else if (bytes == 0)
                {
                    close(event.data.fd);
                    std::cout<<"close "<<event.data.fd<<std::endl;
                    struct epoll_event ev;
                    ev.data.fd = event.data.fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, event.data.fd, &ev);
                }
                else
                {
                    //std::cout<<"send to server succeed"<<std::endl;
                    ++g_idx;
                }
            }
        }
    }
}
