#include "network/net_io.hpp"
#include <HElib/FHEContext.h>

FHEcontext receive_context(std::istream &s) {
    unsigned long m, p, r;
    std::vector<long> gens, ords;
    readContextBase(s, m, p, r, gens, ords);
    FHEcontext context(m, p, r, gens, ords);
    NTL::zz_p::init(p);
    s >> context;
    return context;
}

void send_context(std::ostream &s, FHEcontext const& context) {
    writeContextBase(s, context);
    s << context;
}

int run_server(routine_t server_routine) {
    boost::asio::io_service ios;
    tcp::endpoint endpoint(tcp::v4(), network::port);
    tcp::acceptor acceptor(ios, endpoint);
    for (;;) {
        tcp::iostream conn;
        boost::system::error_code err;
        acceptor.accept(*conn.rdbuf(), err);
        if (!err) {
            server_routine(conn);
            conn.close();
            break;
        }
    }  
    return 0;
}

int run_client(routine_t client_routine) {
    tcp::iostream conn(network::addr, std::to_string(network::port));
    if (!conn) {
        std::cerr << "Can not connect to server!" << std::endl;
        return -1;
    }
    client_routine(conn);
    conn.close();
    return 1;
}

