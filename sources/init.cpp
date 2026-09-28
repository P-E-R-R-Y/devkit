/**
 * @file init.cpp
 * @brief devkit init: lays down config.yaml, then generates the project.
 *
 * The name comes from the current directory: a repository is called after its
 * folder, and repeating it as an argument invited the two to disagree.
 */

#include "Assets.hpp"
#include "cmd.hpp"

#include <yaml-cpp/yaml.h>

#include <iostream>
#include <utility>

int init(const cli::Call &call) {
    const std::string name = std::filesystem::current_path().filename().string();

    if (std::filesystem::exists(configuration) && !call.has("force")) {
        std::cerr << configuration.string() << " already exists, --force to overwrite it" << std::endl;
        return 1;
    }

    YAML::Emitter out;

    out.SetIndent(4);
    out << YAML::BeginMap
        << YAML::Key << "global" << YAML::Value
            << YAML::BeginMap
            << YAML::Key << "name"          << YAML::Value << name
            << YAML::Key << "version"       << YAML::Value << call.value("version")
            << YAML::Key << "cmake"         << YAML::Value << call.value("cmake")
            //static, shared, app: cumulative. The main is the only difference.
            << YAML::Key << "kind" << YAML::Value << YAML::Flow << YAML::BeginSeq;
    for (const std::string &kind : call.values("kind"))
        out << kind;
    out << YAML::EndSeq    << YAML::Key << "tests"         << YAML::Value << !call.has("no-tests")
            << YAML::Key << "documentation" << YAML::Value << !call.has("no-docs")
            << YAML::Key << "cicd"          << YAML::Value << !call.has("no-cicd")
            << YAML::EndMap
        //an explicit empty list: a key without a value would read as null
        << YAML::Key << "repositories" << YAML::Value << YAML::Flow << YAML::BeginSeq << YAML::EndSeq;

    /* --example takes no option: it pins what it needs to open a window on
     * its own. icore gives the loop, raylib_impl the rendering - and brings
     * igraphic, iaudio and system along, PUBLIC.
     *
     * The dependencies are laid down once for every sandbox; names says which
     * folders exist, and devkit examples add appends to it. */
    if (call.has("example")) {
        out << YAML::Key << "example" << YAML::Value << YAML::BeginMap
            << YAML::Key << "repositories" << YAML::Value << YAML::BeginSeq;
        for (const auto &[name, tag] : {std::pair<const char *, const char *>{"icore", "v0.2.0"},
                                        {"raylib_impl", "v0.2.0"}}) {
            YAML::Node one;

            one["name"] = name;
            one["tag"] = tag;
            one.SetStyle(YAML::EmitterStyle::Block);
            out << one;
        }
        out << YAML::EndSeq
            << YAML::Key << "names" << YAML::Value << YAML::Flow
            << YAML::BeginSeq << "basic" << YAML::EndSeq
            << YAML::EndMap;
    }
    out << YAML::EndMap;

    if (!assets::write(configuration, std::string(out.c_str()) + "\n")) {
        std::cerr << "cannot write: " << configuration.string() << std::endl;
        return 1;
    }
    //generation follows: init yields a ready project, not a file to act on
    if (generate(call.has("force")) != 0)
        return 1;
    std::cout << name << " " << call.value("version") << " initialised" << std::endl;
    return 0;
}
