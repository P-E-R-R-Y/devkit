/**
 * @file list.cpp
 * @brief devkit list: the public P-E-R-R-Y repositories, and their latest version.
 */

#include "Github.hpp"
#include "cmd.hpp"

#include <algorithm>
#include <iostream>

namespace github {

    bool repositories(std::vector<Entry> &found) {
        const std::string key = token();

        if (key.empty()) {
            std::cerr << "a token is needed: export GITHUB_TOKEN=<...>\n"
                      << "GitHub refuses GraphQL without authentication." << std::endl;
            return false;
        }

        const http::Answer answer = http::request("https://api.github.com/graphql", key, query);

        if (!answer.ok()) {
            std::cerr << "github answered " << answer.status
                      << (answer.status == 401 ? ": token refused" : "") << std::endl;
            return false;
        }
        //a 200 may carry a GraphQL error: read it rather than return an empty list
        if (answer.body.find("\"errors\"") != std::string::npos) {
            std::cerr << "github refused the request: " << answer.body.substr(0, 200) << std::endl;
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
                //"[]" closes at once: that repository carries no tag
                if (answer.body[at] != ']')
                    nextName(answer.body, at, entry.tag);
            }
            found.push_back(entry);
        }

        if (found.empty()) {
            std::cerr << "no repository in the answer" << std::endl;
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
    std::cout << "\n" << found.size() << " public repositories" << std::endl;
    return 0;
}
