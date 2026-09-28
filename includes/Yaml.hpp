/**
 * @file Yaml.hpp
 * @brief Lire et reecrire config.yaml sans en perdre la forme.
 *
 * Les commandes qui modifient la configuration partagent toutes le meme
 * aller-retour : charger, toucher un noeud, reecrire avec la meme indentation.
 */

#pragma once

#include "Assets.hpp"
#include "cmd.hpp"

#include <yaml-cpp/yaml.h>

#include <iostream>

namespace devkit {

    /** @brief Charge config.yaml. Rend faux et explique s'il manque ou ment. */
    inline bool open(YAML::Node &root) {
        if (!std::filesystem::exists(configuration)) {
            std::cerr << "aucun " << configuration.string() << " ici, lance devkit init" << std::endl;
            return false;
        }
        try {
            root = YAML::LoadFile(configuration.string());
        } catch (const YAML::Exception &error) {
            std::cerr << configuration.string() << " illisible : " << error.what() << std::endl;
            return false;
        }
        return true;
    }

    /** @brief Reecrit config.yaml, puis regenere le projet. */
    inline int save(const YAML::Node &root) {
        YAML::Emitter out;

        out.SetIndent(4);
        out << root;
        if (!assets::write(configuration, std::string(out.c_str()) + "\n")) {
            std::cerr << "ecriture impossible : " << configuration.string() << std::endl;
            return 1;
        }
        //la generation suit la configuration : il n'y a rien a lancer apres
        return generate(false);
    }

    /** @brief La liste ou vit une dependance : celle du produit, ou celle des bacs a sable. */
    inline YAML::Node dependencies(YAML::Node &root, bool sandbox) {
        return sandbox ? root["example"]["repositories"] : root["repositories"];
    }
}
