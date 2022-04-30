#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>

namespace theway
{
    std::string load_file(const std::string& p_file);
    void print_hex(const std::string& p_data, std::size_t p_length);
}
#endif