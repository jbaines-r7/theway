#include "repackage.hpp"
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
#include <vector>

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

    bool do_repackage(const std::string& p_input_file)
    {
        if (std::filesystem::is_directory(p_input_file) == false)
        {
            std::cerr << "Provided file is not a directory" << std::endl;
            return false;
        }

        std::string temp_entries;
        std::map<std::string, manifest_entry> entries;

        std::uint32_t entry_count = 0;
        std::cout << "-> Sizeof manifest entry " << std::hex << (int)sizeof(manifest_entry) << std::endl;

        // the files are written in, mostly, alphabetical order
        std::vector<std::filesystem::path> files_in_directory;
        std::copy(std::filesystem::directory_iterator(p_input_file), std::filesystem::directory_iterator(), std::back_inserter(files_in_directory));
        std::sort(files_in_directory.begin(), files_in_directory.end());

        // loop over the files to write, correcting filenames, adding padding and initializing the map
        std::cout << "-> Files to package " << std::endl;
        for (const std::filesystem::path& filename : files_in_directory)
        {
            entry_count++;

            // 1 byte 0, 4 bytes offset, 4 bytes length, 4 bytes name length
            std::string name(filename.string());
            name.erase(0, p_input_file.size());

            // replace safe character
            while (name.find("%") != std::string::npos)
            {
                std::size_t index = name.find("%");
                name.replace(index, 1, "/");
            }

            std::cout << "\t-> " << name << std::endl;

            if ((name.size() % 0x10) == 0|| name.size() > 0x10)
            {
                // wat
                name.push_back('\x00');
            }

            if (name == "version.prop")
            {
                name.push_back('\x00');
            }

            // pad on 4 byte boundary
            while ((name.size() % 4) != 0)
            {
                // the +1 logic is fucked. likely i fucked up the struct
                name.push_back('\x00');
            }
            while (name.find("@") != std::string::npos)
            {
                std::size_t index = name.find("@");
                name.replace(index, 1, ".");
            }

            memset(&entries[name], 0, sizeof(manifest_entry));
            std::cout << name.size() << std::endl;
            std::cout << name << std::endl;
            entries[name].m_name_length = name.size();
            temp_entries.append(reinterpret_cast<char*>(&entries[name]), sizeof(manifest_entry));
            temp_entries.append(name);
        }

        std::cout << "-> Entries size: " << temp_entries.size() << std::endl;

        char header[0x70] = { 0 };
        memset(header, 0, sizeof(header));
        memcpy(header, "ASDM IMG7.17(1)", 15);
        memcpy(header + 0x1c, "Device Manager Version 7.17(1)", 0x1e);

        char hash[0x10] = { 0 };
        memset(hash, 0, sizeof(hash));

        std::string compilation_date("Wed, 24 Nov 2021 08:12:33 GMT");
        compilation_date.push_back('\x00');
        compilation_date.push_back('\x00');
        compilation_date.push_back('\x00');

        std::uint32_t data_offset = sizeof(header) + sizeof(hash) + compilation_date.size() + sizeof(entry_count) + temp_entries.size();

        std::string data;
        std::string asdm_entry;
        for (const std::filesystem::path& filename : files_in_directory)
        {
            std::string name(filename.string());
            std::string file_data(load_file(name));
            if (file_data.empty())
            {
                std::cerr << "Failed to load a  file for packing" << std::endl;
                return false;
            }

            name.erase(0, p_input_file.size());

            // replace safe character
            while (name.find("%") != std::string::npos)
            {
                std::size_t index = name.find("%");
                name.replace(index, 1, "/");
            }
            while (name.find("@") != std::string::npos)
            {
                std::size_t index = name.find("@");
                name.replace(index, 1, ".");
            }

            if ((name.size() % 0x10) == 0 || name.size() > 0x10)
            {
                // wat
                name.push_back('\x00');
            }

            if (name == "version.prop")
            {
                name.push_back('\x00');
            }

            // pad on 4 byte boundary
            while ((name.size() % 4) != 0)
            {
                // the +1 logic is fucked. likely i fucked up the struct
                name.push_back('\x00');
            }

            std::uint32_t length = file_data.size();
            entries[name].m_data_length = length;
            entries[name].m_data_offset = data_offset;
            std::cout << "\t->Offset (" << name << ") (" << data_offset << " -> " << (data_offset + file_data.size()) << ")" << std::endl;
            
            if (name.starts_with("dm-launcher.msi"))
            {
                entries[asdm_entry].m_data_offset = data_offset;
            }
            if (!name.starts_with("asdm50-install.msi"))
            {
                data_offset += file_data.size();
                data.append(file_data.data(), file_data.size());
                while ((data.size() % 4) != 0)
                {
                    data.push_back('\x00');
                    ++data_offset;
                }
            }
            else
            {
                asdm_entry.assign(name);
            }
        }

        // compute total length
        std::uint32_t total_length = sizeof(header) + sizeof(hash) + compilation_date.size() + sizeof(entry_count) + temp_entries.size() + data.size();
        memcpy(header + 0x6c, &total_length, sizeof(uint32_t));

        // dump to disk
        std::fstream out_file;
        out_file.open("test.bin", std::ios::out | std::ios::binary);
        out_file.write(&header[0], sizeof(header));
        out_file.write(&hash[0], sizeof(hash));
        out_file.write(compilation_date.data(), compilation_date.size());
        out_file.write(reinterpret_cast<char*>(&entry_count), sizeof(std::uint32_t));
        for (std::map<std::string, manifest_entry>::iterator it = entries.begin(); it != entries.end(); ++it)
        {
            out_file.write(reinterpret_cast<char*>(&(it->second.m_data_offset)), sizeof(std::uint32_t));
            out_file.write(reinterpret_cast<char*>(&(it->second.m_data_length)), sizeof(std::uint32_t));
            out_file.write(reinterpret_cast<char*>(&(it->second.m_name_length)), sizeof(std::uint32_t));
            out_file.write(it->first.data(), it->first.size());
        }
        out_file.write(data.data(), data.size());
        out_file.close();

        system("python3 ../compute_hash.py -i test.bin -o test.final.bin");

        return true;
    }
}