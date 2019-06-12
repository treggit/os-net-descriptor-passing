//
// Created by Anton Shelepov on 2019-05-17.
//

#include "file_descriptor.h"
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

file_descriptor::file_descriptor() = default;

file_descriptor::file_descriptor(int descriptor) : descriptor(descriptor) {}

file_descriptor::~file_descriptor() {
    if (descriptor != -1) {
        close(descriptor);
    }
}

file_descriptor::file_descriptor(file_descriptor&& other) noexcept : descriptor(*other) {
    other.descriptor = -1;
}

file_descriptor& file_descriptor::operator=(file_descriptor&& other) noexcept {
    descriptor = other.descriptor;
    other.descriptor = -1;

    return *this;
}

int file_descriptor::operator*() const {
    return descriptor;
}

bool file_descriptor::valid() const {
    return descriptor != -1;
}
