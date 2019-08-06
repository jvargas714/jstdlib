#include <algorithm>
#include <climits>
#include <arpa/inet.h>
#include <sys/select.h>
#include "fd_sets.h"
#include "net_types.h"
#include "logger.h"

#define GEN LOG_MODULE::GENERAL


fd_sets::fd_sets(): max_fd(0), working_set{0}, master_set{0}, timeout{0} {};

std::vector<int> fd_sets::get_active_fds() const {
    std::vector<int> fds;
    // std::cout << "MAX FD: " << max_fd << std::endl;
    for (int fd = 0; fd <= max_fd; fd++) {
        if (FD_ISSET(fd, &working_set)) {
            LOG_DEBUG(GEN, "adding active fd: ", fd);
            fds.push_back(fd);
        }
    }
    LOG_DEBUG(GEN, "returning ", fds.size(), " active sockets");
    return fds;
}

void fd_sets::clear() {
    FD_ZERO(&master_set);
    FD_ZERO(&working_set);
    max_fd = 0;
}

void fd_sets::set_working_set() {
    FD_ZERO(&working_set);
    working_set = master_set;
}

void fd_sets::add_fd(int fd) {
    FD_SET(fd, &master_set);
    max_fd = std::max(fd, max_fd);
}

    // __time_t tv_sec;		/* Seconds.  */
    // __suseconds_t tv_usec;	/* Microseconds.  */

void fd_sets::set_timeout_ms(long millisecs) {
    int64_t tmp = millisecs * 1000;
    if (tmp > LONG_MAX) {
        timeout.tv_sec = millisecs / 1000;
        timeout.tv_usec = 0;
    } else {
        timeout.tv_sec = 0;
        timeout.tv_usec = millisecs * 1000;
    }
}

int fd_sets::select_set(bool readfds) {
    if (readfds)
        return select(max_fd+1, 
        &working_set, 
        NULL, 
        NULL,
        NULL);
        // ((timeout.tv_sec == 0 && timeout.tv_usec == 0) ? nullptr:&timeout));
    else 
        return select(max_fd+1, 
        NULL, 
        &working_set, 
        NULL,
        &timeout);
}

void fd_sets::clear_fd(int fd) {
    FD_CLR(fd, &master_set);
}

