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

#----------- example -----------
# The sandboxes: one folder each under examples/, with their own main, so
# that none of them crosses the product's.

# --- devkit:example ---
# --- devkit:end ---

# --- devkit:tests ---
# --- devkit:end ---
