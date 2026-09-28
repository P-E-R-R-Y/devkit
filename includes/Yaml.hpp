/**
 * @file Yaml.hpp
 * @brief Reading and rewriting config.yaml without losing its shape.
 *
 * Every command that modifies the configuration shares the same round trip:
 * load, touch one node, rewrite with the same indentation.
 */

#pragma once

#include "Assets.hpp"
#include "cmd.hpp"

#include <yaml-cpp/yaml.h>

#include <iostream>

namespace devkit {

    /** @brief Loads config.yaml. Returns false and explains if it is missing or lying. */
    inline bool open(YAML::Node &root) {
        if (!std::filesystem::exists(configuration)) {
            std::cerr << "no " << configuration.string() << " here, run devkit init" << std::endl;
            return false;
        }
        try {
            root = YAML::LoadFile(configuration.string());
        } catch (const YAML::Exception &error) {
            std::cerr << configuration.string() << " unreadable: " << error.what() << std::endl;
            return false;
        }
        return true;
    }

    /** @brief Rewrites config.yaml, then regenerates the project. */
    inline int save(const YAML::Node &root) {
        YAML::Emitter out;

        out.SetIndent(4);
        out << root;
        if (!assets::write(configuration, std::string(out.c_str()) + "\n")) {
            std::cerr << "cannot write: " << configuration.string() << std::endl;
            return 1;
        }
        //generation follows the configuration: there is nothing left to run
        return generate(false);
    }

    /** @brief Where a dependency lives: the product's list, or the sandboxes'. */
    inline YAML::Node dependencies(YAML::Node &root, bool sandbox) {
        return sandbox ? root["example"]["repositories"] : root["repositories"];
    }
}
