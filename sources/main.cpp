/**
 * @ Author: Perry Chouteau
 * @ Create Time: 2025-02-20 16:15:07
 * @ Description: devkit, the command line.
 *
 *   cd physics && devkit init -k static shared
 *   devkit deps add system -v v1.0.1
 *   devkit deps add raylib_impl -v v0.2.0 -e
 *   devkit deps outdated
 *   devkit examples add collisions
 *   devkit info
 *
 * Every command touching config.yaml regenerates the project right after:
 * there is never a "and now, build it".
 */

#include "Cli.hpp"
#include "cmd.hpp"

/** @brief What the command bears on: the product, or the sandbox pool. */
static const cli::Option scope{
    .name = "example", .alias = "e",
    .help = "bears on the sandboxes instead of the product", .arity = cli::Arity::Flag};

static const cli::Option version{
    .name = "version", .alias = "v", .help = "git tag, for instance v1.0.0"};

static const cli::Option shared{
    .name = "shared", .alias = "s",
    .help = "copied next to the binary instead of being linked", .arity = cli::Arity::Flag};

static const cli::Command devkit{
    .name = "devkit",
    .help = "creates and maintains P-E-R-R-Y projects",
    .commands = {
        {.name = "init", .help = "sets up the current directory",
         .options = {
             {.name = "kind", .alias = "k", .help = "what the repository produces, cumulative",
              .arity = cli::Arity::Many, .choices = {"static", "shared", "app"}, .required = true},
             {.name = "version", .alias = "v", .help = "project version", .fallback = "v0.1.0"},
             {.name = "cmake", .help = "minimum CMake version", .fallback = "3.24"},
             {.name = "force", .alias = "f", .help = "overwrites an existing config.yaml", .arity = cli::Arity::Flag},
             {.name = "example", .help = "lays down a sandbox named basic", .arity = cli::Arity::Flag},
             {.name = "no-tests", .help = "without tests", .arity = cli::Arity::Flag},
             {.name = "no-docs", .help = "without documentation", .arity = cli::Arity::Flag},
             {.name = "no-cicd", .help = "without continuous integration", .arity = cli::Arity::Flag},
         },
         .run = init},

        {.name = "deps", .help = "this project's dependencies",
         .commands = {
             {.name = "ls", .help = "list them", .run = depsLs},
             {.name = "add", .help = "add one", .options = {version, shared, scope},
              .arguments = {.name = "repository", .min = 1, .max = 1}, .run = depsAdd},
             {.name = "rm", .help = "drop one", .options = {scope},
              .arguments = {.name = "repository", .min = 1, .max = 1}, .run = depsRm},
             {.name = "set", .help = "change its version or its linkage", .options = {version, shared, scope},
              .arguments = {.name = "repository", .min = 1, .max = 1}, .run = depsSet},
             {.name = "outdated", .help = "pinned versions against github", .run = depsOutdated},
         }},

        {.name = "examples", .help = "this project's sandboxes",
         .commands = {
             {.name = "ls", .help = "list them", .run = examplesLs},
             {.name = "add", .help = "add one", .arguments = {.name = "name", .min = 1, .max = 1},
              .run = examplesAdd},
             {.name = "rm", .help = "drop one", .arguments = {.name = "name", .min = 1, .max = 1},
              .run = examplesRm},
         }},

        {.name = "info", .help = "this project at a glance", .run = info},

        {.name = "list", .help = "the public P-E-R-R-Y repositories",
         .options = {{.name = "url", .help = "print the full addresses", .arity = cli::Arity::Flag}},
         .run = list},

        {.name = "sync", .help = "regenerates, after a config.yaml edited by hand",
         .options = {
             {.name = "force", .alias = "f", .help = "rewrites existing files", .arity = cli::Arity::Flag},
         },
         .run = sync},

        {.name = "clean", .help = "empties build/ without removing the folder", .run = clean},
    },
};

int main(int argc, char **argv) {
    return cli::execute(devkit, argc, argv);
}
