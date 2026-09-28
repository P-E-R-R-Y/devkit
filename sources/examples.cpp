/**
 * @file examples.cpp
 * @brief devkit examples: the sandboxes, one folder each.
 *
 * examples/<name>/ gives the target <project>_<name>. The first sandbox also
 * lays down the dependency pool that all of them share.
 */

#include "Config.hpp"
#include "Yaml.hpp"

#include <iostream>

namespace {

    /** @brief icore gives the loop, raylib_impl the rendering. */
    void pool(YAML::Node example) {
        if (example["repositories"])
            return;

        YAML::Node repositories;

        for (const auto &[name, tag] :
             {std::pair<const char *, const char *>{"icore", "v0.2.0"}, {"raylib_impl", "v0.2.0"}}) {
            YAML::Node one;

            one["name"] = name;
            one["tag"] = tag;
            one.SetStyle(YAML::EmitterStyle::Block);
            repositories.push_back(one);
        }
        repositories.SetStyle(YAML::EmitterStyle::Block);
        example["repositories"] = repositories;
    }
}

int examplesLs(const cli::Call &) {
    YAML::Node root;

    if (!devkit::open(root))
        return 1;

    const devkit::Config config = devkit::Config::load(configuration.string());

    if (config.example.names.empty())
        std::cout << "no sandbox yet, add one: devkit examples add basic" << std::endl;
    for (const std::string &name : config.example.names)
        std::cout << name
                  << std::string(std::max<int>(1, 24 - static_cast<int>(name.size())), ' ')
                  << "examples/" << name << "  ->  " << config.name << "_" << name << std::endl;
    return 0;
}

int examplesAdd(const cli::Call &call) {
    YAML::Node root;

    if (!devkit::open(root))
        return 1;

    const std::string name = call.arguments.front();
    YAML::Node example = root["example"];
    YAML::Node names = example["names"];

    for (const YAML::Node &one : names)
        if (one.as<std::string>("") == name) {
            std::cerr << name << " already exists" << std::endl;
            return 1;
        }

    pool(example);
    names.push_back(name);
    names.SetStyle(YAML::EmitterStyle::Flow);
    example["names"] = names;
    root["example"] = example;

    if (devkit::save(root) != 0)
        return 1;
    std::cout << "examples/" << name << " added" << std::endl;
    return 0;
}

int examplesRm(const cli::Call &call) {
    YAML::Node root;

    if (!devkit::open(root))
        return 1;

    const std::string name = call.arguments.front();
    YAML::Node example = root["example"];
    YAML::Node kept(YAML::NodeType::Sequence);
    bool found = false;

    for (const YAML::Node &one : example["names"]) {
        if (one.as<std::string>("") == name) {
            found = true;
            continue;
        }
        kept.push_back(one.as<std::string>(""));
    }
    if (!found) {
        std::cerr << name << " is not a sandbox of this project" << std::endl;
        return 1;
    }
    kept.SetStyle(YAML::EmitterStyle::Flow);
    example["names"] = kept;
    root["example"] = example;

    if (devkit::save(root) != 0)
        return 1;
    //the folder stays: it holds code written by hand
    std::cout << name << " dropped from the configuration (examples/" << name
              << " stays, yours to remove)" << std::endl;
    return 0;
}
