#include "selecttopool.h"
#include <stdio.h>

void MY_FD_ZERO(fd_my_set* set) {
    if (set == NULL) return;

    set->nfds = 0;
    // 首次初始化时分配内存
    if (set->fds == NULL) {
        set->max_size = 1024;
        set->fds = (pollfd_t*)malloc(sizeof(pollfd_t) * set->max_size);
    }
    // 清空数组内容
    if (set->fds != NULL) {
        memset(set->fds, 0, sizeof(pollfd_t) * set->max_size);
    }
}

void MY_FD_SET(socket_fd_t fd, fd_my_set* set) {
    if (set == NULL || set->fds == NULL) {
        printf("FD_SET error: invalid set or uninitialized fds\n");
        return;
    }

    // 动态扩容
    if (set->nfds >= set->max_size) {
        int new_size = set->max_size * 2;
        pollfd_t* new_fds = (pollfd_t*)realloc(
            set->fds,
            sizeof(pollfd_t) * new_size
        );
        if (new_fds == NULL) {
            printf("FD_SET error: realloc failed for size %d\n", new_size);
            return;
        }
        set->fds = new_fds;
        set->max_size = new_size;
    }

    // 检查是否已存在该fd
    int i;
    for (i = 0; i < set->nfds; i++) {
        if (set->fds[i].fd == fd) {
            return;  // 已存在，无需重复添加
        }
    }

    // 添加新fd（监听读事件）
    if (i == set->nfds) {
        set->fds[set->nfds].fd = fd;
        set->fds[set->nfds].events = POLLIN;
        set->fds[set->nfds].revents = 0;
        set->nfds++;
    }
}

void MY_FD_CLR(socket_fd_t fd, fd_my_set* set) {
    if (set == NULL || set->fds == NULL || set->nfds <= 0) {
        return;
    }

    // 查找fd并移除
    for (int i = 0; i < set->nfds; i++) {
        if (set->fds[i].fd == fd) {
            // 用最后一个元素覆盖当前位置
            if (i != set->nfds - 1) {
                set->fds[i] = set->fds[set->nfds - 1];
            }
            set->nfds--;
            break;
        }
    }
}
int MY_FD_CONTAINS(socket_fd_t fd, const fd_my_set* set) {
    if (set == NULL || set->fds == NULL || set->nfds <= 0) {
        return 0;
    }

    // 遍历集合，检查fd是否存在
    for (int i = 0; i < set->nfds; i++) {
        if (set->fds[i].fd == fd) {
            return 1; // 存在于集合中
        }
    }
    return 0; // 不在集合中
}
int MY_FD_ISSET(int fd, const fd_my_set* set) {
    // 安全检查：避免空指针
    if (set == NULL || set->fds == NULL || set->nfds <= 0) {
        return 0;
    }

    // 缓存集合信息，提升性能
    const int nfds = set->nfds;
    pollfd_t* fds = set->fds;

    // 遍历查找目标fd并检查事件
    for (int i = 0; i < nfds; i++) {
        if (fds[i].fd == fd) {
            // 检查可读、错误、挂断事件
            if (fds[i].revents & (POLLIN | POLLERR | POLLHUP)) {
                return 1; // 就绪
            }
            break; // 找到后无需继续遍历
        }
    }

    return 0; // 未就绪
}
void MY_FD_DESTROY(fd_my_set* set) {
    if (set == NULL) return;

    if (set->fds != NULL) {
        free(set->fds);
        set->fds = NULL;
        set->nfds = 0;
        set->max_size = 0;
    }
}
int my_select(int nfds, fd_my_set* readfds, fd_my_set* writefds, fd_my_set* exceptfds, struct timeval* timeout) {
    // 转换超时时间（毫秒）
    int timeout_ms = -1;  // -1表示无限等待
    if (timeout != NULL) {
        timeout_ms = (int)(timeout->tv_sec * 1000 + timeout->tv_usec / 1000);
    }

    // 仅处理读事件集合（如需写/异常事件，可扩展）
    if (readfds == NULL || readfds->fds == NULL) {
        return 0;
    }
    int i32Ret = 0;
#ifdef _WIN32
    // Windows使用WSAPoll
    i32Ret = WSAPoll(readfds->fds, readfds->nfds, timeout_ms);
#else
    // Linux使用原生poll
    i32Ret = poll(readfds->fds, readfds->nfds, timeout_ms);
#endif
    return i32Ret;
}
