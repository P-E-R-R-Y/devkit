# cmake/FindYaml.cmake
#
# yaml-cpp : lecture et ecriture de YAML.
# Cible a lier : yaml-cpp::yaml-cpp

if(NOT TARGET yaml-cpp::yaml-cpp)
    include(FetchContent)
    FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG        0.8.0
    )
    # yaml-cpp 0.8.0 demande cmake 3.4, que CMake 4 refuse. On lui accorde
    # le minimum qu'il reclame, sans toucher a notre propre projet.
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

    # on ne veut que la bibliotheque
    set(YAML_CPP_BUILD_TESTS   OFF CACHE BOOL "" FORCE)
    set(YAML_CPP_BUILD_TOOLS   OFF CACHE BOOL "" FORCE)
    set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
    set(YAML_CPP_INSTALL       OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(yaml-cpp)
endif()
