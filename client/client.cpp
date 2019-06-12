//
// Created by Anton Shelepov on 2019-05-17.
//

#include "client.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sys/un.h>
#include "../fifo/fifo.h"
#include <fcntl.h>
#include "../utils/utils.h"

client_exception::client_exception(std::string const& msg, bool call_strerror)
        : std::runtime_error(call_strerror ? (msg + ": " + strerror(errno)) : msg) {}

client::client() : socket_fd(socket(AF_UNIX, SOCK_STREAM, 0)) {
    if (!socket_fd.valid()) {
        throw client_exception("Couldn't create socket");
    }
    log("Client was deployed successfully");
}

void client::establish_connection(std::string const& path) {
    sockaddr_un server_addr;

    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, path.c_str());

    if (connect(*socket_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(sockaddr_un)) == -1) {
        throw client_exception("Couldn't connect to the server");
    }

    log("Connected to server via given socket");
}


void client::log(std::string const& msg) {
    std::cerr << "client: " << msg << std::endl;
}

int client::get_fd() {
    msghdr msg{};

    char buffer[512];
    iovec io{};
    io.iov_base = buffer;
    io.iov_len = sizeof(buffer);

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;

    char msg_control_buf[512];
    msg.msg_control = msg_control_buf;
    msg.msg_controllen = sizeof(msg_control_buf);

    if (recvmsg(*socket_fd, &msg, 0) == -1) {
        throw client_exception("Couldn't receive file descriptor");
    }

    cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);

    int fd;
    memcpy(&fd, CMSG_DATA(cmsg), sizeof(fd));

    return fd;
}

void client::run() {
    alive = true;

    int in = get_fd();
    int out = get_fd();

    while (alive) {
        std::string request;
        getline(std::cin, request);

        if(!std::cin || request == "exit") {
            alive = false;
            break;
        }

        if (!request.empty()) {
            send(out, request);
            std::cout << read(in) << std::endl;
        }
    }
}


std::string client::read(int desc) {
    std::string message = utils::read(desc);
    if (message.size() == 0) {
        alive = false;
        throw client_exception("Failed to receive full message");
    }

    log("Read message successfully: " + message);
    return message;
}

void client::send(int desc, std::string const& message) {
    log("Sending message " + message + "...");
    size_t was_sent = utils::send(desc, message);

    if (was_sent == 0) {
        alive = false;
        throw client_exception("Failed to send request");
    }
    if (was_sent != message.size()) {
        log("Not full message was sent");
    } else {
        log("Send message successfully");
    }
}

int main(int argc, char* argv[]) {
    std::cout << "Simple echo client. Server will echo your requests. Type \"exit\" request to exit" << std::endl;

    if (argc < 2) {
        std::cout << "Expected unix socket pathname" << std::endl;
        return 0;
    }
    std::string socket_path = argv[1];
    try {
        client cli;
        cli.establish_connection(socket_path);
        cli.run();
    } catch (client_exception& e) {
        std::cout << e.what() << std::endl;
    }
}