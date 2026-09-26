#include "Cli.hpp"

#include <gtest/gtest.h>

using List = std::vector<std::string>;

namespace {

    /** @brief Un arbre a la kubectl, dont chaque action garde le Call recu. */
    struct Fixture : ::testing::Test {
        cli::Call last;

        cli::Command tree{
            .name = "k",
            .options = {{.name = "namespace", .alias = "n", .fallback = "default"}},
            .commands = {
                {.name = "get",
                 .options = {
                     {.name = "output", .alias = "o", .choices = {"yaml", "json"}},
                     {.name = "watch", .alias = "w", .arity = cli::Arity::Flag},
                     {.name = "label", .alias = "l", .arity = cli::Arity::Many},
                 },
                 .commands = {
                     {.name = "pods", .aliases = {"po"}, .arguments = {.max = 1}, .run = keep()},
                 }},
                {.name = "delete",
                 .commands = {
                     {.name = "pods", .arguments = {.min = 1, .max = cli::many}, .run = keep()},
                 }},
                {.name = "scale",
                 .options = {{.name = "replicas", .required = true}},
                 .arguments = {.min = 1, .max = 1, .choices = {"web", "api"}},
                 .run = keep()},
            },
        };

        /** @brief Une action qui retient le Call recu. */
        cli::Function keep() {
            return [this](const cli::Call &call) { last = call; return 0; };
        }

        int run(std::vector<const char *> argv) {
            argv.insert(argv.begin(), "k");
            return cli::execute(tree, static_cast<int>(argv.size()), argv.data());
        }
    };
}

TEST_F(Fixture, DescendThroughCommands) {
    ASSERT_EQ(run({"get", "pods", "nginx"}), 0);
    EXPECT_EQ(last.path, List({"get", "pods"}));
    EXPECT_EQ(last.arguments, List{"nginx"});
}

TEST_F(Fixture, AliasResolvesToName) {
    ASSERT_EQ(run({"get", "po"}), 0);
    EXPECT_EQ(last.path, List({"get", "pods"}));
}

TEST_F(Fixture, OptionsGoAnywhereAfterTheirCommand) {
    ASSERT_EQ(run({"-n", "prod", "get", "pods", "nginx", "-o", "yaml"}), 0);
    EXPECT_EQ(last.value("namespace"), "prod");
    EXPECT_EQ(last.value("output"), "yaml");
    EXPECT_EQ(last.arguments, List{"nginx"});
}

TEST_F(Fixture, OneTakesExactlyOneValue) {
    ASSERT_EQ(run({"get", "pods", "-n", "prod", "nginx"}), 0);
    EXPECT_EQ(last.value("namespace"), "prod");
    EXPECT_EQ(last.arguments, List{"nginx"});
}

TEST_F(Fixture, FlagTakesNoValue) {
    ASSERT_EQ(run({"get", "-w", "pods"}), 0);
    EXPECT_TRUE(last.has("watch"));
    EXPECT_EQ(last.path, List({"get", "pods"}));
}

TEST_F(Fixture, ManyTakesUntilNextOption) {
    ASSERT_EQ(run({"get", "pods", "-l", "app=web", "tier=front", "-o=json"}), 0);
    EXPECT_EQ(last.values("label"), List({"app=web", "tier=front"}));
    EXPECT_EQ(last.value("output"), "json");
}

TEST_F(Fixture, LastOccurrenceWins) {
    ASSERT_EQ(run({"get", "pods", "-n", "a", "--namespace", "b"}), 0);
    EXPECT_EQ(last.value("namespace"), "b");
}

TEST_F(Fixture, FallbackFillsAbsentOption) {
    ASSERT_EQ(run({"get", "pods"}), 0);
    EXPECT_EQ(last.value("namespace"), "default");
}

TEST_F(Fixture, NegativeNumberIsAValue) {
    ASSERT_EQ(run({"scale", "web", "--replicas", "-1"}), 0);
    EXPECT_EQ(last.value("replicas"), "-1");
}

TEST_F(Fixture, UnboundedArguments) {
    ASSERT_EQ(run({"delete", "pods", "a", "b", "c"}), 0);
    EXPECT_EQ(last.arguments.size(), 3u);
}

TEST_F(Fixture, Errors) {
    EXPECT_EQ(run({"get", "deployments"}), 1);             // commande inconnue
    EXPECT_EQ(run({"get", "pods", "-z"}), 1);              // option inconnue
    EXPECT_EQ(run({"get", "pods", "-n"}), 1);              // valeur manquante
    EXPECT_EQ(run({"get", "pods", "-o", "xml"}), 1);       // valeur hors choix
    EXPECT_EQ(run({"get", "pods", "a", "b"}), 1);          // argument en trop
    EXPECT_EQ(run({"delete", "pods"}), 1);                 // argument manquant
    EXPECT_EQ(run({"scale", "web"}), 1);                   // option requise
    EXPECT_EQ(run({"scale", "db", "--replicas", "2"}), 1); // argument hors choix
    EXPECT_EQ(run({"-o", "yaml", "get", "pods"}), 1);      // option avant sa commande
}

TEST_F(Fixture, HelpWinsOverMissingRequirements) {
    EXPECT_EQ(run({"scale", "--help"}), 0);
}

TEST_F(Fixture, ShortHelp) {
    EXPECT_EQ(run({"get", "-h"}), 0);
    EXPECT_EQ(run({"scale", "-h"}), 0);
}

TEST(Cli, DeclaredAliasTakesPrecedenceOverShortHelp) {
    cli::Call last;
    const cli::Command tree{
        .name = "k",
        .options = {{.name = "host", .alias = "h"}},
        .run = [&](const cli::Call &call) { last = call; return 0; },
    };
    const char *byAlias[] = {"k", "-h", "localhost"};
    const char *byName[] = {"k", "--help"};

    ASSERT_EQ(cli::execute(tree, 3, byAlias), 0);
    EXPECT_EQ(last.value("host"), "localhost");
    EXPECT_FALSE(last.has("help"));
    EXPECT_EQ(cli::execute(tree, 2, byName), 0);     // l'aide reste joignable
}
