# ----------- general -----------

cmake_minimum_required(VERSION 3.24)
project(devkit)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# pour que l'editeur voie les memes options que le compilateur
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# ----------- dependencies -----------

find_package(Yaml REQUIRED)
find_package(CURL REQUIRED)

# ----------- executables -----------

# devkit : la ligne de commande du kit
# une action par fichier : ajouter une commande = ajouter un .cpp
file(GLOB SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sources/*.cpp)

add_executable(${PROJECT_NAME} main.cpp ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)
target_link_libraries(${PROJECT_NAME} PRIVATE yaml-cpp::yaml-cpp CURL::libcurl)
target_compile_definitions(${PROJECT_NAME} PRIVATE DEVKIT_ASSETS="${CMAKE_CURRENT_SOURCE_DIR}/assets")
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

# kubectl : l'exemple de Cli.hpp
add_executable(kubectl examples/kubectl.cpp)
target_include_directories(kubectl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)
target_compile_options(kubectl PRIVATE -Wall -Wextra)

# ----------- tests -----------

find_package(GoogleTest REQUIRED)

enable_testing()

add_executable(${PROJECT_NAME}_tests tests/TestCli.cpp)
target_include_directories(${PROJECT_NAME}_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)
target_link_libraries(${PROJECT_NAME}_tests gtest_main)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_tests)
