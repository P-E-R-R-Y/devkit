#!/bin/sh
# Installe devkit. Deux modes :
#   ./install.sh            lien vers le binaire compile : recompiler suffit a mettre a jour
#   ./install.sh --copy     copie binaire et gabarits : le depot peut disparaitre
#
#   --prefix <dir>          racine d'installation (defaut : ~/.local)

set -e

root=$(cd "$(dirname "$0")" && pwd)
prefix="$HOME/.local"
mode="link"

while [ $# -gt 0 ]; do
    case "$1" in
        --copy)   mode="copy" ;;
        --prefix) shift; prefix="$1" ;;
        -h|--help)
            sed -n '2,7p' "$0" | sed 's/^# \{0,1\}//'
            exit 0 ;;
        *) echo "option inconnue : $1" >&2; exit 1 ;;
    esac
    shift
done

echo "compilation..."
log=$(mktemp)
if ! cmake -S "$root" -B "$root/build" > "$log" 2>&1 ||
   ! cmake --build "$root/build" --target devkit -j 8 >> "$log" 2>&1; then
    cat "$log" >&2
    rm -f "$log"
    exit 1
fi
rm -f "$log"

mkdir -p "$prefix/bin"

if [ "$mode" = "copy" ]; then
    mkdir -p "$prefix/share/devkit"
    rm -rf "$prefix/share/devkit/assets"
    cp -R "$root/assets" "$prefix/share/devkit/assets"
    #retirer d'abord : un lien laisse par le mode precedent pointerait sur la source
    rm -f "$prefix/bin/devkit"
    cp "$root/build/devkit" "$prefix/bin/devkit"
    echo "installe   $prefix/bin/devkit"
    echo "gabarits   $prefix/share/devkit/assets"
    echo
    echo "Le dossier du depot reste prioritaire tant qu'il existe."
    echo "Pour forcer la copie : export DEVKIT_ASSETS=$prefix/share/devkit/assets"
else
    ln -sf "$root/build/devkit" "$prefix/bin/devkit"
    echo "lien       $prefix/bin/devkit -> $root/build/devkit"
    echo "gabarits   $root/assets"
    echo
    echo "Recompiler le depot met a jour la commande, sans reinstaller."
fi

case ":$PATH:" in
    *":$prefix/bin:"*) ;;
    *)
        echo
        echo "$prefix/bin n'est pas dans ton PATH. Ajoute a ton ~/.zshrc :"
        echo "  export PATH=\"$prefix/bin:\$PATH\"" ;;
esac
