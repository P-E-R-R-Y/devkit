/**
 * @file info.cpp
 * @brief devkit info: this project at a glance.
 */

#include "Config.hpp"
#include "cmd.hpp"

#include <iostream>

int info(const cli::Call &) {
    if (!std::filesystem::exists(configuration)) {
        std::cerr << "no " << configuration.string() << " here, run devkit init" << std::endl;
        return 1;
    }

    try {
        const devkit::Config config = devkit::Config::load(configuration.string());

        std::cout << config.name << " " << config.version << "\n  ";
        for (const std::string &kind : config.kind)
            std::cout << kind << " ";
        std::cout << " cmake " << config.cmake
                  << ", tests " << (config.tests ? "yes" : "no")
                  << ", docs " << (config.documentation ? "yes" : "no")
                  << ", ci " << (config.cicd ? "yes" : "no") << "\n"
                  << "  " << config.repositories.size() << " dependency(ies), "
                  << config.example.names.size() << " sandbox(es)" << std::endl;
    } catch (const YAML::Exception &error) {
        std::cerr << configuration.string() << " unreadable: " << error.what() << std::endl;
        return 1;
    }
    return 0;
}
