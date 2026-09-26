/**
 * @file list.cpp
 * @brief devkit list : les depots publics de P-E-R-R-Y, et leur derniere version.
 *
 * Une seule requete GraphQL, donc un jeton obligatoire : GitHub refuse
 * GraphQL sans authentification, et l'API REST anonyme est limitee a 60
 * requetes par heure, soit moins d'un "list --version" complet.
 */

#include "Http.hpp"
#include "cmd.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

    const std::string organisation = "P-E-R-R-Y";

    struct Entry {
        std::string name;
        std::string tag = "-";
    };

    /** @brief Le jeton, s'il est dans l'environnement. */
    std::string token() {
        for (const char *variable : {"GITHUB_TOKEN", "GH_TOKEN"})
            if (const char *value = std::getenv(variable))
                return value;
        return "";
    }

    /** @brief La valeur de la prochaine clef "name", et ou elle s'arrete. */
    bool nextName(const std::string &json, std::size_t &at, std::string &value) {
        const std::string mark = "\"name\":\"";
        const std::size_t from = json.find(mark, at);

        if (from == std::string::npos)
            return false;

        const std::size_t start = from + mark.size();
        const std::size_t end = json.find('"', start);

        if (end == std::string::npos)
            return false;
        value = json.substr(start, end - start);
        at = end;
        return true;
    }

    /*
     * Delimiteur gql : dans une chaine brute R"( ... )", la suite )" ferme
     * la chaine. La parenthese de refs( aurait ete mangee.
     */
    const std::string query =
        R"gql({"query":"query { organization(login: \")gql" + organisation +
        R"gql(\") { repositories(first: 100, privacy: PUBLIC) { nodes { name )gql"
        R"gql(refs(refPrefix: \"refs/tags/\", first: 1, )gql"
        R"gql(orderBy: {field: TAG_COMMIT_DATE, direction: DESC}) { nodes { name } } } } } }"})gql";
}

int list(const cli::Call &call) {
    const std::string key = token();

    if (key.empty()) {
        std::cerr << "il faut un jeton : export GITHUB_TOKEN=<...>\n"
                  << "GitHub refuse GraphQL sans authentification." << std::endl;
        return 1;
    }

    const http::Answer answer = http::request("https://api.github.com/graphql", key, query);

    if (!answer.ok()) {
        std::cerr << "github a repondu " << answer.status
                  << (answer.status == 401 ? " : jeton refuse" : "") << std::endl;
        return 1;
    }
    //un 200 peut porter une erreur GraphQL : la lire plutot que rendre une liste vide
    if (answer.body.find("\"errors\"") != std::string::npos) {
        std::cerr << "github a refuse la requete : " << answer.body.substr(0, 200) << std::endl;
        return 1;
    }

    const std::string refs = "\"refs\":{\"nodes\":[";
    std::vector<Entry> repositories;
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
        repositories.push_back(entry);
    }

    if (repositories.empty()) {
        std::cerr << "aucun depot dans la reponse" << std::endl;
        return 1;
    }

    std::sort(repositories.begin(), repositories.end(),
              [](const Entry &a, const Entry &b) { return a.name < b.name; });

    for (const Entry &entry : repositories) {
        const std::string shown = call.has("url")
                                ? "https://github.com/" + organisation + "/" + entry.name : entry.name;

        std::cout << shown << std::string(std::max<int>(1, 40 - static_cast<int>(shown.size())), ' ')
                  << entry.tag << std::endl;
    }
    std::cout << "\n" << repositories.size() << " depots publics" << std::endl;
    return 0;
}
