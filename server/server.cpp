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
    fifo channel(FIFO_NAME);
    if (!channel.valid()) {
        throw server_exception("Couldn't create fifo channel");
    }

    send(client_desc, channel.get_in_name() + "\n" + channel.get_out_name());

    if (!channel.open()) {
        throw server_exception("Couldn't open fifo channel");
    }

    int in = channel.get_in_descriptor();
    int out = channel.get_out_descriptor();

    log("Waiting for requests");

    while (true) {
        send(out, read(in));
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