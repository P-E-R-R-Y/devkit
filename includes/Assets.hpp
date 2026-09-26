/**
 * @file Assets.hpp
 * @brief Les gabarits de devkit/assets, recopies en remplacant les {{clefs}}.
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
     * @brief Le dossier des gabarits, cherche dans cet ordre :
     *
     *  1. $DEVKIT_ASSETS, pour essayer un jeu de gabarits sans rien installer ;
     *  2. le dossier du depot, fixe a la compilation — il gagne tant qu'il
     *     existe, pour qu'une modification de gabarit prenne effet aussitot ;
     *  3. la copie installee, seule restante si le depot a disparu.
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

    /** @brief Le contenu d'un fichier, "" s'il est illisible. */
    inline std::string read(const std::filesystem::path &file) {
        std::ifstream in(file);
        std::ostringstream all;

        all << in.rdbuf();
        return all.str();
    }

    /** @brief Remplace chaque {{clef}} par sa valeur. */
    inline std::string fill(std::string text, const std::map<std::string, std::string> &values) {
        for (const auto &[key, value] : values) {
            const std::string mark = "{{" + key + "}}";

            for (std::size_t at = text.find(mark); at != std::string::npos; at = text.find(mark, at))
                text.replace(at, mark.size(), value);
        }
        return text;
    }

    /** @brief Ecrit un texte, en creant les dossiers manquants. */
    inline bool write(const std::filesystem::path &target, const std::string &text) {
        if (target.has_parent_path())
            std::filesystem::create_directories(target.parent_path());

        std::ofstream out(target);

        out << text;
        return static_cast<bool>(out);
    }

    /**
     * @brief Copie un gabarit d'assets vers target, {{clefs}} remplacees.
     *
     * Un gabarit absent est une erreur, pas un fichier vide : c'est la
     * difference entre "rien a generer" et "je n'ai pas trouve le modele".
     */
    inline bool render(const std::string &name, const std::filesystem::path &target,
                       const std::map<std::string, std::string> &values = {}) {
        const std::filesystem::path source = root() / name;

        if (!std::filesystem::exists(source)) {
            std::fprintf(stderr, "gabarit introuvable : %s\n", source.c_str());
            return false;
        }
        return write(target, fill(read(source), values));
    }

    /**
     * @brief Remplace le contenu entre "# --- devkit:<marker> ---" et
     *        "# --- devkit:end ---".
     *
     * Les marqueurs restent, pour qu'une seconde passe retrouve son bloc.
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

    /** @brief Les mots, separes par une espace. */
    inline std::string join(const std::vector<std::string> &words) {
        std::string out;

        for (const std::string &word : words)
            out += (out.empty() ? "" : " ") + word;
        return out;
    }

    /** @brief "system" -> "System", pour Find<Nom>.cmake et find_package(<Nom>). */
    inline std::string capitalize(std::string name) {
        if (!name.empty())
            name[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(name[0])));
        return name;
    }
}
