/**
 * @file Config.hpp
 * @brief config.yaml, held in memory. The project's only source of truth.
 */

#pragma once

#include <yaml-cpp/yaml.h>

#include <string>
#include <vector>

namespace devkit {

    /** @brief A dependency: its repository, its version, the way it is linked. */
    struct Repository {
        std::string name;
        std::string tag;
        std::string linkage = "static";   ///< static: linked in. shared: copied alongside.
    };

    /**
     * @brief The sandboxes: shared dependencies, and one folder per subject
     *        of study.
     *
     * examples/<name>/ gives the target <project>_<name>. The dependencies
     * are pinned once and shared, each folder stays on its own.
     */
    struct Example {
        std::vector<Repository> repositories;
        std::vector<std::string> names;

        bool empty() const { return names.empty(); }
    };

    struct Config {
        std::string name;
        std::string brief;
        std::string version = "v0.1.0";
        std::string cmake = "3.24";
        std::vector<std::string> kind{"static"};   ///< static, shared, app: cumulative
        bool tests = true;
        bool documentation = true;
        bool cicd = true;
        std::vector<Repository> repositories;
        Example example;                      ///< laid down by --example and examples add

        /** @brief Reads the file. Throws YAML::Exception if it is unreadable. */
        static Config load(const std::string &file) {
            const YAML::Node root = YAML::LoadFile(file);
            const YAML::Node global = root["global"];
            Config config;

            config.name = global["name"].as<std::string>("unnamed");
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
            const YAML::Node example = root["example"];

            for (const YAML::Node &one : example["repositories"])
                config.example.repositories.push_back({one["name"].as<std::string>(""),
                                                      one["tag"].as<std::string>("main"),
                                                      one["linkage"].as<std::string>("static")});
            for (const YAML::Node &one : example["names"])
                config.example.names.push_back(one.as<std::string>());
            return config;
        }

        /** @brief Is this output asked for? */
        bool wants(const std::string &wanted) const {
            for (const std::string &one : kind)
                if (one == wanted)
                    return true;
            return false;
        }
    };
}
