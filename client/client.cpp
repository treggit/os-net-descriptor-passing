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

void client::run() {
    alive = true;
    std::string fifo_names = read(*socket_fd);

    file_descriptor out = open(fifo_names.substr(0, fifo_names.find_first_of('\n')).c_str(), O_WRONLY);
    file_descriptor in = open(fifo_names.substr(fifo_names.find_first_of('\n') + 1).c_str(), O_RDONLY);

    if (!in.valid() || !out.valid()) {
        throw client_exception("Couldn't open fifo channel");
    }

    while (alive) {
        std::string request;
        getline(std::cin, request);

        if(!std::cin || request == "exit") {
            alive = false;
            break;
        }

        if (!request.empty()) {
            send(*out, request);
            std::cout << read(*in) << std::endl;
        }
    }
}


std::string client::read(int desc) {
    std::vector<char> buffer(BUFFER_SIZE);
    ssize_t was_read = ::read(desc, buffer.data(), BUFFER_SIZE);

    if (was_read == 0) {
        alive = false;
    }
    if (was_read == -1) {
        throw client_exception("Couldn't read respond from socket " + std::to_string(desc));
    }

    return std::string(buffer.data());
}

void client::send(int desc, std::string const& message) {
    ssize_t was_send = ::write(desc, message.data(), message.size());

    if (was_send == 0) {
        alive = false;
    }
    if (was_send == -1) {
        throw client_exception("Couldn't send request to socket " + std::to_string(desc));
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