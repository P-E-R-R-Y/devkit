/**
 * @file list.cpp
 * @brief devkit list : les depots publics de P-E-R-R-Y, et leur derniere version.
 */

#include "Github.hpp"
#include "cmd.hpp"

#include <algorithm>
#include <iostream>

namespace github {

    bool repositories(std::vector<Entry> &found) {
        const std::string key = token();

        if (key.empty()) {
            std::cerr << "il faut un jeton : export GITHUB_TOKEN=<...>\n"
                      << "GitHub refuse GraphQL sans authentification." << std::endl;
            return false;
        }

        const http::Answer answer = http::request("https://api.github.com/graphql", key, query);

        if (!answer.ok()) {
            std::cerr << "github a repondu " << answer.status
                      << (answer.status == 401 ? " : jeton refuse" : "") << std::endl;
            return false;
        }
        //un 200 peut porter une erreur GraphQL : la lire plutot que rendre une liste vide
        if (answer.body.find("\"errors\"") != std::string::npos) {
            std::cerr << "github a refuse la requete : " << answer.body.substr(0, 200) << std::endl;
            return false;
        }

        const std::string refs = "\"refs\":{\"nodes\":[";
        std::size_t at = 0;
        std::string name;

        while (nextName(answer.body, at, name)) {
            Entry entry{name};
            const std::size_t start = answer.body.find(refs, at);

            if (start != std::string::npos) {
                at = start + refs.size();
                //"[]" ferme aussitot : ce depot n'a aucune etiquette
                if (answer.body[at] != ']')
                    nextName(answer.body, at, entry.tag);
            }
            found.push_back(entry);
        }

        if (found.empty()) {
            std::cerr << "aucun depot dans la reponse" << std::endl;
            return false;
        }
        std::sort(found.begin(), found.end(),
                  [](const Entry &a, const Entry &b) { return a.name < b.name; });
        return true;
    }
}

int list(const cli::Call &call) {
    std::vector<github::Entry> found;

    if (!github::repositories(found))
        return 1;

    for (const github::Entry &entry : found) {
        const std::string shown = call.has("url")
                                ? "https://github.com/" + github::organisation + "/" + entry.name
                                : entry.name;

        std::cout << shown << std::string(std::max<int>(1, 40 - static_cast<int>(shown.size())), ' ')
                  << entry.tag << std::endl;
    }
    std::cout << "\n" << found.size() << " depots publics" << std::endl;
    return 0;
}
