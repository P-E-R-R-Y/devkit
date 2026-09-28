# devkit — changelog

Markers: 🟢 added · 🔴 breaking · 🔵 fix · ⚪ internal or docs · 🟡 proposed
in the plan, no code written yet.

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
