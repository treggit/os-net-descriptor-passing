//
// Created by Anton Shelepov on 2019-05-31.
//

#ifndef OS_NET_FIFO_H
#define OS_NET_FIFO_H

#include <sys/stat.h>
#include <string>
#include <sys/types.h>

struct fifo {
    explicit fifo(std::string const& pathname);
    ~fifo();

    bool valid() const;
    int get_in_descriptor();
    int get_out_descriptor();
    std::string get_in_name();
    std::string get_out_name();
    bool open();

    static const mode_t MODE = S_IRUSR | S_IWUSR | S_IWGRP;

private:
    std::string in_name, out_name;
    int in_fd = -1;
    int out_fd = -1;

    bool state = true;
};


#endif //OS_NET_FIFO_H
