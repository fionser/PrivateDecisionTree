#ifndef PRIVATE_DECISION_TREEE_PPDT_HPP
#define PRIVATE_DECISION_TREEE_PPDT_HPP
#include "net_io.hpp"

#include <string>
#include <memory>

class PPDTServer {
public:
    PPDTServer() {}

    ~PPDTServer() {}

    bool load(std::string const& file);

    void run(tcp::iostream &conn) ;

private:
    struct Imp;
    std::shared_ptr<Imp> imp_;
};

class PPDTClient {
public:
    PPDTClient() {}

    ~PPDTClient() {}

    bool load(std::string const& file);

    void run(tcp::iostream &conn);

private:
    struct Imp;
    std::shared_ptr<Imp> imp_;
};
#endif // PRIVATE_DECISION_TREEE_PPDT_HPP
