//
// Created by Anton Shelepov on 2019-05-17.
//

#ifndef OS_NET_CLIENT_H
#define OS_NET_CLIENT_H

#include "../file_descriptor/file_descriptor.h"
#include "../fifo/fifo.h"
#include <string>
#include <exception>
#include <netdb.h>
#include <stdexcept>

class client {
public:
    client();
    void establish_connection(std::string const& path);
    void log(std::string const& msg);
    void run();

private:
    std::string read(int desc);
    void send(int desc, std::string const& message);
    int get_fd();

    file_descriptor socket_fd = -1;

    const size_t BUFFER_SIZE = 10 * 4096;
    const size_t TRIES_NUMBER = 10;
    static const int MAX_EVENTS = 5;

    bool alive = true;

};

struct client_exception : std::runtime_error {
    explicit client_exception(std::string const& msg, bool call_strerror = true);
};


#endif //OS_NET_CLIENT_H