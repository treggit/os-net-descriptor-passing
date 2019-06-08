//
// Created by Anton Shelepov on 2019-05-17.
//

#ifndef OS_NET_SERVER_H
#define OS_NET_SERVER_H

#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <exception>
#include <string>
#include "../fifo/fifo.h"
#include "../file_descriptor/file_descriptor.h"
#include <vector>
#include <unordered_map>

class server {

public:
    explicit server(std::string const& path);
    void log(std::string const& msg);
    void run();

private:
    std::string read(int desc);
    void send(int desc, std::string const& message);
    void handle_connection(int client_desc);

    file_descriptor socket_fd;

    static const int BACKLOG_QUEUE_SIZE = 50;
    static const size_t BUFFER_SIZE = 10 * 4096;

    const std::string FIFO_NAME = "/Users/treggit/fifo";
};


struct server_exception : std::runtime_error {
    explicit server_exception(std::string const& msg, bool call_strerror = true);
};


#endif //OS_NET_SERVER_H