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
# everything under sources/ belongs to the object stage, save the two files
# that belong to one output. Drop a .cpp in and it is picked up.

file(GLOB SOURCE_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sources/*.cpp)
list(REMOVE_ITEM SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/symbole.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/main.cpp
)

#----------- targets -----------
# The sources are compiled once, in an OBJECT library, and every output
# helps itself to the same .o.
#
# Only what kind asks for is written. To add an output, add it to kind in
# config.yaml: devkit regenerates on the spot.

# --- devkit:targets ---
# --- devkit:end ---

# the object stage compiles the sources: it needs the same includes and the
# same options as the outputs. $<TARGET_OBJECTS:> carries nothing but the .o,
# so the outputs receive them too, for whoever consumes them.
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

# --- devkit:example ---
# --- devkit:end ---

# --- devkit:tests ---
# --- devkit:end ---
