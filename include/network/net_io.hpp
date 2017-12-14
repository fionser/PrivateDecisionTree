#ifndef PRIVATE_DECISION_TREE_NETWORK_NET_IO_HPP
#define PRIVATE_DECISION_TREE_NETWORK_NET_IO_HPP
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <functional>
#include <iostream>

using boost::asio::ip::tcp;
using routine_t = std::function<void(tcp::iostream &)>;

namespace network {
    extern int port;
    extern std::string addr;
};

class FHEcontext;
FHEcontext receive_context(std::istream &s);

void send_context(std::ostream &s, FHEcontext const& context);

int run_server(routine_t server_routine);

int run_client(routine_t client_routine);
#endif // PRIVATE_DECISION_TREE_NETWORK_NET_IO_HPP
