#----------- general -----------

cmake_minimum_required(VERSION 3.24)
project(devkit)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CUSTOM : pour que l'editeur voie les memes options que le compilateur
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#----------- dependencies -----------

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# --- devkit:deps ---
find_package(Yaml REQUIRED)
find_package(CURL REQUIRED)
find_package(GoogleTest REQUIRED)
# --- devkit:end ---

#----------- sources -----------

# une action par fichier : ajouter une commande = ajouter un .cpp. main.cpp
# en sort, il appartient a l'executable et pas a l'etage objet.
file(GLOB SOURCE_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sources/*.cpp)
list(REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/sources/main.cpp)

#----------- targets -----------
# Les sources sont compilees une seule fois, dans une bibliotheque OBJECT,
# et les sorties se servent des memes .o. devkit ne produit qu'un binaire,
# il n'y a donc qu'une sortie ici.

# --- devkit:targets ---
add_library(${PROJECT_NAME}_objects OBJECT ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME}_objects PROPERTIES POSITION_INDEPENDENT_CODE ON)

set(DEVKIT_OBJECTS ${PROJECT_NAME}_objects)

add_executable(${PROJECT_NAME}_app $<TARGET_OBJECTS:${PROJECT_NAME}_objects> sources/main.cpp)
set_target_properties(${PROJECT_NAME}_app PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

set(DEVKIT_TARGETS ${PROJECT_NAME}_app)
# --- devkit:end ---

# l'etage objet compile les sources : il lui faut les memes includes et les
# memes options que les sorties. $<TARGET_OBJECTS:> ne transporte que les .o,
# donc les sorties les recoivent aussi, pour ceux qui les consommeront.
foreach(target IN LISTS DEVKIT_OBJECTS DEVKIT_TARGETS)
  target_include_directories(${target} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
  )

  target_compile_options(${target} PRIVATE -Wall -Wextra)

  # CUSTOM : ou trouver les gabarits quand devkit tourne depuis son build.
  # Assets.hpp les cherche d'abord dans $DEVKIT_ASSETS, puis ici, puis dans
  # ~/.local/share/devkit/assets.
  target_compile_definitions(${target} PRIVATE
    DEVKIT_ASSETS="${CMAKE_CURRENT_SOURCE_DIR}/assets"
  )
endforeach()

# --- devkit:link ---
# --- devkit:end ---

# CUSTOM : deux dependances externes, aux cibles deja nommees par leur
# paquet. La resolution <dep>_static de devkit ne s'applique qu'aux depots
# P-E-R-R-Y, qui portent la convention.
foreach(target IN LISTS DEVKIT_OBJECTS DEVKIT_TARGETS)
  target_link_libraries(${target} PUBLIC yaml-cpp::yaml-cpp CURL::libcurl)
endforeach()

# --- devkit:shared ---
# --- devkit:end ---

#----------- example -----------
# Un dossier par bac a sable, chacun avec son main, pour ne pas croiser
# celui du produit. kubectl fait tourner Cli.hpp.

# --- devkit:example ---
add_executable(${PROJECT_NAME}_kubectl examples/kubectl/main.cpp)
target_include_directories(${PROJECT_NAME}_kubectl PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/includes
  ${CMAKE_CURRENT_SOURCE_DIR}/examples/kubectl
)
# --- devkit:end ---

# --- devkit:tests ---
# ----------- tests -----------

enable_testing()

file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)

#sans ce test, un dossier tests/ vide ferait echouer add_executable
if (TEST_SOURCES)
  add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})

  target_include_directories(${PROJECT_NAME}_tests PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
  )

  target_link_libraries(${PROJECT_NAME}_tests
  gtest_main
  )

  include(GoogleTest)
  gtest_discover_tests(${PROJECT_NAME}_tests)
endif()
# --- devkit:end ---
