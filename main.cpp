/**
 * @ Author: Perry Chouteau
 * @ Create Time: 2025-02-20 16:15:07
 * @ Description: devkit, la ligne de commande.
 *
 *   devkit init physics -k static shared -b "Corps rigides"
 *   devkit init hunter  -k shared app          (un iapp : module et binaire)
 *   devkit set system -v v1.0.0
 *   devkit set sfml_impl -v v0.2.0 --shared
 *   devkit get
 *   devkit del system
 *   devkit build
 */

#include "Cli.hpp"
#include "cmd.hpp"

static const cli::Command devkit{
    .name = "devkit",
    .help = "cree et construit des projets P-E-R-R-Y",
    .commands = {
        {.name = "init", .help = "ecrit config.yaml, et rien d'autre",
         .options = {
             {.name = "kind", .alias = "k", .help = "ce que le depot produit, cumulable",
              .arity = cli::Arity::Many, .choices = {"static", "shared", "app"}, .fallback = "static"},
             {.name = "brief", .alias = "b", .help = "une ligne de description", .fallback = "TODO"},
             {.name = "version", .alias = "v", .help = "version du projet", .fallback = "v0.1.0"},
             {.name = "cmake", .help = "version minimale de CMake", .fallback = "3.24"},
             {.name = "force", .alias = "f", .help = "ecrase un config.yaml existant", .arity = cli::Arity::Flag},
             {.name = "example", .help = "ajoute un exemple", .arity = cli::Arity::Flag},
             {.name = "no-tests", .help = "sans tests", .arity = cli::Arity::Flag},
             {.name = "no-docs", .help = "sans documentation", .arity = cli::Arity::Flag},
             {.name = "no-cicd", .help = "sans integration continue", .arity = cli::Arity::Flag},
         },
         .arguments = {.name = "nom", .min = 1, .max = 1},
         .run = init},

        {.name = "set", .help = "ajoute une dependance, ou modifie la sienne",
         .options = {
             {.name = "version", .alias = "v", .help = "tag git, par exemple v1.0.0"},
             {.name = "shared", .alias = "s", .help = "copiee a cote du binaire au lieu d'etre liee",
              .arity = cli::Arity::Flag},
         },
         .arguments = {.name = "depot", .min = 1, .max = 1},
         .run = set},

        {.name = "del", .help = "retire une dependance",
         .arguments = {.name = "depot", .min = 1, .max = 1},
         .run = del},

        {.name = "get", .help = "affiche la configuration, ou une dependance",
         .arguments = {.name = "depot", .max = 1},
         .run = get},

        {.name = "list", .help = "les depots publics de P-E-R-R-Y",
         .options = {{.name = "url", .help = "affiche les adresses completes", .arity = cli::Arity::Flag}},
         .run = list},

        {.name = "build", .help = "genere le projet depuis config.yaml",
         .options = {
             {.name = "force", .alias = "f", .help = "reecrit les fichiers existants", .arity = cli::Arity::Flag},
         },
         .run = build},

        {.name = "clean", .help = "efface le dossier de build", .run = clean},
    },
};

int main(int argc, char **argv) {
    return cli::execute(devkit, argc, argv);
}
