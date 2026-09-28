/**
 * @file info.cpp
 * @brief devkit info : ce projet en un coup d'oeil.
 */

#include "Config.hpp"
#include "cmd.hpp"

#include <iostream>

int info(const cli::Call &) {
    if (!std::filesystem::exists(configuration)) {
        std::cerr << "aucun " << configuration.string() << " ici, lance devkit init" << std::endl;
        return 1;
    }

    try {
        const devkit::Config config = devkit::Config::load(configuration.string());

        std::cout << config.name << " " << config.version << "\n  ";
        for (const std::string &kind : config.kind)
            std::cout << kind << " ";
        std::cout << " cmake " << config.cmake
                  << ", tests " << (config.tests ? "oui" : "non")
                  << ", docs " << (config.documentation ? "oui" : "non")
                  << ", ci " << (config.cicd ? "oui" : "non") << "\n"
                  << "  " << config.repositories.size() << " dependance(s), "
                  << config.example.names.size() << " bac(s) a sable" << std::endl;
    } catch (const YAML::Exception &error) {
        std::cerr << configuration.string() << " illisible : " << error.what() << std::endl;
        return 1;
    }
    return 0;
}
