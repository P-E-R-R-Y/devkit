# devkit — changelog

Markers: 🟢 added · 🔴 breaking · 🔵 fix · ⚪ internal or docs · 🟡 proposed
in the plan, no code written yet.

## v0.1.0

- 🔴 devkit genere des projets P-E-R-R-Y : l'ancien modele de jeu (ECS,
  `Components.hpp`, `Systems.hpp`, les assets audio et image) disparait
- 🟢 `Cli.hpp` : un arbre de commandes declare comme une donnee, arite
  `Flag`/`One`/`Many`, choix imposes, defauts, options requises, alias de
  commandes, `-h`/`--help` partout
- 🟢 `config.yaml` comme source unique : `init`, `set`, `del`, `get`,
  `build`, `clean`, et `list` pour les depots publics de P-E-R-R-Y
- 🟢 `init <nom> -k static shared app` : les trois sorties sont toujours
  ecrites dans le CMakeLists, celles que le depot ne produit pas restent
  commentees
- 🟢 `init --example` : un bac a sable dans `example/`, avec son propre main
  et une classe `<Nom>App : IApp` qui ouvre une fenetre ; il epingle icore et
  raylib_impl a part, `set` ne lie que le produit
- 🟢 `build` regenere les blocs `devkit:*` du CMakeLists sans toucher au code
  ecrit a la main, et ecrit un `cmake/Find<Dep>.cmake` par dependance
- 🟢 `install.sh` : symlink par defaut, `--copy` et `--prefix` au choix, les
  assets resolus par `$DEVKIT_ASSETS` puis `~/.local/share/devkit`
- 🟢 `list` interroge GraphQL avec `GITHUB_TOKEN`, sans dependre de `gh`
- ⚪ 14 tests sur `Cli.hpp`, et `examples/kubectl.cpp` comme demonstration
