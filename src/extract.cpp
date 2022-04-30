#include "extract.hpp"
#include "util.hpp"

#include <arpa/inet.h>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <map>
#include <set>
#include <filesystem>
#include <cstring>
#include <fstream>

namespace theway
{
    #pragma pack(push, 1)
    struct manifest_entry
    {
        std::uint32_t m_data_offset;
        std::uint32_t m_data_length;
        std::uint32_t m_name_length;
    };
    #pragma pack(pop)

    bool do_extract(const std::string& p_input_file)
    {
        std::cout << "[+] Loading " << p_input_file << std::endl;
        std::string asdm(load_file(p_input_file));
        if (asdm.empty())
        {
            std::cerr << "Failed to load " << p_input_file << std::endl;
            return false;
        }

        // we'll come back to this
        std::string asdm_original(asdm);

        std::string header(asdm.begin(), asdm.begin() + 0x80);
        std::string magic(header.data(), strlen(header.data()));
        std::string desc_maybe(header.data() + 0x1c, strlen(header.data() + 0x1c));
        std::uint32_t length = *reinterpret_cast<std::uint32_t*>(header.data() + 0x6c);
        std::string md5hash(header.begin() + 0x70, header.begin() + 0x80);
        std::cout << "-> Magic: " << magic << std::endl;
        std::cout << "-> Description: " << desc_maybe << std::endl;
        std::cout << "-> File length: " << std::hex << length << std::endl;
        std::cout << "-> File hash: ";
        print_hex(md5hash, 0x10);

        // verify length
        if (length != asdm.size())
        {
            std::cerr << "[-] Length field doesn't match the file size" << std::endl;
            return false;
        }

        asdm.erase(0, 0x80);
        std::string compilation_date(asdm.data(), asdm.data() + 0x20);
        std::cout << "-> Compilation date: " << compilation_date << std::endl;
        asdm.erase(0, 0x20);

        std::uint32_t manifest_count = *reinterpret_cast<std::uint32_t*>(asdm.data());
        std::cout << "-> Manifest Entries: " << std::hex << manifest_count << std::endl;
        asdm.erase(0, 4);

        std::map<std::string, std::pair<std::uint32_t, std::uint32_t> > entries;
        std::set<std::uint32_t> offsets;

        for (std::uint32_t i = 0; i < manifest_count; i++)
        {
            std::cout << "-> Entry: " << i << std::endl;

            std::string entry_header(asdm.data(), asdm.data() + 0xe);
            print_hex(entry_header, 0xc);
            asdm.erase(0, 0xc);
            std::uint32_t data_offset = *reinterpret_cast<std::uint32_t*>(entry_header.data());
            std::uint32_t data_size = *reinterpret_cast<std::uint32_t*>(entry_header.data() + 4);
            std::uint32_t entry_length = *reinterpret_cast<std::uint32_t*>(entry_header.data() + 8);

            bool duplicate = false;
            if (offsets.find(data_offset) == offsets.end())
            {
                offsets.insert(data_offset);
            }
            else
            {
                duplicate = true;
            }

            std::cout << "\t-> Entry length: " << entry_length << std::endl;
            std::string filename(asdm.data(), asdm.data() + entry_length);
            std::cout << "\t-> Entry file: " << filename << std::endl;
            std::cout << "\t-> Data offset: " << data_offset << std::endl;
            std::cout << "\t-> Data size: " << data_size << std::endl;
            std::cout << "\t-> Duplicate: " << duplicate << std::endl;
            asdm.erase(0, entry_length);

            entries[filename] = std::make_pair(data_offset, data_size);
        }

        system("rm -rf ./output");
        std::filesystem::create_directory("output");

        for (std::map<std::string, std::pair<std::uint32_t, std::uint32_t> >::iterator iter = entries.begin(); iter != entries.end(); ++iter)
        {
            std::string dirty_name(iter->first);
            while (dirty_name.find("/") != std::string::npos)
            {
                std::size_t index = dirty_name.find("/");
                dirty_name.replace(index, 1, "%");
            }
            std::string out_name("output/" + dirty_name);
            std::fstream entry_file;
            entry_file.open(out_name, std::ios::out | std::ios::binary);
            entry_file.write(asdm_original.data() + iter->second.first, iter->second.second);
            entry_file.close();

        }
        return true;
    }
}