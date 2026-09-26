/**
 * @file build.cpp
 * @brief devkit build : genere le projet depuis config.yaml.
 *
 * Le CMakeLists produit est celui de maths, ecs et system, aux variables
 * pres. Regle d'ecrasement : un fichier deja present n'est jamais remplace,
 * sauf avec --force. Les blocs marques "devkit:" font exception, ils
 * appartiennent a l'outil et sont regeneres a chaque passage.
 */

#include "Assets.hpp"
#include "Config.hpp"
#include "cmd.hpp"

#include <iostream>

namespace {

    using devkit::Config;
    using devkit::Repository;

    /**
     * @brief Les trois sorties, toujours ecrites, les inactives commentees.
     *
     * Le fichier est donc le meme d'un depot a l'autre : seule la marque de
     * commentaire change, et DEVKIT_TARGETS dit ce qui est actif.
     */
    std::string targets(const Config &config) {
        const bool statique = config.wants("static");
        const bool shared = config.wants("shared");
        const bool app = config.wants("app");
        const auto mark = [](bool active) { return active ? "" : "#"; };
        std::string names;

        if (statique) names += " ${PROJECT_NAME}";
        if (shared)   names += " ${PROJECT_NAME}_shared";
        if (app)      names += " ${PROJECT_NAME}_app";

        return std::string(mark(statique)) + "add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})\n"
             + mark(shared) + "add_library(${PROJECT_NAME}_shared SHARED ${SOURCE_FILES})\n"
             //hunter.dylib plutot que libhunter.dylib : c'est le nom de fichier
             //qui sert de clef de chargement
             + mark(shared) + "set_target_properties(${PROJECT_NAME}_shared PROPERTIES"
                              " OUTPUT_NAME ${PROJECT_NAME} PREFIX \"\")\n"
             //avec une statique, l'app n'apporte que son main ; sans elle,
             //elle compile aussi les sources du depot
             + mark(app && statique) + "add_executable(${PROJECT_NAME}_app sources/main.cpp)\n"
             + mark(app && !statique) + "add_executable(${PROJECT_NAME}_app sources/main.cpp ${SOURCE_FILES})\n"
             + mark(app) + "set_target_properties(${PROJECT_NAME}_app PROPERTIES OUTPUT_NAME ${PROJECT_NAME})\n"
             //l'executable n'apporte que son main : le reste vient de la bibliotheque
             + mark(app && statique) + "target_link_libraries(${PROJECT_NAME}_app PRIVATE ${PROJECT_NAME})\n"
             + "\nset(DEVKIT_TARGETS" + (names.empty() ? " ${PROJECT_NAME}" : names) + ")\n";
    }

    /** @brief Le bloc de tests, repris tel quel des autres depots. */
    std::string tests(const Config &config) {
        if (!config.tests)
            return "";

        //les tests se lient a la bibliotheque quand il y en a une, statique
        //de preference : la partagee porte un autre nom de cible
        const std::string linked = config.wants("static") ? "\n  ${PROJECT_NAME}"
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

    /** @brief Les trois blocs que devkit possede dans le CMakeLists. */
    /**
     * @brief La cible du bac a sable, liee a ce que --example a epingle.
     *
     * Les deux if (TARGET) evitent a devkit de savoir : un vendor publie
     * souvent <nom>_static a cote de sa version partagee, et la statique du
     * projet n'existe que si kind la demande.
     */
    std::string example(const Config &config) {
        if (config.example.empty())
            return "";

        std::string out = "add_executable(${PROJECT_NAME}_example example/main.cpp)\n"
                          "\ntarget_include_directories(${PROJECT_NAME}_example PRIVATE\n"
                          "  ${CMAKE_CURRENT_SOURCE_DIR}/includes\n"
                          "  ${CMAKE_CURRENT_SOURCE_DIR}/example\n"
                          ")\n";

        for (const Repository &repository : config.example)
            out += "\nset(_vendor " + repository.name + ")\n"
                   "if (TARGET ${_vendor}_static)\n"
                   "  set(_vendor ${_vendor}_static)\n"
                   "endif()\n"
                   "target_link_libraries(${PROJECT_NAME}_example PRIVATE ${_vendor})\n"
                   "unset(_vendor)\n";

        //la bibliotheque du projet seulement si elle existe : la plupart des
        //depots P-E-R-R-Y sont en en-tetes seuls, includes/ suffit alors
        out += "\nif (TARGET ${PROJECT_NAME})\n"
               "  target_link_libraries(${PROJECT_NAME}_example PRIVATE ${PROJECT_NAME})\n"
               "endif()\n";
        return out;
    }

    void blocks(const Config &config) {
        std::string packages;
        std::string linked;
        std::string shared;

        for (const Repository &repository : config.example)
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
                             "foreach(target IN LISTS DEVKIT_TARGETS)\n"
                             "  target_link_libraries(${target} PUBLIC " + linked + ")\n"
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

    /** @brief Ecrit un gabarit, sauf si la cible existe deja. */
    void once(const std::string &source, const std::filesystem::path &target,
              const std::map<std::string, std::string> &values, bool force) {
        if (force || !std::filesystem::exists(target))
            assets::render(source, target, values);
    }
}

int build(const cli::Call &call) {
    if (!std::filesystem::exists(configuration)) {
        std::cerr << "aucun " << configuration.string() << " ici, lance devkit init" << std::endl;
        return 1;
    }

    Config config;

    try {
        config = Config::load(configuration.string());
    } catch (const YAML::Exception &error) {
        std::cerr << configuration.string() << " illisible : " << error.what() << std::endl;
        return 1;
    }

    const bool force = call.has("force");
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
    if ((config.wants("static") || config.wants("shared"))
        && (force || !std::filesystem::exists("sources/no_source.cpp")))
        assets::write("sources/no_source.cpp", "");

    if (!config.example.empty()) {
        std::filesystem::create_directories("example");
        once("example/App.hpp", std::filesystem::path("example") /
             (assets::capitalize(config.name) + "App.hpp"), values, force);
        once("example/main.cpp", "example/main.cpp", values, force);
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

    //un module par dependance, toujours reecrit : il derive de config.yaml
    std::vector<Repository> all = config.repositories;

    all.insert(all.end(), config.example.begin(), config.example.end());
    for (const Repository &repository : all)
        if (!assets::render("cmake/Find.cmake",
                       std::filesystem::path("cmake") / ("Find" + assets::capitalize(repository.name) + ".cmake"),
                            {{"repository", repository.name}, {"tag", repository.tag}}))
            return 1;

    blocks(config);

    std::cout << config.name << " " << config.version << " genere ("
              << config.repositories.size() << " dependance(s))" << std::endl;
    return 0;
}
