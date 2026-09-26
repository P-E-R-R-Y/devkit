/**
 * @file Config.hpp
 * @brief config.yaml, lu en memoire. La seule source de verite du projet.
 */

#pragma once

#include <yaml-cpp/yaml.h>

#include <string>
#include <vector>

namespace devkit {

    /** @brief Une dependance : son depot, sa version, sa facon d'etre liee. */
    struct Repository {
        std::string name;
        std::string tag;
        std::string linkage = "static";   ///< static : on lie. shared : on copie a cote.
    };

    struct Config {
        std::string name;
        std::string brief;
        std::string version = "v0.1.0";
        std::string cmake = "3.24";
        std::vector<std::string> kind{"static"};   ///< static, shared, app : cumulables
        bool tests = true;
        bool documentation = true;
        bool cicd = true;
        std::vector<Repository> repositories;
        std::vector<Repository> example;      ///< posees par --example, liees au bac a sable

        /** @brief Lit le fichier. Leve YAML::Exception s'il est illisible. */
        static Config load(const std::string &file) {
            const YAML::Node root = YAML::LoadFile(file);
            const YAML::Node global = root["global"];
            Config config;

            config.name = global["name"].as<std::string>("sans-nom");
            config.brief = global["brief"].as<std::string>("");
            config.version = global["version"].as<std::string>("v0.1.0");
            config.cmake = global["cmake"].as<std::string>("3.24");

            config.tests = global["tests"].as<bool>(true);
            config.documentation = global["documentation"].as<bool>(true);
            config.cicd = global["cicd"].as<bool>(true);

            if (global["kind"]) {
                config.kind.clear();
                for (const YAML::Node &one : global["kind"])
                    config.kind.push_back(one.as<std::string>());
            }
            for (const YAML::Node &one : root["repositories"])
                config.repositories.push_back({one["name"].as<std::string>(""),
                                               one["tag"].as<std::string>("main"),
                                               one["linkage"].as<std::string>("static")});
            for (const YAML::Node &one : root["example"])
                config.example.push_back({one["name"].as<std::string>(""),
                                          one["tag"].as<std::string>("main"),
                                          one["linkage"].as<std::string>("static")});
            return config;
        }

        /** @brief Cette sortie est-elle demandee ? */
        bool wants(const std::string &wanted) const {
            for (const std::string &one : kind)
                if (one == wanted)
                    return true;
            return false;
        }
    };
}
