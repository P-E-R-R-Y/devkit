#----------- general -----------

cmake_minimum_required(VERSION {{cmake}})
project({{name}})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#----------- dependencies -----------

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# --- devkit:deps ---
# --- devkit:end ---

#----------- sources -----------

set(SOURCE_FILES
    sources/no_source.cpp
)

#----------- targets -----------
# Les sources sont compilees une seule fois, dans une bibliotheque OBJECT,
# et les sorties se servent des memes .o.
#
# Seul ce que kind demande est ecrit. Pour ajouter une sortie, ajoutez-la a
# kind dans config.yaml et relancez devkit build.

# --- devkit:targets ---
# --- devkit:end ---

# l'etage objet compile les sources : il lui faut les memes includes et les
# memes options que les sorties. $<TARGET_OBJECTS:> ne transporte que les .o,
# donc les sorties les recoivent aussi, pour ceux qui les consommeront.
foreach(target IN LISTS DEVKIT_OBJECTS DEVKIT_TARGETS)
  target_include_directories(${target} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
  )

  target_compile_options(${target} PRIVATE -Wall -Wextra)
endforeach()

# --- devkit:link ---
# --- devkit:end ---

# --- devkit:shared ---
# --- devkit:end ---

#----------- example -----------
# Le bac a sable : son propre main, dans example/, pour ne pas croiser
# celui du produit.

# --- devkit:example ---
# --- devkit:end ---

# --- devkit:tests ---
# --- devkit:end ---
