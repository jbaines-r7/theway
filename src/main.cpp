#include "popl.hpp"
#include <arpa/inet.h>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <map>
#include <filesystem>

/*
 2272  ./theway -f ~/cisco/asdm-7171.bin -e
 2273  ./theway -f ./output/ -r
 2274  ./theway -f ./test.final.bin -e
 */

namespace
{
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

        for (std::uint32_t i = 0; i < manifest_count; i++)
        {
            std::cout << "-> Entry: " << i << std::endl;

            std::string entry_header(asdm.data(), asdm.data() + 0xe);
            print_hex(entry_header, 0xc);
            asdm.erase(0, 0xc);
            std::uint32_t data_offset = *reinterpret_cast<std::uint32_t*>(entry_header.data());
            std::uint32_t data_size = *reinterpret_cast<std::uint32_t*>(entry_header.data() + 4);
            std::uint32_t entry_length = *reinterpret_cast<std::uint32_t*>(entry_header.data() + 8);
            std::cout << "\t-> Entry length: " << entry_length << std::endl;
            std::string filename(asdm.data(), asdm.data() + entry_length);
            std::cout << "\t-> Entry file: " << filename << std::endl;
            std::cout << "\t-> Data offset: " << data_offset << std::endl;
            std::cout << "\t-> Data size: " << data_size << std::endl;
            asdm.erase(0, entry_length);

            entries[filename] = std::make_pair(data_offset, data_size);
        }

        system("rm -rf ./output");
        std::filesystem::create_directory("output");

        for (std::map<std::string, std::pair<std::uint32_t, std::uint32_t> >::iterator iter = entries.begin();
            iter != entries.end(); ++iter)
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
        std::cout << "-> Files to package " << std::endl;
        for(auto it = std::filesystem::directory_iterator(p_input_file); it != std::filesystem::directory_iterator(); ++it)
        {
            entry_count++;

            // 1 byte 0, 4 bytes offset, 4 bytes length, 4 bytes name length
            std::string name((*it).path().string());
            name.erase(0, p_input_file.size());

            // replace safe character
            while (name.find("%") != std::string::npos)
            {
                std::size_t index = name.find("%");
                name.replace(index, 1, "/");
            }

            std::cout << "\t-> " << name << std::endl;

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
        for(auto it = std::filesystem::directory_iterator(p_input_file); it != std::filesystem::directory_iterator(); ++it)
        {
              std::string name((*it).path().string());
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

              // pad on 4 byte boundary
              while ((name.size() % 4) != 0)
              {
                  // the +1 logic is fucked. likely i fucked up the struct
                  name.push_back('\x00');
              }

              std::uint32_t length = file_data.size();
              entries[name].m_data_length = length;
              entries[name].m_data_offset = data_offset;
              std::cout << "\t->Offset (" << name << ")" << data_offset << std::endl;
              data_offset += file_data.size();
              data.append(file_data.data(), file_data.size());
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

int main(int p_argc, char** p_argv)
{
    popl::OptionParser op("Allowed options");
    auto help_option = op.add<popl::Switch>("h", "help", "produce help message");
    auto asdm_file = op.add<popl::Value<std::string>, popl::Attribute::required>("f", "file", "The file to parse");
    auto extract = op.add<popl::Switch>("e", "extract", "Extract the input");
    auto repackage = op.add<popl::Switch>("r", "repackage", "Repackage the input");

    try
    {
        op.parse(p_argc, p_argv);
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
        std::cout << op << std::endl;
        return EXIT_FAILURE;
    }

    if (help_option->is_set())
    {
        std::cout << op << std::endl;
        return EXIT_SUCCESS;
    }

    if (extract->is_set())
    {
        do_extract(asdm_file->value());
    }
    else if (repackage->is_set())
    {
        do_repackage(asdm_file->value());
    }
    else
    {
        std::cout << "[?] But where were they going without even knowing the way." << std::endl;
    }

    return EXIT_FAILURE;
}