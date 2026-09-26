/**
 * @file init.cpp
 * @brief devkit init : ecrit config.yaml, et rien d'autre.
 *
 * Le projet lui-meme est genere par devkit build, depuis ce fichier.
 */

#include "Assets.hpp"
#include "cmd.hpp"

#include <yaml-cpp/yaml.h>

#include <iostream>
#include <utility>

int init(const cli::Call &call) {
    if (std::filesystem::exists(configuration) && !call.has("force")) {
        std::cerr << configuration.string() << " existe deja, --force pour l'ecraser" << std::endl;
        return 1;
    }

    YAML::Emitter out;

    out.SetIndent(4);
    out << YAML::BeginMap
        << YAML::Key << "global" << YAML::Value
            << YAML::BeginMap
            << YAML::Key << "name"          << YAML::Value << call.arguments.front()
            << YAML::Key << "brief"         << YAML::Value << call.value("brief")
            << YAML::Key << "version"       << YAML::Value << call.value("version")
            << YAML::Key << "cmake"         << YAML::Value << call.value("cmake")
            //static, shared, app : cumulables. Le main est la seule difference.
            << YAML::Key << "kind" << YAML::Value << YAML::Flow << YAML::BeginSeq;
    for (const std::string &kind : call.values("kind"))
        out << kind;
    out << YAML::EndSeq    << YAML::Key << "tests"         << YAML::Value << !call.has("no-tests")
            << YAML::Key << "documentation" << YAML::Value << !call.has("no-docs")
            << YAML::Key << "cicd"          << YAML::Value << !call.has("no-cicd")
            << YAML::EndMap
        //liste vide explicite : une clef sans valeur vaudrait null
        << YAML::Key << "repositories" << YAML::Value << YAML::Flow << YAML::BeginSeq << YAML::EndSeq;

    /* --example n'a pas d'option : il epingle lui-meme de quoi ouvrir une
     * fenetre. icore donne la boucle, raylib_impl le rendu - et amene
     * igraphic, iaudio et system avec lui, en PUBLIC. */
    if (call.has("example")) {
        out << YAML::Key << "example" << YAML::Value << YAML::BeginSeq;
        for (const auto &[name, tag] : {std::pair<const char *, const char *>{"icore", "v0.2.0"},
                                        {"raylib_impl", "v0.2.0"}}) {
            YAML::Node one;

            one["name"] = name;
            one["tag"] = tag;
            one.SetStyle(YAML::EmitterStyle::Block);
            out << one;
        }
        out << YAML::EndSeq;
    }
    out << YAML::EndMap;

    if (!assets::write(configuration, std::string(out.c_str()) + "\n")) {
        std::cerr << "ecriture impossible : " << configuration.string() << std::endl;
        return 1;
    }
    std::cout << "ecrit " << configuration.string() << ", lance devkit build" << std::endl;
    return 0;
}
