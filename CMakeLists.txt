# CMakeList.txt : CMake project for devkit, 
# - Define some C++ standard
# - Find dependencies package (static/shared)
# - copy shared libraries
# - Define the executable

cmake_minimum_required(VERSION 3.10)
project(devkit)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## ----------- Find package -----------

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# interface (static)
find_package(iinfo REQUIRED)
find_package(igraphic REQUIRED)

# shared (dynamic)
list(APPEND CMAKE_BIN_SHARED
    raygraphic 
    #sfmlgraphic
    #sdlgraphic
)
foreach(sharedlib IN LISTS CMAKE_BIN_SHARED)
    find_package(${sharedlib} REQUIRED)    
endforeach()

# more (static)
find_package(ecs REQUIRED)
find_package(engine REQUIRED)
find_package(finder REQUIRED)

## ----------- Copy assets -----------

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
message(STATUS "Copying assets")

## ------ Copy shared libraries ------

set(assetpath ${CMAKE_BINARY_DIR}/assets/shared/)

file(MAKE_DIRECTORY ${assetpath})

foreach(sharedlib IN LISTS CMAKE_BIN_SHARED)
    set (sharedlibpath ${CMAKE_BINARY_DIR}/_deps/${sharedlib}-build/lib${sharedlib}${CMAKE_SHARED_LIBRARY_SUFFIX})

    add_custom_target(copy_${sharedlib} ALL)
    add_dependencies(copy_${sharedlib} ${sharedlib})

    add_custom_command( TARGET copy_${sharedlib}
        COMMAND ${CMAKE_COMMAND} -E copy
        ${sharedlibpath}
        ${assetpath}
    )
endforeach()

## ----------- Executable -----------
set(SOURCE_FILES
    main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PUBLIC 
    ecs
    engine
    finder

    iinfo
    igraphic
)
message(STATUS "${CMAKE_BIN_SHARED}")
message(STATUS "${CMAKE_BIN_INTERFACE}")
message(STATUS "${CMAKE_BIN_STATIC}")

## ----------- Include -----------
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/includes/
)

## ----------- Compile Options -----------
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
