//
// Created by Anton Shelepov on 2019-05-17.
//

#ifndef OS_NET_SOCKET_DESCRIPTOR_H
#define OS_NET_SOCKET_DESCRIPTOR_H

#include <unistd.h>

struct file_descriptor {
    file_descriptor();
    file_descriptor(int descriptor);
    file_descriptor(file_descriptor&& other) noexcept;
    file_descriptor& operator=(file_descriptor&& other) noexcept;
    bool valid() const;
    int operator*() const;
    ~file_descriptor();


private:
    int descriptor = -1;
};


#endif //OS_NET_SOCKET_DESCRIPTOR_H