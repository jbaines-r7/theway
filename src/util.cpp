#include "util.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>

namespace theway
{
    /**
     * Load an entire string into memory.
     * @param[in] p_file the file path to load
     * @return the file's data
     */
    std::string load_file(const std::string& p_file)
    {
        std::ifstream inputFile(p_file, std::ifstream::in | std::ifstream::binary);
        if (!inputFile.is_open() || !inputFile.good())
        {
            std::cerr << "Failed to ropen the provided file: " << p_file << std::endl;
            return std::string();
        }

        std::string input((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
        inputFile.close();
        return input;
    }

    /**
     * Write a string as data as hex to stdout
     * @param[in] p_data the data to write
     * @param[in] p_length the amount of the data to write
     */
    void print_hex(const std::string& p_data, std::size_t p_length)
    {
        if (p_data.size() < p_length)
        {
            std::cerr << "Length error!" << std::endl;
            return;
        }

        for (std::size_t i = 0; i < p_length; i++)
        {
            std::cout << std::hex << std::setfill('0') << std::setw(2) << ((int)p_data[i] & 0xff);
        }
        std::cout << std::endl;
    }
}