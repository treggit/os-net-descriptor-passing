//
// Created by Anton Shelepov on 2019-05-31.
//

#include "fifo.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>

fifo::fifo(std::string const& pathname) : in_name(pathname + ".in"), out_name(pathname + ".out") {
    state &= (mkfifo(in_name.c_str(), MODE) != -1 || errno == EEXIST);
    state &= (mkfifo(out_name.c_str(), MODE) != -1 || errno == EEXIST);
}

fifo::~fifo() {
    if (in_fd != -1) {
        close(in_fd);
    }

    if (out_fd != -1) {
        close(out_fd);
    }

    unlink(in_name.c_str());
    unlink(out_name.c_str());
}

bool fifo::open() {
    if (in_fd == -1) {
        in_fd = ::open(in_name.c_str(), O_RDONLY);
    }

    if (out_fd == -1) {
        out_fd = ::open(out_name.c_str(), O_WRONLY);
    }

    return (in_fd != -1 && out_fd != -1);
}

bool fifo::valid() const {
    return state;
}

int fifo::get_in_descriptor() {
    open();
    return in_fd;
}

int fifo::get_out_descriptor() {
    open();
    return out_fd;
}

std::string fifo::get_in_name() {
    return in_name;
}

std::string fifo::get_out_name() {
    return out_name;
}
