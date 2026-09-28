/**
 * @file sync.cpp
 * @brief Generating the project from config.yaml.
 *
 * The CMakeLists produced is the one of maths, ecs and system, to the
 * variables near. Overwrite rule: a file already present is never replaced,
 * unless --force. The blocks marked "devkit:" are the exception, they belong
 * to the tool and are regenerated on every pass.
 */

#include "Assets.hpp"
#include "Config.hpp"
#include "cmd.hpp"

#include <iostream>

namespace {

    using devkit::Config;
    using devkit::Repository;

    /**
     * @brief The object stage, then whatever this repository produces.
     *
     * The sources are compiled once, in an OBJECT library, and the outputs
     * all help themselves to the same .o.
     *
     * Only the outputs kind asks for are written. A missing output leaves
     * the file clean: nothing to uncomment, and nothing pointing at a
     * sources/main.cpp that devkit never wrote.
     *
     * Every target carries its suffix: a target name is unique across the
     * whole tree, and two add_library of the same name stop the configure.
     * OUTPUT_NAME makes the suffix invisible on disk, where everything comes
     * out under the project's name.
     *
     * The alias gives the bare name to the static one, or to the shared one
     * for want of better: repositories still writing <name> keep building.
     */
    std::string targets(const Config &config) {
        const bool shared = config.wants("shared");
        const bool app = config.wants("app");
        //with nothing asked for, a static remains: a repository always
        //produces at least one output
        const bool statique = config.wants("static") || (!shared && !app);
        const std::string objects = "$<TARGET_OBJECTS:${PROJECT_NAME}_objects>";
        std::string out;
        std::string names;

        out += "add_library(${PROJECT_NAME}_objects OBJECT ${SOURCE_FILES})\n"
               //a static may end up inside a shared one downstream: PIC from the start
               "set_target_properties(${PROJECT_NAME}_objects PROPERTIES"
               " POSITION_INDEPENDENT_CODE ON)\n"
               "\nset(DEVKIT_OBJECTS ${PROJECT_NAME}_objects)\n";

        if (statique) {
            names += " ${PROJECT_NAME}_static";
            out += "\nadd_library(${PROJECT_NAME}_static STATIC " + objects + ")\n"
                   "set_target_properties(${PROJECT_NAME}_static PROPERTIES"
                   " OUTPUT_NAME ${PROJECT_NAME})\n"
                   "add_library(${PROJECT_NAME} ALIAS ${PROJECT_NAME}_static)\n";
        }
        if (shared) {
            names += " ${PROJECT_NAME}_shared";
            //each output takes its own file: no_source.cpp feeds the object
            //stage, symbole.cpp gives the loader its entry points, main.cpp
            //gives the app its own
            out += "\nadd_library(${PROJECT_NAME}_shared SHARED " + objects + " sources/symbole.cpp)\n"
                   //hunter.dylib rather than libhunter.dylib: the file name is
                   //what serves as the loading key
                   "set_target_properties(${PROJECT_NAME}_shared PROPERTIES"
                   " OUTPUT_NAME ${PROJECT_NAME} PREFIX \"\")\n";
            if (!statique)
                out += "add_library(${PROJECT_NAME} ALIAS ${PROJECT_NAME}_shared)\n";
        }
        if (app) {
            names += " ${PROJECT_NAME}_app";
            out += "\nadd_executable(${PROJECT_NAME}_app " + objects + " sources/main.cpp)\n"
                   "set_target_properties(${PROJECT_NAME}_app PROPERTIES"
                   " OUTPUT_NAME ${PROJECT_NAME})\n";
        }
        //an alias is read-only: the loops write on the targets, so they
        //only ever take the real names
        return out + "\nset(DEVKIT_TARGETS" + names + ")\n";
    }

    /** @brief The tests block, taken as is from the other repositories. */
    std::string tests(const Config &config) {
        if (!config.tests)
            return "";

        //the tests link the library when there is one, the static by
        //preference: the shared one goes through dynamic loading
        const std::string linked = config.wants("static") ? "\n  ${PROJECT_NAME}_static"
                                 : config.wants("shared") ? "\n  ${PROJECT_NAME}_shared" : "";

        return "# ----------- tests -----------\n"
               "\nenable_testing()\n"
               "\nfile(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)\n"
               "\n#sans ce test, un dossier tests/ vide ferait echouer add_executable\n"
               "if (TEST_SOURCES)\n"
               "  add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})\n"
               "\n  target_include_directories(${PROJECT_NAME}_tests PUBLIC\n"
               "    ${CMAKE_CURRENT_SOURCE_DIR}/includes\n"
               "  )\n"
               "\n  target_link_libraries(${PROJECT_NAME}_tests\n"
               "  gtest_main" + linked + "\n"
               "  )\n"
               "\n  include(GoogleTest)\n"
               "  gtest_discover_tests(${PROJECT_NAME}_tests)\n"
               "endif()\n";
    }

    /**
     * @brief The dependencies, under the target name they carry.
     *
     * A repository moved to the convention publishes <name>_static; those not
     * there yet keep the bare name. The if (TARGET) covers both, and will
     * fall the day every P-E-R-R-Y repository has been regenerated.
     */
    std::string resolve(const std::string &list, const std::string &names) {
        return "set(" + list + " \"\")\n"
               "foreach(dep IN ITEMS " + names + ")\n"
               "  if (TARGET ${dep}_static)\n"
               "    list(APPEND " + list + " ${dep}_static)\n"
               "  else()\n"
               "    list(APPEND " + list + " ${dep})\n"
               "  endif()\n"
               "endforeach()\n";
    }

    /**
     * @brief One target per sandbox, linked to whatever --example pinned.
     *
     * examples/<name>/ gives <project>_<name>. The dependencies are shared,
     * the project's library is linked only if kind produces it - a sandbox
     * stays usable on a repository that only makes an app.
     */
    std::string example(const Config &config) {
        if (config.example.empty())
            return "";

        std::vector<std::string> vendors;
        std::string out;

        for (const Repository &repository : config.example.repositories)
            vendors.push_back(repository.name);
        out += resolve("DEVKIT_EXAMPLE_LINK", assets::join(vendors));

        for (const std::string &name : config.example.names)
            out += "\nadd_executable(${PROJECT_NAME}_" + name + " examples/" + name + "/main.cpp)\n"
                   "target_include_directories(${PROJECT_NAME}_" + name + " PRIVATE\n"
                   "  ${CMAKE_CURRENT_SOURCE_DIR}/includes\n"
                   "  ${CMAKE_CURRENT_SOURCE_DIR}/examples/" + name + "\n"
                   ")\n"
                   "target_link_libraries(${PROJECT_NAME}_" + name + " PRIVATE ${DEVKIT_EXAMPLE_LINK})\n"
                   "if (TARGET ${PROJECT_NAME}_static)\n"
                   "  target_link_libraries(${PROJECT_NAME}_" + name + " PRIVATE ${PROJECT_NAME}_static)\n"
                   "endif()\n";
        return out;
    }

    void blocks(const Config &config) {
        std::string packages;
        std::string linked;
        std::string shared;

        for (const Repository &repository : config.example.repositories)
            packages += "find_package(" + assets::capitalize(repository.name) + " REQUIRED)\n";
        for (const Repository &repository : config.repositories) {
            packages += "find_package(" + assets::capitalize(repository.name) + " REQUIRED)\n";
            if (repository.linkage == "shared")
                shared += (shared.empty() ? "" : " ") + repository.name;
            else
                linked += (linked.empty() ? "" : " ") + repository.name;
        }
        if (config.tests)
            packages += "find_package(GoogleTest REQUIRED)\n";

        assets::replaceBlock("CMakeLists.txt", "deps", packages);
        assets::replaceBlock("CMakeLists.txt", "targets", targets(config));
        assets::replaceBlock("CMakeLists.txt", "tests", tests(config));
        assets::replaceBlock("CMakeLists.txt", "example", example(config));
        assets::replaceBlock("CMakeLists.txt", "link", linked.empty() ? "" :
                             resolve("DEVKIT_LINK", linked) +
                             //the object stage needs the deps' headers to compile,
                             //the outputs need them to link
                             "\nforeach(target IN LISTS DEVKIT_OBJECTS DEVKIT_TARGETS)\n"
                             "  target_link_libraries(${target} PUBLIC ${DEVKIT_LINK})\n"
                             "endforeach()\n");
        assets::replaceBlock("CMakeLists.txt", "shared", shared.empty() ? "" :
                             "# copiees a cote du binaire, pour etre chargees a l'execution\n"
                             "set(DEVKIT_SHARED " + shared + ")\n"
                             "\nforeach(shared IN LISTS DEVKIT_SHARED)\n"
                             "  foreach(target IN LISTS DEVKIT_TARGETS)\n"
                             "    add_custom_command(TARGET ${target} POST_BUILD\n"
                             "      COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${target}>/lib\n"
                             "      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${shared}> "
                             "$<TARGET_FILE_DIR:${target}>/lib/)\n"
                             "  endforeach()\n"
                             "endforeach()\n");
    }

    /** @brief Writes a template, unless the target already exists. */
    void once(const std::string &source, const std::filesystem::path &target,
              const std::map<std::string, std::string> &values, bool force) {
        if (force || !std::filesystem::exists(target))
            assets::render(source, target, values);
    }
}

int generate(bool force) {
    if (!std::filesystem::exists(configuration)) {
        std::cerr << "no " << configuration.string() << " here, run devkit init" << std::endl;
        return 1;
    }

    Config config;

    try {
        config = Config::load(configuration.string());
    } catch (const YAML::Exception &error) {
        std::cerr << configuration.string() << " unreadable: " << error.what() << std::endl;
        return 1;
    }

    const std::map<std::string, std::string> values{
        {"name", config.name}, {"brief", config.brief},
        {"appclass", assets::capitalize(config.name) + "App"},
        {"version", config.version}, {"cmake", config.cmake},
    };

    for (const char *folder : {"includes", "sources", "cmake"})
        std::filesystem::create_directories(folder);

    once("CMakeLists.txt", "CMakeLists.txt", values, force);
    once("gitignore", ".gitignore", {}, force);

    if (config.wants("app"))
        once("main.cpp", "sources/main.cpp", values, force);
    if (config.wants("shared"))
        once("symbole.cpp", "sources/symbole.cpp", values, force);
    //the object stage refuses to live without a source, and a header-only
    //repository has none: this empty file feeds it
    if (force || !std::filesystem::exists("sources/no_source.cpp"))
        assets::write("sources/no_source.cpp", "");

    for (const std::string &name : config.example.names) {
        const std::filesystem::path folder = std::filesystem::path("examples") / name;
        //the class is named after the sandbox rather than the project: two
        //folders live side by side without treading on each other
        std::map<std::string, std::string> own = values;

        own["appclass"] = assets::capitalize(name) + "App";
        own["example"] = name;
        std::filesystem::create_directories(folder);
        once("example/App.hpp", folder / (own["appclass"] + ".hpp"), own, force);
        once("example/main.cpp", folder / "main.cpp", own, force);
    }

    if (config.tests) {
        std::filesystem::create_directories("tests");
        once("cmake/FindGoogleTest.cmake", "cmake/FindGoogleTest.cmake", {}, force);
    }

    if (config.documentation)
        for (const char *page : {"docs/Readme.md", "docs/Log.md", "docs/Topics.dox",
                                 "docs/Doxyfile", "docs/DoxygenLayout.xml"})
            once(page, page, values, force);

    if (config.cicd)
        for (const char *flow : {"tests.yml", "docs.yml"})
            once(std::string("ci/") + flow, std::filesystem::path(".github/workflows") / flow, {}, force);

    //one module per dependency, derived from config.yaml
    std::vector<Repository> all = config.repositories;

    all.insert(all.end(), config.example.repositories.begin(), config.example.repositories.end());
    //once() rather than render(): a Find reworked by hand stays in place
    //when a dependency is added later
    for (const Repository &repository : all)
        once("cmake/Find.cmake",
             std::filesystem::path("cmake") / ("Find" + assets::capitalize(repository.name) + ".cmake"),
             {{"repository", repository.name}, {"tag", repository.tag}}, force);

    blocks(config);

    return 0;
}

int sync(const cli::Call &call) {
    if (generate(call.has("force")) != 0)
        return 1;
    std::cout << "project regenerated from " << configuration.string() << std::endl;
    return 0;
}
