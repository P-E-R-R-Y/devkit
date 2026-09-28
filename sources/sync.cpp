/**
 * @file sync.cpp
 * @brief La generation du projet depuis config.yaml.
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
     * @brief L'etage objet, puis ce que ce depot produit.
     *
     * Les sources sont compilees une seule fois, dans une bibliotheque
     * OBJECT, et les sorties se servent des memes .o.
     *
     * Seules les sorties demandees par kind sont ecrites. Une sortie
     * absente laisse le fichier propre : rien a decommenter, et rien qui
     * renvoie a un sources/main.cpp que devkit n'a pas ecrit.
     *
     * Chaque cible porte son suffixe : un nom de cible est unique pour tout
     * l'arbre, et deux add_library du meme nom arretent le configure.
     * OUTPUT_NAME rend le suffixe invisible sur le disque, ou tout sort
     * sous le nom du projet.
     *
     * L'alias donne le nom nu a la statique, ou a la partagee faute de
     * mieux : les depots qui ecrivent encore <nom> continuent de compiler.
     */
    std::string targets(const Config &config) {
        const bool shared = config.wants("shared");
        const bool app = config.wants("app");
        //sans rien de demande, il reste une statique : un depot produit
        //toujours au moins une sortie
        const bool statique = config.wants("static") || (!shared && !app);
        const std::string objects = "$<TARGET_OBJECTS:${PROJECT_NAME}_objects>";
        std::string out;
        std::string names;

        out += "add_library(${PROJECT_NAME}_objects OBJECT ${SOURCE_FILES})\n"
               //une statique peut finir dans une partagee en aval : PIC des le depart
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
            //chaque sortie prend son fichier : no_source.cpp nourrit l'etage
            //objet, symbole.cpp donne au chargeur ses points d'entree,
            //main.cpp donne a l'app le sien
            out += "\nadd_library(${PROJECT_NAME}_shared SHARED " + objects + " sources/symbole.cpp)\n"
                   //hunter.dylib plutot que libhunter.dylib : c'est le nom de
                   //fichier qui sert de clef de chargement
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
        //l'alias est en lecture seule : les boucles ecrivent sur les cibles,
        //elles ne prennent donc que les vrais noms
        return out + "\nset(DEVKIT_TARGETS" + names + ")\n";
    }

    /** @brief Le bloc de tests, repris tel quel des autres depots. */
    std::string tests(const Config &config) {
        if (!config.tests)
            return "";

        //les tests se lient a la bibliotheque quand il y en a une, statique
        //de preference : la partagee passe par le chargement dynamique
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
     * @brief Les dependances sous le nom de cible qu'elles portent.
     *
     * Un depot passe a la convention publie <nom>_static ; ceux qui n'y sont
     * pas encore gardent le nom nu. Le if (TARGET) couvre les deux, et
     * tombera quand tous les depots P-E-R-R-Y auront ete regeneres.
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
     * @brief Une cible par bac a sable, liee a ce que --example a epingle.
     *
     * examples/<nom>/ donne <projet>_<nom>. Les dependances sont communes,
     * la bibliotheque du projet n'est liee que si kind la produit - un bac
     * a sable reste utilisable sur un depot qui ne fait qu'une app.
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
                             //l'etage objet a besoin des en-tetes des deps pour
                             //compiler, les sorties en ont besoin pour lier
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

    /** @brief Ecrit un gabarit, sauf si la cible existe deja. */
    void once(const std::string &source, const std::filesystem::path &target,
              const std::map<std::string, std::string> &values, bool force) {
        if (force || !std::filesystem::exists(target))
            assets::render(source, target, values);
    }
}

int generate(bool force) {
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
    //l'etage objet refuse de vivre sans source, et un depot en en-tetes
    //seuls n'en a aucune : ce fichier vide le nourrit
    if (force || !std::filesystem::exists("sources/no_source.cpp"))
        assets::write("sources/no_source.cpp", "");

    for (const std::string &name : config.example.names) {
        const std::filesystem::path folder = std::filesystem::path("examples") / name;
        //la classe porte le nom du bac a sable, pas celui du projet : deux
        //dossiers cohabitent sans se marcher dessus
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

    //un module par dependance, toujours reecrit : il derive de config.yaml
    std::vector<Repository> all = config.repositories;

    all.insert(all.end(), config.example.repositories.begin(), config.example.repositories.end());
    //once() et non render() : un Find bricole a la main reste en place quand
    //on ajoute une dependance plus tard
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
    std::cout << "projet regenere depuis " << configuration.string() << std::endl;
    return 0;
}
