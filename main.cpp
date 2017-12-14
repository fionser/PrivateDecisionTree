#include <HElib/NumbTh.h>

#include <iostream>
#include <fstream>
#include <functional>

#include "network/PPDT.hpp"

int play_server(std::string const& file) {
    PPDTServer server;
    if (!server.load(file)) {
        std::cerr << "Error happened when to load file: " << file << std::endl;
        return -1;
    } else {
        auto server_routine = std::bind(&PPDTServer::run, server, std::placeholders::_1);
        return run_server(server_routine);
    }
}

int play_client(std::string const& file) {
    PPDTClient client;
    if (!client.load(file)) {
        std::cerr << "Error happened when to load file: " << file << std::endl;
        return -1;
    } else {
        auto client_routine = std::bind(&PPDTClient::run, client, std::placeholders::_1);
        return run_client(client_routine);
    }
}

namespace network {
    int port = 12345;
    std::string addr = "127.0.0.1";
}

int main(int argc, char *argv[]) {
    ArgMapping amap;
    long role = 0;
    std::string input_file = "";
    amap.arg("r", role, "role");
    amap.arg("i", input_file, "server model or client's input");
    amap.arg("p", network::port, "port");
    amap.arg("a", network::addr, "server addr");
    amap.parse(argc, argv);

    if (role == 0) {
        play_client(input_file);
    } else if (role == 1) {
        play_server(input_file);
    } else {
        amap.usage("Private Decision Tree");
    }
    return 0;
}
