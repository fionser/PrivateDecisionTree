#ifndef PRIVATE_DECISION_TREE_UTIL_LITERALL_HPP
#define PRIVATE_DECISION_TREE_UTIL_LITERALL_HPP
#include <vector>
#include <boost/algorithm/string.hpp>

namespace util {
std::string trim(const std::string &line) {
    size_t first = line.find_first_not_of(' ');
    if (first == std::string::npos)
        return line;
    size_t last = line.find_last_not_of(' ');
    return line.substr(first, last - first + 1);
}

std::vector<std::string> split_by(std::string const& str, char delimiter) {
    std::vector<std::string> fields;
    boost::split(fields, str, [delimiter](char c) -> bool { return c == delimiter; });
    return fields;
}

std::vector<std::string> split_by_space(std::string const& str) {
    return split_by(str, ' ');
}

}
#endif // PRIVATE_DECISION_TREE_UTIL_LITERALL_HPP
