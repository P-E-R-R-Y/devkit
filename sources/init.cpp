/**
 * @file init.cpp
 * @brief devkit init : pose config.yaml, puis genere le projet.
 *
 * Le nom vient du dossier courant : un depot s'appelle comme son repertoire,
 * et le repeter en argument ouvrait la porte au desaccord entre les deux.
 */

#include "Assets.hpp"
#include "cmd.hpp"

#include <yaml-cpp/yaml.h>

#include <iostream>
#include <utility>

int init(const cli::Call &call) {
    const std::string name = std::filesystem::current_path().filename().string();

    if (std::filesystem::exists(configuration) && !call.has("force")) {
        std::cerr << configuration.string() << " existe deja, --force pour l'ecraser" << std::endl;
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
     * igraphic, iaudio et system avec lui, en PUBLIC.
     *
     * Les dependances sont posees une fois pour tous les bacs a sable ;
     * names dit quels dossiers existent, et devkit set example en ajoute. */
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
        std::cerr << "ecriture impossible : " << configuration.string() << std::endl;
        return 1;
    }
    //la generation suit : init rend un projet pret, pas un fichier a suivre
    if (generate(call.has("force")) != 0)
        return 1;
    std::cout << name << " " << call.value("version") << " initialise" << std::endl;
    return 0;
}
