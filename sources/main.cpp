/**
 * @ Author: Perry Chouteau
 * @ Create Time: 2025-02-20 16:15:07
 * @ Description: devkit, la ligne de commande.
 *
 *   cd physics && devkit init -k static shared
 *   devkit deps add system -v v1.0.1
 *   devkit deps add raylib_impl -v v0.2.0 -e
 *   devkit deps outdated
 *   devkit examples add collisions
 *   devkit info
 *
 * Toute commande qui touche a config.yaml regenere le projet dans la foulee :
 * il n'y a jamais de "et maintenant, construis".
 */

#include "Cli.hpp"
#include "cmd.hpp"

/** @brief Ou porte la commande : le produit, ou le pool des bacs a sable. */
static const cli::Option scope{
    .name = "example", .alias = "e",
    .help = "porte sur les bacs a sable au lieu du produit", .arity = cli::Arity::Flag};

static const cli::Option version{
    .name = "version", .alias = "v", .help = "tag git, par exemple v1.0.0"};

static const cli::Option shared{
    .name = "shared", .alias = "s",
    .help = "copiee a cote du binaire au lieu d'etre liee", .arity = cli::Arity::Flag};

static const cli::Command devkit{
    .name = "devkit",
    .help = "cree et construit des projets P-E-R-R-Y",
    .commands = {
        {.name = "init", .help = "initialise le dossier courant",
         .options = {
             {.name = "kind", .alias = "k", .help = "ce que le depot produit, cumulable",
              .arity = cli::Arity::Many, .choices = {"static", "shared", "app"}, .fallback = "static"},
             {.name = "version", .alias = "v", .help = "version du projet", .fallback = "v0.1.0"},
             {.name = "cmake", .help = "version minimale de CMake", .fallback = "3.24"},
             {.name = "force", .alias = "f", .help = "ecrase un config.yaml existant", .arity = cli::Arity::Flag},
             {.name = "example", .help = "pose un bac a sable nomme basic", .arity = cli::Arity::Flag},
             {.name = "no-tests", .help = "sans tests", .arity = cli::Arity::Flag},
             {.name = "no-docs", .help = "sans documentation", .arity = cli::Arity::Flag},
             {.name = "no-cicd", .help = "sans integration continue", .arity = cli::Arity::Flag},
         },
         .run = init},

        {.name = "deps", .help = "les dependances de ce projet",
         .commands = {
             {.name = "ls", .help = "les lister", .run = depsLs},
             {.name = "add", .help = "en ajouter une", .options = {version, shared, scope},
              .arguments = {.name = "depot", .min = 1, .max = 1}, .run = depsAdd},
             {.name = "rm", .help = "en retirer une", .options = {scope},
              .arguments = {.name = "depot", .min = 1, .max = 1}, .run = depsRm},
             {.name = "set", .help = "changer sa version ou sa liaison", .options = {version, shared, scope},
              .arguments = {.name = "depot", .min = 1, .max = 1}, .run = depsSet},
             {.name = "outdated", .help = "celles en retard sur github", .run = depsOutdated},
         }},

        {.name = "examples", .help = "les bacs a sable de ce projet",
         .commands = {
             {.name = "ls", .help = "les lister", .run = examplesLs},
             {.name = "add", .help = "en ajouter un", .arguments = {.name = "nom", .min = 1, .max = 1},
              .run = examplesAdd},
             {.name = "rm", .help = "en retirer un", .arguments = {.name = "nom", .min = 1, .max = 1},
              .run = examplesRm},
         }},

        {.name = "info", .help = "ce projet en un coup d'oeil", .run = info},

        {.name = "list", .help = "les depots publics de P-E-R-R-Y",
         .options = {{.name = "url", .help = "affiche les adresses completes", .arity = cli::Arity::Flag}},
         .run = list},

        {.name = "sync", .help = "regenere, apres un config.yaml edite a la main",
         .options = {
             {.name = "force", .alias = "f", .help = "reecrit les fichiers existants", .arity = cli::Arity::Flag},
         },
         .run = sync},

        {.name = "clean", .help = "vide build/ sans enlever le dossier", .run = clean},
    },
};

int main(int argc, char **argv) {
    return cli::execute(devkit, argc, argv);
}
