# devkit

🧰 **P-E-R-R-Y devkit**

The command line that creates and maintains P-E-R-R-Y repositories.

[![Build](https://github.com/P-E-R-R-Y/devkit/actions/workflows/tests.yml/badge.svg)](https://github.com/P-E-R-R-Y/devkit/actions)
[![Docs](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://p-e-r-r-y.github.io/devkit)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)

---

## ✨ Overview

`devkit` turns a single `config.yaml` into a complete repository: CMake, find
modules, tests, documentation, CI workflows and sandboxes. Every generated
project is the same file, to the variables near — the format the other
P-E-R-R-Y repositories already use.

Core goals:
- 🧩 **One source of truth**: `config.yaml` describes the repository, everything else derives from it
- ⚡ **No build step**: every command that edits the configuration regenerates in stride
- 🔒 **Nothing handwritten is lost**: only the `devkit:` blocks are rewritten
- 🎯 **One compilation**: an OBJECT stage feeds the static, the shared and the app
- ✅ **Unit-tested** with GoogleTest

---

## 🧱 Features

- Three outputs from one declaration — `static`, `shared`, `app` — each under the project's name on disk
- Suffixed target names (`physics_static`), so several flavours coexist in one build tree
- Dependencies pinned to a git tag, fetched by generated `cmake/Find<Dep>.cmake` modules
- Sandboxes: one folder per subject of study, each with its own `main` and its own `IApp`
- `deps outdated` compares what you pinned against the organisation's latest tags
- A declarative command tree (`Cli.hpp`) with arities, choices, defaults and aliases

---

## 🧩 Example Usage

```sh
mkdir physics && cd physics

# the name comes from the directory
devkit -h
devkit init -k static --example

devkit deps add system -v v1.0.1
devkit examples add collisions

devkit info
# physics v0.1.0
#   static shared  cmake 3.24, tests oui, docs oui, ci oui
#   1 dependance(s), 2 bac(s) a sable

cmake -B build -S . && cmake --build build
# libphysics.a   physics.dylib   physics_basic   physics_collisions
```

The generated `CMakeLists.txt` compiles the sources once and serves every output
from the same objects:

```cmake
add_library(${PROJECT_NAME}_objects OBJECT ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME}_objects PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(${PROJECT_NAME}_static STATIC $<TARGET_OBJECTS:${PROJECT_NAME}_objects>)
set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
add_library(${PROJECT_NAME} ALIAS ${PROJECT_NAME}_static)

add_library(${PROJECT_NAME}_shared SHARED $<TARGET_OBJECTS:${PROJECT_NAME}_objects> sources/symbole.cpp)
set_target_properties(${PROJECT_NAME}_shared PROPERTIES OUTPUT_NAME ${PROJECT_NAME} PREFIX "")
```

---

## 🔧 Install

```sh
./install.sh            # symlinks devkit into ~/.local/bin
./install.sh --copy     # copies the binary and its assets instead
```

`devkit list` and `devkit deps outdated` query the GitHub GraphQL API, which
needs a token:

```sh
export GITHUB_TOKEN=$(gh auth token)
```
