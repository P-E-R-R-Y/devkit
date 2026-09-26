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
# Les trois sorties sont toujours ecrites. Celles que ce depot ne produit
# pas sont commentees : les decommenter, et ajouter la cible a
# DEVKIT_TARGETS, suffit a les activer.

# --- devkit:targets ---
# --- devkit:end ---

foreach(target IN LISTS DEVKIT_TARGETS)
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
