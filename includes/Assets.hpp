/**
 * @file Assets.hpp
 * @brief The templates of devkit/assets, copied with the {{keys}} replaced.
 */

#pragma once

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace assets {

    /**
     * @brief The templates folder, looked up in this order:
     *
     *  1. $DEVKIT_ASSETS, to try a set of templates without installing a thing;
     *  2. the repository folder, fixed at compile time - it wins as long as it
     *     exists, so that editing a template takes effect at once;
     *  3. the installed copy, all that is left once the repository is gone.
     */
    inline std::filesystem::path root() {
        if (const char *forced = std::getenv("DEVKIT_ASSETS"))
            return forced;
        if (std::filesystem::exists(DEVKIT_ASSETS))
            return DEVKIT_ASSETS;
        if (const char *home = std::getenv("HOME"))
            return std::filesystem::path(home) / ".local/share/devkit/assets";
        return DEVKIT_ASSETS;
    }

    /** @brief A file's contents, "" when it cannot be read. */
    inline std::string read(const std::filesystem::path &file) {
        std::ifstream in(file);
        std::ostringstream all;

        all << in.rdbuf();
        return all.str();
    }

    /** @brief Replaces every {{key}} with its value. */
    inline std::string fill(std::string text, const std::map<std::string, std::string> &values) {
        for (const auto &[key, value] : values) {
            const std::string mark = "{{" + key + "}}";

            for (std::size_t at = text.find(mark); at != std::string::npos; at = text.find(mark, at))
                text.replace(at, mark.size(), value);
        }
        return text;
    }

    /** @brief Writes a text, creating the missing folders. */
    inline bool write(const std::filesystem::path &target, const std::string &text) {
        if (target.has_parent_path())
            std::filesystem::create_directories(target.parent_path());

        std::ofstream out(target);

        out << text;
        return static_cast<bool>(out);
    }

    /**
     * @brief Copies a template from assets to target, {{keys}} replaced.
     *
     * A missing template is an error rather than an empty file: that is the
     * difference between "nothing to generate" and "I could not find the
     * model".
     */
    inline bool render(const std::string &name, const std::filesystem::path &target,
                       const std::map<std::string, std::string> &values = {}) {
        const std::filesystem::path source = root() / name;

        if (!std::filesystem::exists(source)) {
            std::fprintf(stderr, "template not found: %s\n", source.c_str());
            return false;
        }
        return write(target, fill(read(source), values));
    }

    /**
     * @brief Replaces the contents between "# --- devkit:<marker> ---" and
     *        "# --- devkit:end ---".
     *
     * The markers stay, so that a second pass finds its block again.
     */
    inline bool replaceBlock(const std::filesystem::path &file, const std::string &marker,
                             const std::string &content) {
        const std::string opening = "# --- devkit:" + marker + " ---";
        const std::string closing = "# --- devkit:end ---";
        std::string text = read(file);
        const std::size_t from = text.find(opening);

        if (from == std::string::npos)
            return false;

        const std::size_t to = text.find(closing, from);

        if (to == std::string::npos)
            return false;
        text.replace(from + opening.size(), to - from - opening.size(), "\n" + content);
        return write(file, text);
    }

    /** @brief The words, separated by one space. */
    inline std::string join(const std::vector<std::string> &words) {
        std::string out;

        for (const std::string &word : words)
            out += (out.empty() ? "" : " ") + word;
        return out;
    }

    /** @brief "system" -> "System", for Find<Name>.cmake and find_package(<Name>). */
    inline std::string capitalize(std::string name) {
        if (!name.empty())
            name[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(name[0])));
        return name;
    }
}
