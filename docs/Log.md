# devkit — changelog

Markers: 🟢 added · 🔴 breaking · 🔵 fix · ⚪ internal or docs · 🟡 proposed
in the plan, no code written yet.

## v0.4.0

- 🔴 the two entry points move to the root, one per role: `main.cpp` for the
  executable, `symbole.cpp` for the shared library. `sources/` now holds the
  object stage alone, with no exception to carve out of it
- 🔴 `SOURCE_FILES` goes back to an explicit list, yours to maintain. It sits
  outside the `devkit:` markers, so devkit writes it once and never again: a
  file appears in the diff when you add it, and a draft left behind compiles
  nothing
- 🔵 `deps add` and `deps set -v` now carry the tag into the find module,
  rewriting that one line and leaving everything around it alone. Asking for
  a version and watching the module keep the old one was the worst of both
- 🟢 `tests/api.sh`: 130 checks over every command and every transition a
  project goes through, each stating what it proves, ending on a class
  written by hand that a sandbox uses, compiles against and breaks with

## v0.3.0

- 🔴 `-k` is now required: `devkit init` no longer picks static in silence
- 🟢 every command reports a drift between `config.yaml` and a
  `cmake/Find<Dep>.cmake` retouched by hand, and names the one the build
  actually uses; `sync --force` realigns the module
- 🟢 `tests/matrix.sh`: the ten combinations of kind, generated, configured,
  built and tested, dependencies fetched into one shared cache
- 🔵 an `app`-only repository had its tests linking no library at all, so
  they reached no symbol; the tests now take the objects and `DEVKIT_LINK`
- 🔵 `init --force` no longer rewrites the generated files: it bears on
  `config.yaml` alone, and a CMakeLists carrying custom blocks survives
- 🟢 the sources are globbed: dropping a `.cpp` into `sources/` is enough,
  `symbole.cpp` and `main.cpp` excepted
- 🟢 a `docs/Readme.md` template in the format of ecs and i18n, with the
  blocks left empty
- ⚪ the section titles moved inside the generated blocks: an empty block no
  longer leaves an orphan heading
- ⚪ devkit runs its own generated CI workflows

## v0.2.0

- 🔴 devkit speaks English: every message, every help text, every comment,
  and the comments of the files it generates
- 🟢 `deps outdated` now lists every dependency, pinned version beside the
  organisation's latest tag, and marks only those behind
- ⚪ `docs/Readme.md` in the format of ecs and i18n

## v0.1.0

- 🔴 devkit generates P-E-R-R-Y projects: the old game template (ECS,
  `Components.hpp`, `Systems.hpp`, the audio and image assets) is gone
- 🟢 `Cli.hpp`: a command tree declared as data, `Flag`/`One`/`Many` arities,
  imposed choices, defaults, required options, command aliases, `-h`/`--help`
  everywhere
- 🟢 `config.yaml` as the single source: `init`, `deps`, `examples`, `info`,
  `list`, `sync`, `clean`
- 🟢 `init -k static shared app` in the current directory: the name comes from
  the folder, and only the requested outputs are written
- 🟢 every target carries its suffix (`_static`, `_shared`, `_app`), and
  `OUTPUT_NAME` has them all come out under the project's name; an alias gives
  the bare name to the static one for existing consumers
- 🟢 OBJECT stage: the sources are compiled once, the three outputs help
  themselves to the same `.o`, in `POSITION_INDEPENDENT_CODE`
- 🟢 `sources/no_source.cpp`, `sources/symbole.cpp` and `sources/main.cpp`:
  one file per output, written only if that output exists
- 🟢 `deps add|rm|set <dep> [-v <tag>] [-e]`: `-e` bears on the sandbox pool
  instead of the product
- 🟢 `examples add|rm <name>`: one folder per subject of study, with its own
  `main` and its `<Name>App : IApp` class, target `<project>_<name>`
- 🟢 generation follows every command that touches the configuration; `sync`
  remains for a `config.yaml` edited by hand
- 🟢 `clean` empties `build/` while keeping the folder
- 🟢 `install.sh`: symlink by default, `--copy` and `--prefix` at will, the
  assets resolved through `$DEVKIT_ASSETS` then `~/.local/share/devkit`
- 🔵 `cmake/Find<Dep>.cmake` is no longer rewritten: a module reworked by hand
  did not survive adding a dependency
- 🔵 an added dependency came out as `~`: yaml-cpp does not propagate a write
  made into a null node received as a parameter
- ⚪ devkit follows its own format: object stage, five `devkit:` blocks, and
  `examples/kubectl` as the sandbox of `Cli.hpp`
- ⚪ 14 tests on `Cli.hpp`

---

## 🟡 Later, if it ever earns its place

- a `template/` folder per repository, holding the fragments devkit would
  compose into a generated app or sandbox: the includes, the members, and
  what belongs in init/event/update/draw. ecs would bring its registry,
  raylib_impl its window, i18n its locales, and `devkit examples add` would
  assemble a sandbox that already runs.

  Deliberately postponed: the build's quality comes first, and this touches
  every repository at once.
