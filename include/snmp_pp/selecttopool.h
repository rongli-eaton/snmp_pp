#ifndef MY_POLL_WRAPPER_H
#define MY_POLL_WRAPPER_H

#include <stdlib.h>
#include <string.h>
#include "config_snmp_pp.h"

// 区分Windows和Linux平台
#ifdef _WIN32
#include <winsock2.h>
// Windows使用WSAPoll，对应结构体为WSAPOLLFD
typedef WSAPOLLFD pollfd_t;
typedef SOCKET socket_fd_t;  // Windows的socket类型
#else
#include <poll.h>
// Linux使用原生poll，对应结构体为pollfd
typedef struct pollfd pollfd_t;
typedef int socket_fd_t;     // Linux的socket类型（int）
#endif

// 自定义fd_my_set结构体（跨平台统一）
typedef struct myfdset {
    myfdset():fds(nullptr),nfds(0),max_size(0){}
    ~myfdset(){
        if (fds) {
            free(fds);
            fds = nullptr;
        }
        nfds = 0;
        max_size = 0;
    }
    pollfd_t* fds;           // 存储pollfd数组
    int nfds;                // 当前fd数量
    int max_size;            // 数组最大容量（支持动态扩容）
} fd_my_set;

void MY_FD_ZERO(fd_my_set* set);

void MY_FD_SET(socket_fd_t fd, fd_my_set* set);

void MY_FD_CLR(socket_fd_t fd, fd_my_set* set);

int MY_FD_CONTAINS(socket_fd_t fd, const fd_my_set* set);

int MY_FD_ISSET(int fd, const fd_my_set* set);

void MY_FD_DESTROY(fd_my_set* set);

int DLLOPT my_select(int nfds, fd_my_set* readfds, fd_my_set* writefds, fd_my_set* exceptfds, struct timeval* timeout);
       

#endif  // POLL_WRAPPER_H
