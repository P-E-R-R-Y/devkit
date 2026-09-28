/**
 * @file clean.cpp
 * @brief devkit clean: empties build/ without removing the folder.
 *
 * The folder survives because tools open it before anything is rebuilt
 * inside.
 */

#include "cmd.hpp"

#include <iostream>

int clean(const cli::Call &) {
    const std::filesystem::path folder = "build";

    if (!std::filesystem::exists(folder)) {
        std::cout << "nothing to clean" << std::endl;
        return 0;
    }

    std::uintmax_t erased = 0;

    try {
        for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(folder))
            erased += std::filesystem::remove_all(entry.path());
    } catch (const std::filesystem::filesystem_error &error) {
        std::cerr << "cannot clean: " << error.what() << std::endl;
        return 1;
    }
    std::cout << erased << " entry(ies) erased, " << folder.string() << " kept" << std::endl;
    return 0;
}
