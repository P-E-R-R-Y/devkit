/**
 * @file Github.hpp
 * @brief Les depots de l'organisation, et leur derniere etiquette.
 *
 * Une seule requete GraphQL, donc un jeton obligatoire : GitHub refuse
 * GraphQL sans authentification, et l'API REST anonyme est limitee a 60
 * requetes par heure, soit moins d'un inventaire complet.
 */

#pragma once

#include "Http.hpp"

#include <cstdlib>
#include <string>
#include <vector>

namespace github {

    inline const std::string organisation = "P-E-R-R-Y";

    struct Entry {
        std::string name;
        std::string tag = "-";
    };

    /** @brief Le jeton, s'il est dans l'environnement. */
    inline std::string token() {
        for (const char *variable : {"GITHUB_TOKEN", "GH_TOKEN"})
            if (const char *value = std::getenv(variable))
                return value;
        return "";
    }

    /** @brief La valeur de la prochaine clef "name", et ou elle s'arrete. */
    inline bool nextName(const std::string &json, std::size_t &at, std::string &value) {
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
    inline const std::string query =
        R"gql({"query":"query { organization(login: \")gql" + organisation +
        R"gql(\") { repositories(first: 100, privacy: PUBLIC) { nodes { name )gql"
        R"gql(refs(refPrefix: \"refs/tags/\", first: 1, )gql"
        R"gql(orderBy: {field: TAG_COMMIT_DATE, direction: DESC}) { nodes { name } } } } } }"})gql";

    /**
     * @brief Les depots publics, tries. Rend faux et explique sur stderr.
     */
    bool repositories(std::vector<Entry> &found);
}
