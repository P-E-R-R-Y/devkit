/**
 * @ Author: Perry Chouteau
 * @ Create Time: 2026-09-20 01:12:32
 * @ Description: devkit's actions. One per noun, under sources/.
 */

#pragma once

#include "Cli.hpp"

#include <filesystem>

/** @brief A project's source of truth. Everything else derives from it. */
inline const std::filesystem::path configuration = "config.yaml";

/**
 * @brief Regenerates the project from config.yaml.
 *
 * Called at the end of every command touching the configuration: there is
 * never a "and now run build". Nothing written by hand is overwritten,
 * outside the devkit: blocks.
 */
int generate(bool force);

int init(const cli::Call &call);          ///< writes config.yaml in the current directory

int depsLs(const cli::Call &call);        ///< the dependencies, of the product and of the sandboxes
int depsAdd(const cli::Call &call);       ///< adds a dependency
int depsRm(const cli::Call &call);        ///< drops a dependency
int depsSet(const cli::Call &call);       ///< changes its version or its linkage
int depsOutdated(const cli::Call &call);  ///< pinned versions against github's latest tags

int examplesLs(const cli::Call &call);    ///< the sandboxes
int examplesAdd(const cli::Call &call);   ///< adds examples/<name>
int examplesRm(const cli::Call &call);    ///< drops a sandbox from the configuration

int info(const cli::Call &call);          ///< this project at a glance
int list(const cli::Call &call);          ///< the public P-E-R-R-Y repositories
int sync(const cli::Call &call);          ///< regenerates, when config.yaml was edited by hand
int clean(const cli::Call &call);         ///< empties build/ without removing the folder
