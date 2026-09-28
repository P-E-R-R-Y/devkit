/**
 * @file deps.cpp
 * @brief devkit deps : les dependances, du produit comme des bacs a sable.
 *
 * -e les fait porter sur le pool des bacs a sable plutot que sur le produit.
 * Chaque modification regenere : il n'y a jamais de commande a lancer apres.
 */

#include "Config.hpp"
#include "Github.hpp"
#include "Yaml.hpp"

#include <algorithm>
#include <iostream>

namespace {

    using devkit::Repository;

    void show(const std::string &title, const std::vector<Repository> &all) {
        std::cout << title << std::endl;
        if (all.empty())
            std::cout << "  aucune" << std::endl;
        for (const Repository &one : all)
            std::cout << "  " << one.name
                      << std::string(std::max<int>(1, 24 - static_cast<int>(one.name.size())), ' ')
                      << one.tag << "  (" << one.linkage << ")" << std::endl;
    }

    /** @brief Le noeud d'une dependance deja listee, vide sinon. */
    YAML::Node find(YAML::Node list, const std::string &name) {
        for (YAML::Node one : list)
            if (one["name"].as<std::string>("") == name)
                return one;
        return YAML::Node(YAML::NodeType::Undefined);
    }

    /*
     * Rendu par valeur, jamais rempli en place : un Node nul passe en
     * parametre reste nul chez l'appelant tant qu'il n'est pas devenu une
     * collection, et la dependance partait en "~".
     */
    YAML::Node entry(const std::string &name, const std::string &tag, const std::string &linkage) {
        YAML::Node one;

        one["name"] = name;
        one["tag"] = tag;
        one["linkage"] = linkage;
        one.SetStyle(YAML::EmitterStyle::Block);
        return one;
    }

    int write(YAML::Node &root, YAML::Node list, bool sandbox) {
        list.SetStyle(YAML::EmitterStyle::Block);
        if (sandbox)
            root["example"]["repositories"] = list;
        else
            root["repositories"] = list;
        return devkit::save(root);
    }
}

int depsLs(const cli::Call &) {
    YAML::Node root;

    if (!devkit::open(root))
        return 1;

    const devkit::Config config = devkit::Config::load(configuration.string());

    show("produit :", config.repositories);
    show("bacs a sable :", config.example.repositories);
    return 0;
}

int depsAdd(const cli::Call &call) {
    YAML::Node root;

    if (!devkit::open(root))
        return 1;

    const std::string name = call.arguments.front();
    const bool sandbox = call.has("example");
    const std::string tag = call.value("version").empty() ? "main" : call.value("version");
    //une dependance est liee par defaut ; --shared la fait copier a cote du binaire
    const std::string linkage = call.has("shared") ? "shared" : "static";
    YAML::Node list = devkit::dependencies(root, sandbox);

    if (find(list, name)) {
        std::cerr << name << " est deja la, utilise devkit deps set" << std::endl;
        return 1;
    }
    if (call.value("version").empty())
        std::cerr << "sans -v, " << name << " suivra la branche par defaut : "
                  << "deux compilations peuvent differer" << std::endl;

    list.push_back(entry(name, tag, linkage));
    if (write(root, list, sandbox) != 0)
        return 1;
    std::cout << name << " " << tag << " ajoute" << (sandbox ? " aux bacs a sable" : "") << std::endl;
    return 0;
}

int depsSet(const cli::Call &call) {
    YAML::Node root;

    if (!devkit::open(root))
        return 1;

    const std::string name = call.arguments.front();
    const bool sandbox = call.has("example");
    YAML::Node list = devkit::dependencies(root, sandbox);
    YAML::Node one = find(list, name);

    if (!one) {
        std::cerr << name << " n'est pas une dependance, utilise devkit deps add" << std::endl;
        return 1;
    }
    if (!call.value("version").empty())
        one["tag"] = call.value("version");
    if (call.has("shared"))
        one["linkage"] = "shared";
    one.SetStyle(YAML::EmitterStyle::Block);

    if (write(root, list, sandbox) != 0)
        return 1;
    std::cout << name << " " << one["tag"].as<std::string>("main") << " modifie" << std::endl;
    return 0;
}

int depsRm(const cli::Call &call) {
    YAML::Node root;

    if (!devkit::open(root))
        return 1;

    const std::string name = call.arguments.front();
    const bool sandbox = call.has("example");
    const YAML::Node list = devkit::dependencies(root, sandbox);
    YAML::Node kept(YAML::NodeType::Sequence);
    bool found = false;

    for (const YAML::Node &one : list) {
        if (one["name"].as<std::string>("") == name) {
            found = true;
            continue;
        }
        kept.push_back(entry(one["name"].as<std::string>(""), one["tag"].as<std::string>("main"),
                             one["linkage"].as<std::string>("static")));
    }
    if (!found) {
        std::cerr << name << " n'est pas une dependance de ce projet" << std::endl;
        return 1;
    }
    if (write(root, kept, sandbox) != 0)
        return 1;
    //le cmake/Find<Nom>.cmake reste : il a pu etre retouche a la main
    std::cout << name << " retire (cmake/Find" << assets::capitalize(name)
              << ".cmake reste, a toi de l'effacer)" << std::endl;
    return 0;
}

int depsOutdated(const cli::Call &) {
    if (!std::filesystem::exists(configuration)) {
        std::cerr << "aucun " << configuration.string() << " ici, lance devkit init" << std::endl;
        return 1;
    }

    const devkit::Config config = devkit::Config::load(configuration.string());
    std::vector<github::Entry> found;

    if (!github::repositories(found))
        return 1;

    std::vector<Repository> all = config.repositories;
    unsigned late = 0;

    all.insert(all.end(), config.example.repositories.begin(), config.example.repositories.end());
    for (const Repository &one : all) {
        const auto latest = std::find_if(found.begin(), found.end(),
                                         [&](const github::Entry &e) { return e.name == one.name; });

        //un depot hors de l'organisation n'est pas en retard, il est ailleurs
        if (latest == found.end())
            continue;
        if (latest->tag == one.tag)
            continue;
        std::cout << one.name
                  << std::string(std::max<int>(1, 24 - static_cast<int>(one.name.size())), ' ')
                  << one.tag << "  ->  " << latest->tag << std::endl;
        late++;
    }
    std::cout << (late == 0 ? "tout est a jour" : std::to_string(late) + " a mettre a jour") << std::endl;
    return 0;
}
