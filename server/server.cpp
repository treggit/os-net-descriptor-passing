//
// Created by Anton Shelepov on 2019-05-17.
//

#include "server.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <vector>
#include <sys/types.h>
#include <cstring>
#include "../fifo/fifo.h"
#include "../file_descriptor/file_descriptor.h"
#include <sys/un.h>
#include <fcntl.h>
#include "../utils/utils.h"

server_exception::server_exception(std::string const& msg, bool call_strerror)
        : std::runtime_error(call_strerror ? (msg + ": " + strerror(errno)) : msg) {}

void server::log(const std::string& msg) {
    std::cerr << "server: " + msg << std::endl;
}

server::server(std::string const& path) : socket_fd(socket(AF_UNIX, SOCK_STREAM, 0)) {

    if (!socket_fd.valid()) {
        throw server_exception("Couldn't create socket");
    }

    sockaddr_un addr = {0};

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path.c_str());

    if (bind(*socket_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
        throw server_exception("Couldn't bind server to the socket");
    }

    if (listen(*socket_fd, BACKLOG_QUEUE_SIZE) == -1) {
        throw server_exception("Couldn't start listening to the socket");
    }


    log("Server was deployed successfully");
}

void server::send_fd(int client, int fd) {
    msghdr msg = {};
    char buf[CMSG_SPACE(sizeof(fd))];
    memset(buf, 0, sizeof(buf));

    iovec io{};
    io.iov_base = (void *) "";
    io.iov_len = 1;

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(fd));

    memmove(CMSG_DATA(cmsg), &fd, sizeof(fd));

    msg.msg_controllen = cmsg->cmsg_len;

    if (sendmsg(client, &msg, 0) == -1) {
        throw server_exception("Couldn't pass file descriptor");
    }
}

void server::run() {
    log("Waiting for connections");

    while (true) {
        file_descriptor client_socket = accept(*socket_fd, nullptr, nullptr);

        log("Client connected, descriptor " + std::to_string(*client_socket));
        try {
            handle_connection(*client_socket);
        } catch (server_exception& e) {
            log(e.what());
        }
    }
}

void server::handle_connection(int client_desc) {
    log("Establishing FIFO...");
    fifo channel;
    if (!channel.valid()) {
        throw server_exception("Couldn't create fifo channel", false);
    }

    send_fd(client_desc, channel.get_out().first);
    send_fd(client_desc, channel.get_in().second);

    log("Waiting for requests");

    while (true) {
        send(channel.get_out().second, read(channel.get_in().first));
    }
}

std::string server::read(int desc) {
    std::string message = utils::read(desc);
    if (message.length() == 0) {
        throw server_exception("Client disconnected", false);
    }
    log("Read message successfully: " + message);
    return message;
}

void server::send(int desc, std::string const& message) {
    log("Sending message " + message + "...");
    size_t was_sent = utils::send(desc, message);

    if (was_sent == 0) {
        throw server_exception("Client disconnected", false);
    }
    if (was_sent != message.size()) {
        log("Not full message was sent");
    } else {
        log("Send message successfully");
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Expected unix socket pathname" << std::endl;
        return 0;
    }
    std::string socket_path = argv[1];

    try {
        server serv(socket_path);
        serv.run();
    } catch (server_exception& e) {
        std::cout << e.what() << std::endl;
    }
}