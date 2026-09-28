#!/bin/sh
# Every devkit command, and every transition a project can go through.
#
# Each case states what it proves. The dependencies are fetched once into a
# shared cache: without it, the sandbox cases would pull raylib each time.
#
#   tests/api.sh [devkit binary] [work directory]

set -u

devkit=${1:-$(cd "$(dirname "$0")/.." && pwd)/build/devkit}
work=${2:-${TMPDIR:-/tmp}/devkit-api}
cache=$work/_deps
passed=0
failed=0

rm -rf "$work" && mkdir -p "$cache"

title() { printf '\n== %s\n' "$1"; }

ok()   { printf '   ok    %s\n' "$1"; passed=$((passed + 1)); }
ko()   { printf '   FAIL  %s\n' "$1"; failed=$((failed + 1)); [ $# -gt 1 ] && printf '         %s\n' "$2"; }

# check <what it proves> <command...>   : the command must succeed
check() { what=$1; shift; if "$@" >out 2>&1; then ok "$what"; else ko "$what" "$(tail -3 out | tr '\n' ' ')"; fi }

# refuse <what it proves> <command...>  : the command must fail
refuse() { what=$1; shift; if "$@" >out 2>&1; then ko "$what" "succeeded when it should not"; else ok "$what"; fi }

# holds <what it proves> <file> <text>  : the file must contain the text
holds() { if grep -qF "$3" "$2" 2>/dev/null; then ok "$1"; else ko "$1" "$2 lacks: $3"; fi }

# lacks <what it proves> <file> <text>  : the file must not contain the text
lacks() { if grep -qF "$3" "$2" 2>/dev/null; then ko "$1" "$2 still holds: $3"; else ok "$1"; fi }

# nomatch <what it proves> <file> <extended regex>  : nothing may match
nomatch() { if grep -qE "$3" "$2" 2>/dev/null; then ko "$1" "$2 matches: $3"; else ok "$1"; fi }

# exists / absent <what it proves> <path>
exists() { if [ -e "$2" ]; then ok "$1"; else ko "$1" "$2 is missing"; fi }
absent() { if [ -e "$2" ]; then ko "$1" "$2 should be gone"; else ok "$1"; fi }

fresh() { rm -rf "$work/$1" && mkdir -p "$work/$1" && cd "$work/$1" || exit 1; }

build() { cmake -B build -S . -DFETCHCONTENT_BASE_DIR="$cache" >>out 2>&1 && cmake --build build -j8 >>out 2>&1; }

# ----------------------------------------------------------------------------

title "init: refuses to guess what the repository produces"
fresh init-kind
refuse "devkit init without -k stops rather than picking static in silence" "$devkit" init
absent "nothing is written when it stops" config.yaml

title "init: lays down a whole repository from one flag"
fresh init-static
check "devkit init -k static succeeds" "$devkit" init -k static
exists "config.yaml is written" config.yaml
exists "the CMakeLists is generated" CMakeLists.txt
exists "the tests workflow is generated" .github/workflows/tests.yml
exists "the Readme template is generated" docs/Readme.md
exists "the changelog is generated" docs/Log.md
exists "no_source.cpp feeds the object stage" sources/no_source.cpp
holds "the project is named after the directory" config.yaml "name: init-static"
holds "the static target carries its suffix" CMakeLists.txt "add_library(\${PROJECT_NAME}_static STATIC"
holds "the bare name is an alias of the static" CMakeLists.txt "add_library(\${PROJECT_NAME} ALIAS \${PROJECT_NAME}_static)"
holds "the file name drops the suffix" CMakeLists.txt "OUTPUT_NAME \${PROJECT_NAME}"
lacks "no shared target is written for a static repository" CMakeLists.txt "_shared SHARED"
lacks "no commented-out line is left behind" CMakeLists.txt "#add_library"
absent "no main.cpp for a repository without an app" main.cpp
absent "no symbole.cpp for a repository without a shared library" symbole.cpp

title "init: refuses to overwrite, and never destroys handwritten work"
printf 'int mine() { return 1; }\n' > sources/mine.cpp
printf '#pragma once\n' > includes/Mine.hpp
printf '\n# my own block\nmessage(STATUS "mine")\n' >> CMakeLists.txt
refuse "a second init stops on the existing config.yaml" "$devkit" init -k static
check "init --force is accepted" "$devkit" init -k static -f
exists "a handwritten source survives" sources/mine.cpp
exists "a handwritten header survives" includes/Mine.hpp
holds "a custom CMake block outside the markers survives" CMakeLists.txt "my own block"

title "kind: a repository gains outputs along the way"
fresh kind-grow
check "init as a static library" "$devkit" init -k static
sed -i.bak 's|kind: \[static\]|kind: [static, shared, app]|' config.yaml && rm -f config.yaml.bak
check "sync after editing kind by hand" "$devkit" sync
holds "the shared target appears" CMakeLists.txt "_shared SHARED"
holds "the executable appears" CMakeLists.txt "add_executable(\${PROJECT_NAME}_app"
exists "main.cpp is written for the new executable" main.cpp
exists "symbole.cpp is written for the new shared library" symbole.cpp
holds "the shared library takes symbole.cpp alone" CMakeLists.txt "_shared SHARED \$<TARGET_OBJECTS:\${PROJECT_NAME}_objects> symbole.cpp"
holds "the executable takes main.cpp alone" CMakeLists.txt "_app \$<TARGET_OBJECTS:\${PROJECT_NAME}_objects> main.cpp"

title "kind: a repository drops an output, and keeps the code"
sed -i.bak 's|kind: \[static, shared, app\]|kind: [static]|' config.yaml && rm -f config.yaml.bak
check "sync after narrowing kind" "$devkit" sync
lacks "the shared target is gone from the CMakeLists" CMakeLists.txt "_shared SHARED"
lacks "the executable is gone from the CMakeLists" CMakeLists.txt "add_executable(\${PROJECT_NAME}_app"
exists "main.cpp is kept on disk, it may hold your work" main.cpp
exists "symbole.cpp is kept on disk too" symbole.cpp

title "deps: the product's dependencies"
fresh deps
check "init" "$devkit" init -k static
check "deps add pins a repository to a tag" "$devkit" deps add system -v v1.0.1
holds "the tag reaches config.yaml" config.yaml "tag: v1.0.1"
exists "a find module is generated" cmake/FindSystem.cmake
holds "the find module pins the same tag" cmake/FindSystem.cmake "set(tag v1.0.1)"
holds "find_package is called" CMakeLists.txt "find_package(System REQUIRED)"
holds "the dependency is linked to every target" CMakeLists.txt "target_link_libraries(\${target} PUBLIC \${DEVKIT_LINK})"
refuse "deps add refuses a dependency already there" "$devkit" deps add system -v v1.0.1
check "deps set changes the version" "$devkit" deps set system -v v1.0.0
holds "the new version reaches config.yaml" config.yaml "tag: v1.0.0"
holds "the find module follows" cmake/FindSystem.cmake "set(tag v1.0.0)"
check "deps set --shared changes the linkage" "$devkit" deps set system -s
holds "the linkage reaches config.yaml" config.yaml "linkage: shared"
holds "a shared dependency is copied next to the binary" CMakeLists.txt "DEVKIT_SHARED"
check "deps ls lists it" "$devkit" deps ls
holds "deps ls names the dependency" out "system"
refuse "deps rm refuses an unknown dependency" "$devkit" deps rm nowhere
check "deps rm drops it" "$devkit" deps rm system
lacks "find_package is gone" CMakeLists.txt "find_package(System REQUIRED)"
exists "the find module is kept, it may have been reworked" cmake/FindSystem.cmake

title "examples: several sandboxes, side by side"
fresh examples
check "init" "$devkit" init -k static
check "examples ls on a project without any" "$devkit" examples ls
check "examples add creates the first one" "$devkit" examples add basic
exists "the sandbox folder is created" examples/basic
exists "its own main is written" examples/basic/main.cpp
exists "its app class is named after the folder" examples/basic/BasicApp.hpp
holds "the pool of sandbox dependencies is pinned" config.yaml "icore"
holds "the sandbox target is named after the folder" CMakeLists.txt "add_executable(\${PROJECT_NAME}_basic"
check "examples add creates a second one" "$devkit" examples add collisions
exists "the second folder is independent" examples/collisions/CollisionsApp.hpp
holds "the second target appears too" CMakeLists.txt "add_executable(\${PROJECT_NAME}_collisions"
refuse "examples add refuses a name already taken" "$devkit" examples add basic
check "examples ls lists both" "$devkit" examples ls
holds "the first is listed" out "basic"
holds "the second is listed" out "collisions"
check "examples rm drops one" "$devkit" examples rm collisions
lacks "its target is gone" CMakeLists.txt "add_executable(\${PROJECT_NAME}_collisions"
exists "its folder is kept, it holds handwritten code" examples/collisions/main.cpp
refuse "examples rm refuses an unknown name" "$devkit" examples rm nowhere

title "deps -e: the sandbox pool, apart from the product"
check "deps add -e adds to the sandboxes" "$devkit" deps add igraphic -v v0.3.0 -e
check "deps ls separates the two lists" "$devkit" deps ls
holds "the product list stays empty" out "none"
holds "the sandbox dependency is listed" out "igraphic"
lacks "a sandbox dependency is not linked to the product" CMakeLists.txt "DEVKIT_LINK igraphic"
holds "it is linked to the sandboxes" CMakeLists.txt "DEVKIT_EXAMPLE_LINK"
check "deps rm -e drops it from the sandboxes" "$devkit" deps rm igraphic -e
check "deps ls again" "$devkit" deps ls
lacks "it is gone from the sandbox list" out "igraphic"

title "sync: config.yaml against a find module reworked by hand"
fresh drift
check "init" "$devkit" init -k static
check "deps add" "$devkit" deps add system -v v1.0.1
sed -i.bak 's|set(tag v1.0.1)|set(tag v1.0.0)|' cmake/FindSystem.cmake && rm -f cmake/FindSystem.cmake.bak
"$devkit" sync >out 2>&1
holds "sync reports the disagreement" out "config.yaml says v1.0.1"
holds "sync names the one the build uses" out "the build uses the module"
holds "the find module is left alone" cmake/FindSystem.cmake "set(tag v1.0.0)"
sed -i.bak 's|tag: v1.0.1|tag: v0.1.0|' config.yaml && rm -f config.yaml.bak
"$devkit" sync >out 2>&1
holds "sync calls out a downgrade" out "downgrade"
check "sync --force realigns the module" "$devkit" sync --force
holds "the module now follows config.yaml" cmake/FindSystem.cmake "set(tag v0.1.0)"

title "info, clean and the read-only commands"
fresh plain
check "init" "$devkit" init -k static shared
check "deps add" "$devkit" deps add system -v v1.0.1
check "examples add" "$devkit" examples add basic
check "info reads the project" "$devkit" info
holds "info names the project" out "plain"
holds "info shows what it produces" out "static shared"
holds "info counts the dependencies" out "1 dependency"
holds "info counts the sandboxes" out "1 sandbox"
mkdir -p build && touch build/leftover.o
check "clean empties the build folder" "$devkit" clean
exists "the build folder itself is kept" build
absent "its contents are gone" build/leftover.o
refuse "a command outside a project explains itself" sh -c "cd \"$work\" && \"$devkit\" info"

title "github: the two commands that need a token"
fresh github
check "init" "$devkit" init -k static
check "deps add" "$devkit" deps add system -v v0.1.0
if [ -n "${GITHUB_TOKEN:-}${GH_TOKEN:-}" ]; then
    check "list reaches the organisation" "$devkit" list
    holds "list names a known repository" out "ecs"
    holds "list shows its latest tag" out "v"
    check "list --url prints full addresses" "$devkit" list --url
    holds "the address is complete" out "https://github.com/P-E-R-R-Y/ecs"
    check "deps outdated compares the pins" "$devkit" deps outdated
    holds "outdated heads its two columns" out "pinned"
    holds "outdated flags the one left behind" out "<- update"
else
    refuse "list stops without a token" "$devkit" list
    holds "it says which variable to set" out "GITHUB_TOKEN"
    refuse "deps outdated stops without a token" "$devkit" deps outdated
    printf '   skip  the online cases need GITHUB_TOKEN\n'
fi

title "end to end: a class written by hand, used by a sandbox"
fresh usage
check "init as a static library" "$devkit" init -k static
cat > includes/Counter.hpp <<'HPP'
#pragma once

/** A class the user writes, nothing devkit knows about. */
class Counter {
    public:
        void tick() { _value++; }
        int value() const { return _value; }
    private:
        int _value = 0;
};
HPP
cat > sources/counter.cpp <<'CPP'
#include "Counter.hpp"

int twice(Counter &counter) { counter.tick(); counter.tick(); return counter.value(); }
CPP
sed -i.bak 's|    sources/no_source.cpp|    sources/no_source.cpp\n    sources/counter.cpp|' CMakeLists.txt && rm -f CMakeLists.txt.bak
mkdir -p tests
cat > tests/TestCounter.cpp <<'CPP'
#include "Counter.hpp"
#include <gtest/gtest.h>

int twice(Counter &counter);

TEST(Counter, CountsUp) { Counter c; c.tick(); EXPECT_EQ(c.value(), 1); }
TEST(Counter, LinksTheLibrary) { Counter c; EXPECT_EQ(twice(c), 2); }
CPP
check "examples add creates the sandbox" "$devkit" examples add demo
# the user wires their class into the generated app, as they would by hand
python3 - <<'PY'
path = "examples/demo/DemoApp.hpp"
text = open(path).read()
text = text.replace('#include <memory>', '#include "Counter.hpp"\n\n#include <memory>')
text = text.replace('            //TODO: step usage forward', '            _counter.tick();')
text = text.replace('        physics::Body', '        Counter _counter;\n        physics::Body')
text = text.replace('        graphic::IWindow2 *_window = nullptr;',
                    '        graphic::IWindow2 *_window = nullptr;\n        Counter _counter;')
open(path, "w").write(text)
PY
holds "the sandbox includes the handwritten header" examples/demo/DemoApp.hpp "Counter.hpp"
holds "the sandbox holds an instance of the class" examples/demo/DemoApp.hpp "Counter _counter;"
if build; then ok "cmake -B build -S . && cmake --build build succeeds"; else ko "the project builds" "$(grep -m2 -iE 'error' out | tr '\n' ' ')"; fi
exists "the static library is produced" build/libusage.a
exists "the sandbox binary is produced" build/usage_demo
exists "the tests binary is produced" build/usage_tests
if ctest --test-dir build >>out 2>&1; then ok "the tests pass, so the class works through the library"; else ko "the tests pass" "$(tail -3 out | tr '\n' ' ')"; fi
if nm build/usage_demo 2>/dev/null | grep -q "Counter"; then ok "the sandbox binary really carries the class"; else ko "the sandbox binary carries the class"; fi

title "end to end: the sandbox follows the library it uses"
# reset() is called by the sandbox alone: dropping it must break the sandbox
# and nothing else, which is what proves the header is really tracked
# make compares timestamps: editing a header inside the same second as the
# last build would go unnoticed, and the case would pass for the wrong reason
edit() { sleep 1; python3 -c "
import sys
path, old, new = sys.argv[1:4]
text = open(path).read()
assert old in text, path + ' lacks ' + old
open(path, 'w').write(text.replace(old, new, 1))
" "$@"; }

edit includes/Counter.hpp "void tick() { _value++; }" "void tick() { _value++; }
        void reset() { _value = 0; }"
edit examples/demo/DemoApp.hpp "_counter.tick();" "_counter.tick();
            _counter.reset();"
if build; then ok "the sandbox calls a method only it uses"; else ko "the sandbox builds with the new method" "$(grep -m1 -iE 'error' out)"; fi

edit includes/Counter.hpp "
        void reset() { _value = 0; }" ""
lacks "the method is really gone from the header" includes/Counter.hpp "void reset()"
holds "the sandbox still calls it" examples/demo/DemoApp.hpp "_counter.reset();"
if cmake --build build --target usage_demo >out 2>&1; then ko "dropping the method must stop the sandbox build"; else ok "dropping a method breaks the sandbox that uses it, at compile time"; fi
holds "the error names the sandbox" out "DemoApp.hpp"
nomatch "the library itself compiles fine, only the sandbox breaks" out "counter\\.cpp.*error"

edit includes/Counter.hpp "void tick() { _value++; }" "void tick() { _value++; }
        void reset() { _value = 0; }"
if build; then ok "putting the method back builds again"; else ko "the build recovers"; fi

# ----------------------------------------------------------------------------

printf '\n%s passed, %s failed\n' "$passed" "$failed"
exit "$failed"
