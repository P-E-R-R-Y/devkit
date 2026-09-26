/**
 * @file get.cpp
 * @brief devkit get [depot] : affiche la configuration, ou une dependance.
 */

#include "Config.hpp"
#include "cmd.hpp"

#include <iostream>

int get(const cli::Call &call) {
    if (!std::filesystem::exists(configuration)) {
        std::cerr << "aucun " << configuration.string() << " ici, lance devkit init" << std::endl;
        return 1;
    }

    try {
        const devkit::Config config = devkit::Config::load(configuration.string());

        if (!call.arguments.empty()) {
            const std::string wanted = call.arguments.front();

            for (const devkit::Repository &repository : config.repositories)
                if (repository.name == wanted) {
                    std::cout << repository.name << " " << repository.tag
                              << " (" << repository.linkage << ")" << std::endl;
                    return 0;
                }
            std::cerr << wanted << " n'est pas une dependance de ce projet" << std::endl;
            return 1;
        }

        std::cout << config.name << " " << config.version << " - " << config.brief << "\n"
                  << " ";
        for (const std::string &kind : config.kind)
            std::cout << " " << kind;
        std::cout << ", cmake " << config.cmake << "\n"
                  << "  tests " << (config.tests ? "oui" : "non")
                  << ", docs " << (config.documentation ? "oui" : "non")
                  << ", ci " << (config.cicd ? "oui" : "non") << std::endl;

        if (config.repositories.empty())
            std::cout << "  aucune dependance" << std::endl;
        for (const devkit::Repository &repository : config.repositories)
            std::cout << "  " << repository.name << " " << repository.tag
                      << " (" << repository.linkage << ")" << std::endl;
    } catch (const YAML::Exception &error) {
        std::cerr << configuration.string() << " illisible : " << error.what() << std::endl;
        return 1;
    }
    return 0;
}
