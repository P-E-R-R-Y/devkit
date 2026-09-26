/**
 * @file del.cpp
 * @brief devkit del <depot> : retire une dependance de config.yaml.
 */

#include "Assets.hpp"
#include "cmd.hpp"

#include <yaml-cpp/yaml.h>

#include <iostream>

int del(const cli::Call &call) {
    if (!std::filesystem::exists(configuration)) {
        std::cerr << "aucun " << configuration.string() << " ici, lance devkit init" << std::endl;
        return 1;
    }

    const std::string name = call.arguments.front();

    try {
        YAML::Node config = YAML::LoadFile(configuration.string());
        YAML::Node kept(YAML::NodeType::Sequence);
        bool found = false;

        for (const YAML::Node &repository : config["repositories"]) {
            if (repository["name"].as<std::string>("") == name) {
                found = true;
                continue;
            }
            kept.push_back(repository);
        }

        if (!found) {
            std::cerr << name << " n'est pas une dependance de ce projet" << std::endl;
            return 1;
        }

        kept.SetStyle(kept.size() == 0 ? YAML::EmitterStyle::Flow : YAML::EmitterStyle::Block);
        config["repositories"] = kept;

        YAML::Emitter out;

        out.SetIndent(4);
        out << config;
        assets::write(configuration, std::string(out.c_str()) + "\n");

        //le module cmake devient sans objet : create ne le reecrira plus
        std::filesystem::remove(std::filesystem::path("cmake") / ("Find" + assets::capitalize(name) + ".cmake"));
    } catch (const YAML::Exception &error) {
        std::cerr << configuration.string() << " illisible : " << error.what() << std::endl;
        return 1;
    }

    std::cout << name << " retire, lance devkit build" << std::endl;
    return 0;
}
