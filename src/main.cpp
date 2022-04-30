#include "popl.hpp"
#include "extract.hpp"
#include "repackage.hpp"

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
    void print_art()
    {
        std::cout << "        .       .        ." << std::endl;
        std::cout << "         .      .       .        .'               .--." << std::endl;
        std::cout << " '.       .     .      .       .'       ________.'_.'_____.--.___" << std::endl;
        std::cout << "   '.      .    .     .      .'        ()_() ooo   ~/   -.|      \"-._" << std::endl;
        std::cout << "     '.     .   .    .     .'          ((((<____   |      |  ____  = )" << std::endl;
        std::cout << "       '.    .  .   .    .'       .-'  (__)/ () \\___\\_____|_/ () \\__/)" << std::endl;
        std::cout << ".        '   ______    .'      .-'    ___'.'.__.'_________'.'.__.'____" << std::endl;
        std::cout << " '-.      .-~      ~-.      .-'      /" << std::endl;
        std::cout << "    '-. .'            '. .-'      __/" << std::endl;
        std::cout << "_      .                .      _./" << std::endl;
        std::cout << " '-._ .                  . _.-'  |    And when the car broke down" << std::endl;
        std::cout << "      :                  :      /     They started walking" << std::endl;
        std::cout << "  .-' .                  . '- .'      But where were they going" << std::endl;
        std::cout << "-'     .                . . .'        without even knowing \033[0;31mthe way\033[0m" << std::endl;
        std::cout << "    .'  '.            .' _.'" << std::endl;
        std::cout << "  .'  .'  '__________'.-'             ASDM Malicious Package Tool" << std::endl;
        std::cout << "~~~~~~~~~~~~~~~~~~~~~/                CVE-2022-8888" << std::endl;
        std::cout << "~~  ~~~    ~~~ ~~ ~ /                 Art by JRO" << std::endl;
        std::cout << "  ~~   ~~~~  ~~~~ ~/                  Hax by \033[0;31mjbaines 🦞\033[0m" << std::endl;
        std::cout << std::endl;
    }
}

int main(int p_argc, char** p_argv)
{
    popl::OptionParser op("Allowed options");
    auto help_option = op.add<popl::Switch>("h", "help", "Produces this help message");
    auto asdm_file = op.add<popl::Value<std::string>, popl::Attribute::required>("i", "input", "The file to parse or directory to extract");
    auto extract = op.add<popl::Switch>("e", "extract", "Extract the input");
    auto repackage = op.add<popl::Switch>("r", "repackage", "Repackage the input");
    auto generate = op.add<popl::Switch>("g", "generate", "Generates a malicious ASM package");
    auto noart = op.add<popl::Switch>("n", "noart", "Don't display the ascii art");

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

    if (!noart->is_set())
    {
        print_art();
    }

    if (help_option->is_set())
    {
        std::cout << op << std::endl;
        return EXIT_SUCCESS;
    }

    if (extract->is_set())
    {
        theway::do_extract(asdm_file->value());
    }
    else if (repackage->is_set())
    {
        //todo asdm_file should end with '/'
        theway::do_repackage(asdm_file->value());
    }
    else if (generate->is_set())
    {
        // hi
    }
    else
    {
        std::cerr << "[!] Please select -e, -r, or -g." << std::endl;
    }

    return EXIT_FAILURE;
}