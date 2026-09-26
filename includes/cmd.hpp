/**
 * @ Author: Perry Chouteau
 * @ Create Time: 2026-09-20 01:12:32
 * @ Description: les actions de devkit. Une par fichier, dans sources/.
 */

#pragma once

#include "Cli.hpp"

#include <filesystem>

/** @brief La source de verite d'un projet. init l'ecrit, build en derive tout. */
inline const std::filesystem::path configuration = "config.yaml";

int init(const cli::Call &call);    ///< ecrit config.yaml, et rien d'autre
int set(const cli::Call &call);     ///< ajoute ou modifie une dependance
int del(const cli::Call &call);     ///< retire une dependance
int get(const cli::Call &call);     ///< affiche la configuration
int list(const cli::Call &call);    ///< les depots publics de P-E-R-R-Y
int build(const cli::Call &call);   ///< genere le projet depuis config.yaml
int clean(const cli::Call &call);   ///< efface le dossier de build
