//
// Created by Anton Shelepov on 2019-05-31.
//

#include "fifo.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include "../file_descriptor/file_descriptor.h"


fifo::fifo() {
    int fds[2];
    status &= (::pipe(fds) != -1);
    if (status) {
        in.first = fds[0];
        in.second = fds[1];
    }
    status &= (::pipe(fds) != -1);
    if (status) {
        out.first = fds[0];
        out.second = fds[1];
    }
}

std::pair<int, int> fifo::get_in() {
    return {*in.first, *in.second};
}

std::pair<int, int> fifo::get_out() {
    return {*out.first, *out.second};
}

bool fifo::valid() {
    return status;
}
