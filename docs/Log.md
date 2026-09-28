# devkit — changelog

Markers: 🟢 added · 🔴 breaking · 🔵 fix · ⚪ internal or docs · 🟡 proposed
in the plan, no code written yet.

## v0.1.0

- 🔴 devkit genere des projets P-E-R-R-Y : l'ancien modele de jeu (ECS,
  `Components.hpp`, `Systems.hpp`, les assets audio et image) disparait
- 🟢 `Cli.hpp` : un arbre de commandes declare comme une donnee, arite
  `Flag`/`One`/`Many`, choix imposes, defauts, options requises, alias de
  commandes, `-h`/`--help` partout
- 🟢 `config.yaml` comme source unique : `init`, `deps`, `examples`, `info`,
  `list`, `sync`, `clean`
- 🟢 `init -k static shared app` dans le dossier courant : le nom vient du
  repertoire, et seules les sorties demandees sont ecrites
- 🟢 chaque cible porte son suffixe (`_static`, `_shared`, `_app`), et
  `OUTPUT_NAME` les fait toutes sortir sous le nom du projet ; un alias donne
  le nom nu a la statique pour les consommateurs existants
- 🟢 etage OBJECT : les sources sont compilees une fois, les trois sorties se
  servent des memes `.o`, en `POSITION_INDEPENDENT_CODE`
- 🟢 `sources/no_source.cpp`, `sources/symbole.cpp` et `sources/main.cpp` :
  un fichier par sortie, ecrit seulement si la sortie existe
- 🟢 `deps add|rm|set <dep> [-v <tag>] [-e]` : `-e` porte sur le pool des
  bacs a sable au lieu du produit
- 🟢 `deps outdated` compare les versions epinglees aux dernieres etiquettes
  de l'organisation
- 🟢 `examples add|rm <nom>` : un dossier par sujet d'essai, avec son `main`
  et sa classe `<Nom>App : IApp`, cible `<projet>_<nom>`
- 🟢 la generation suit chaque commande qui touche a la configuration ;
  `sync` reste pour un `config.yaml` edite a la main
- 🟢 `clean` vide `build/` en conservant le dossier
- 🟢 `install.sh` : symlink par defaut, `--copy` et `--prefix` au choix, les
  assets resolus par `$DEVKIT_ASSETS` puis `~/.local/share/devkit`
- 🔵 `cmake/Find<Dep>.cmake` n'est plus reecrit : un module retouche a la
  main survivait mal a l'ajout d'une dependance
- 🔵 une dependance ajoutee sortait en `~` : yaml-cpp ne propage pas
  l'ecriture faite dans un noeud nul recu en parametre
- ⚪ devkit suit son propre format : etage objet, cinq blocs `devkit:`, et
  `examples/kubectl` comme bac a sable de `Cli.hpp`
- ⚪ 14 tests sur `Cli.hpp`
