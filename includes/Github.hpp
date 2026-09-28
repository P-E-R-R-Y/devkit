/**
 * @file Github.hpp
 * @brief The organisation's repositories, and their latest tag.
 *
 * A single GraphQL request, hence a mandatory token: GitHub refuses GraphQL
 * without authentication, and the anonymous REST API is capped at 60
 * requests an hour, less than one full inventory.
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

    /** @brief The token, if the environment carries one. */
    inline std::string token() {
        for (const char *variable : {"GITHUB_TOKEN", "GH_TOKEN"})
            if (const char *value = std::getenv(variable))
                return value;
        return "";
    }

    /** @brief The value of the next "name" key, and where it stops. */
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
     * gql delimiter: inside a raw string R"( ... )", the sequence )" closes
     * the string. The parenthesis of refs( would have been eaten.
     */
    inline const std::string query =
        R"gql({"query":"query { organization(login: \")gql" + organisation +
        R"gql(\") { repositories(first: 100, privacy: PUBLIC) { nodes { name )gql"
        R"gql(refs(refPrefix: \"refs/tags/\", first: 1, )gql"
        R"gql(orderBy: {field: TAG_COMMIT_DATE, direction: DESC}) { nodes { name } } } } } }"})gql";

    /**
     * @brief The public repositories, sorted. Returns false and explains on stderr.
     */
    bool repositories(std::vector<Entry> &found);
}
