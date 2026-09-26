/**
 * @file set.cpp
 * @brief devkit set <depot> : ajoute une dependance, ou modifie la sienne.
 *
 * N'ecrit que la configuration. C'est devkit build qui en tire le
 * cmake/Find<Depot>.cmake et les blocs du CMakeLists.
 */

#include "Assets.hpp"
#include "cmd.hpp"

#include <yaml-cpp/yaml.h>

#include <iostream>

int set(const cli::Call &call) {
    if (!std::filesystem::exists(configuration)) {
        std::cerr << "aucun " << configuration.string() << " ici, lance devkit init" << std::endl;
        return 1;
    }

    const std::string name = call.arguments.front();
    const std::string tag = call.value("version").empty() ? "main" : call.value("version");
    //une dependance est liee par defaut ; --shared la fait copier a cote du binaire
    const std::string linkage = call.has("shared") ? "shared" : "static";

    if (call.value("version").empty())
        std::cerr << "sans -v, " << name << " suivra la branche par defaut : "
                  << "deux compilations peuvent differer" << std::endl;

    bool known = false;

    try {
        YAML::Node config = YAML::LoadFile(configuration.string());
        YAML::Node repositories = config["repositories"];

        for (YAML::Node repository : repositories)
            if (repository["name"].as<std::string>("") == name) {
                repository["tag"] = tag;
                repository["linkage"] = linkage;
                known = true;
            }

        if (!known) {
            YAML::Node repository;

            repository["name"] = name;
            repository["tag"] = tag;
            repository["linkage"] = linkage;
            repository.SetStyle(YAML::EmitterStyle::Block);
            repositories.push_back(repository);
        }

        //sans ca, la liste resterait au style "flot" herite du [] de depart
        repositories.SetStyle(YAML::EmitterStyle::Block);
        config["repositories"] = repositories;

        YAML::Emitter out;

        out.SetIndent(4);
        out << config;
        assets::write(configuration, std::string(out.c_str()) + "\n");
    } catch (const YAML::Exception &error) {
        std::cerr << configuration.string() << " illisible : " << error.what() << std::endl;
        return 1;
    }

    std::cout << name << " " << tag << " (" << linkage << ") "
          << (known ? "modifie" : "ajoute") << ", lance devkit build" << std::endl;
    return 0;
}
