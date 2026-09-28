/**
 * @ Author: Perry Chouteau
 * @ Create Time: 2026-09-20 01:12:32
 * @ Description: les actions de devkit. Une par nom, dans sources/.
 */

#pragma once

#include "Cli.hpp"

#include <filesystem>

/** @brief La source de verite d'un projet. Tout le reste en derive. */
inline const std::filesystem::path configuration = "config.yaml";

/**
 * @brief Regenere le projet depuis config.yaml.
 *
 * Appelee a la fin de chaque commande qui touche a la configuration : il n'y
 * a jamais de "et maintenant lance build". Rien de ce qui est ecrit a la
 * main n'est ecrase, hors des blocs devkit:.
 */
int generate(bool force);

int init(const cli::Call &call);          ///< ecrit config.yaml dans le dossier courant

int depsLs(const cli::Call &call);        ///< les dependances, du produit et des bacs a sable
int depsAdd(const cli::Call &call);       ///< ajoute une dependance
int depsRm(const cli::Call &call);        ///< retire une dependance
int depsSet(const cli::Call &call);       ///< change la version ou la liaison
int depsOutdated(const cli::Call &call);  ///< compare les versions epinglees a celles de github

int examplesLs(const cli::Call &call);    ///< les bacs a sable
int examplesAdd(const cli::Call &call);   ///< ajoute examples/<nom>
int examplesRm(const cli::Call &call);    ///< retire un bac a sable de la configuration

int info(const cli::Call &call);          ///< ce projet en un coup d'oeil
int list(const cli::Call &call);          ///< les depots publics de P-E-R-R-Y
int sync(const cli::Call &call);          ///< regenere, quand config.yaml a ete edite a la main
int clean(const cli::Call &call);         ///< vide build/ sans enlever le dossier
