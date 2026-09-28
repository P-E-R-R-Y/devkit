/**
 * @file clean.cpp
 * @brief devkit clean : vide build/ sans enlever le dossier.
 *
 * Le dossier survit parce qu'il est suivi par le .gitignore et que des
 * outils l'ouvrent avant qu'on y reconstruise quoi que ce soit.
 */

#include "cmd.hpp"

#include <iostream>

int clean(const cli::Call &) {
    const std::filesystem::path folder = "build";

    if (!std::filesystem::exists(folder)) {
        std::cout << "rien a nettoyer" << std::endl;
        return 0;
    }

    std::uintmax_t erased = 0;

    try {
        for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(folder))
            erased += std::filesystem::remove_all(entry.path());
    } catch (const std::filesystem::filesystem_error &error) {
        std::cerr << "nettoyage impossible : " << error.what() << std::endl;
        return 1;
    }
    std::cout << erased << " entree(s) effacee(s), " << folder.string() << " conserve" << std::endl;
    return 0;
}
