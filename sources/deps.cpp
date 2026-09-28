/**
 * @file deps.cpp
 * @brief devkit deps: the dependencies, of the product as of the sandboxes.
 *
 * -e makes them bear on the sandbox pool rather than on the product. Every
 * change regenerates: there is never a command left to run afterwards.
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
            std::cout << "  none" << std::endl;
        for (const Repository &one : all)
            std::cout << "  " << one.name
                      << std::string(std::max<int>(1, 24 - static_cast<int>(one.name.size())), ' ')
                      << one.tag << "  (" << one.linkage << ")" << std::endl;
    }

    /** @brief The node of an already listed dependency, empty otherwise. */
    YAML::Node find(YAML::Node list, const std::string &name) {
        for (YAML::Node one : list)
            if (one["name"].as<std::string>("") == name)
                return one;
        return YAML::Node(YAML::NodeType::Undefined);
    }

    /*
     * Returned by value, never filled in place: a null Node passed as a
     * parameter stays null for the caller until it becomes a collection,
     * and the dependency used to come out as "~".
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

    show("product:", config.repositories);
    show("sandboxes:", config.example.repositories);
    return 0;
}

int depsAdd(const cli::Call &call) {
    YAML::Node root;

    if (!devkit::open(root))
        return 1;

    const std::string name = call.arguments.front();
    const bool sandbox = call.has("example");
    const std::string tag = call.value("version").empty() ? "main" : call.value("version");
    //a dependency is linked by default; --shared has it copied next to the binary
    const std::string linkage = call.has("shared") ? "shared" : "static";
    YAML::Node list = devkit::dependencies(root, sandbox);

    if (find(list, name)) {
        std::cerr << name << " is already there, use devkit deps set" << std::endl;
        return 1;
    }
    if (call.value("version").empty())
        std::cerr << "without -v, " << name << " will follow the default branch: "
                  << "two builds may differ" << std::endl;

    list.push_back(entry(name, tag, linkage));
    if (write(root, list, sandbox) != 0)
        return 1;
    std::cout << name << " " << tag << " added" << (sandbox ? " to the sandboxes" : "") << std::endl;
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
        std::cerr << name << " is not a dependency, use devkit deps add" << std::endl;
        return 1;
    }
    if (!call.value("version").empty())
        one["tag"] = call.value("version");
    if (call.has("shared"))
        one["linkage"] = "shared";
    one.SetStyle(YAML::EmitterStyle::Block);

    if (write(root, list, sandbox) != 0)
        return 1;
    std::cout << name << " " << one["tag"].as<std::string>("main") << " changed" << std::endl;
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
        std::cerr << name << " is not a dependency of this project" << std::endl;
        return 1;
    }
    if (write(root, kept, sandbox) != 0)
        return 1;
    //the cmake/Find<Name>.cmake stays: it may have been reworked by hand
    std::cout << name << " dropped (cmake/Find" << assets::capitalize(name)
              << ".cmake stays, yours to remove)" << std::endl;
    return 0;
}

int depsOutdated(const cli::Call &) {
    //pinned on the left, github's latest on the right, for every dependency
    if (!std::filesystem::exists(configuration)) {
        std::cerr << "no " << configuration.string() << " here, run devkit init" << std::endl;
        return 1;
    }

    const devkit::Config config = devkit::Config::load(configuration.string());
    std::vector<github::Entry> found;

    if (!github::repositories(found))
        return 1;

    std::vector<Repository> all = config.repositories;
    unsigned late = 0;

    all.insert(all.end(), config.example.repositories.begin(), config.example.repositories.end());
    std::cout << "  dependency             pinned      latest" << std::endl;
    for (const Repository &one : all) {
        const auto latest = std::find_if(found.begin(), found.end(),
                                         [&](const github::Entry &e) { return e.name == one.name; });
        //a repository outside the organisation is not behind, it is elsewhere
        const std::string tag = latest == found.end() ? "?" : latest->tag;
        const bool behind = latest != found.end() && latest->tag != one.tag;

        std::cout << "  " << one.name
                  << std::string(std::max<int>(1, 24 - static_cast<int>(one.name.size())), ' ')
                  << one.tag
                  << std::string(std::max<int>(1, 12 - static_cast<int>(one.tag.size())), ' ')
                  << tag << (behind ? "   <- update" : "") << std::endl;
        late += behind;
    }
    std::cout << (late == 0 ? "everything is up to date"
                            : std::to_string(late) + " to update") << std::endl;
    return 0;
}
