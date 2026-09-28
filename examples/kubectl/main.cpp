/**
 * @file kubectl.cpp
 * @brief Un kubectl miniature : l'arbre en haut, les actions en dessous.
 *
 *   kubectl get pods
 *   kubectl get po nginx -n prod -o yaml
 *   kubectl get svc -l app=web tier=front -w
 *   kubectl create deployment web --image nginx:1.27 --replicas 3
 *   kubectl delete pods nginx redis --force
 *   kubectl scale deploy web --replicas 5
 */

#include "Cli.hpp"

#include <cstdio>

/* ------------------------------------------------------------------ *
 * Les actions : chacune lit ce dont elle a besoin dans le Call.      *
 * ------------------------------------------------------------------ */

static int getPods(const cli::Call &call) {
    std::printf("pods de %s", call.value("namespace").c_str());
    if (!call.arguments.empty())
        std::printf(", nom %s", call.arguments[0].c_str());
    if (call.has("output"))
        std::printf(", format %s", call.value("output").c_str());
    std::printf(call.has("watch") ? ", en continu\n" : "\n");
    return 0;
}

static int getServices(const cli::Call &call) {
    std::printf("services de %s", call.value("namespace").c_str());
    for (const std::string &label : call.values("selector"))
        std::printf(", filtre %s", label.c_str());
    std::printf("\n");
    return 0;
}

static int createDeployment(const cli::Call &call) {
    std::printf("deploiement %s : image %s, %s replique(s), dans %s\n", call.arguments[0].c_str(),
                call.value("image").c_str(), call.value("replicas").c_str(), call.value("namespace").c_str());
    return 0;
}

static int deletePods(const cli::Call &call) {
    for (const std::string &pod : call.arguments)
        std::printf("suppression de %s%s\n", pod.c_str(), call.has("force") ? " (forcee)" : "");
    return 0;
}

static int scaleDeployment(const cli::Call &call) {
    std::printf("%s passe a %s replique(s)\n", call.arguments[0].c_str(), call.value("replicas").c_str());
    return 0;
}

/* ------------------------------------------------------------------ *
 * L'arbre : toute la ligne de commande, declaree en une fois.        *
 * ------------------------------------------------------------------ */

static const cli::Command kubectl{
    .name = "kubectl",
    .help = "pilote un cluster Kubernetes",
    .options = {
        // declaree a la racine : valable pour toutes les commandes
        {.name = "namespace", .alias = "n", .help = "espace de noms", .fallback = "default"},
    },
    .commands = {
        {
            .name = "get",
            .help = "affiche des ressources",
            .options = {
                {.name = "output", .alias = "o", .help = "format de sortie", .choices = {"wide", "yaml", "json"}},
                {.name = "watch", .alias = "w", .help = "suit les changements", .arity = cli::Arity::Flag},
                {.name = "selector", .alias = "l", .help = "filtres par label", .arity = cli::Arity::Many},
            },
            .commands = {
                {.name = "pods", .help = "les pods", .aliases = {"po", "pod"},
                 .arguments = {.name = "nom", .max = 1}, .run = getPods},
                {.name = "services", .help = "les services", .aliases = {"svc"},
                 .arguments = {.name = "nom", .max = 1}, .run = getServices},
            },
        },
        {
            .name = "create",
            .help = "cree une ressource",
            .commands = {
                {.name = "deployment", .help = "un deploiement", .aliases = {"deploy"},
                 .options = {
                     {.name = "image", .help = "image du conteneur", .required = true},
                     {.name = "replicas", .help = "nombre de repliques", .fallback = "1"},
                 },
                 .arguments = {.name = "nom", .min = 1, .max = 1}, .run = createDeployment},
            },
        },
        {
            .name = "delete",
            .help = "supprime des ressources",
            .commands = {
                {.name = "pods", .help = "un ou plusieurs pods", .aliases = {"po", "pod"},
                 .options = {{.name = "force", .help = "sans attendre", .arity = cli::Arity::Flag}},
                 .arguments = {.name = "nom", .min = 1, .max = cli::many}, .run = deletePods},
            },
        },
        {
            .name = "scale",
            .help = "change le nombre de repliques",
            .commands = {
                {.name = "deployment", .help = "un deploiement", .aliases = {"deploy"},
                 .options = {{.name = "replicas", .help = "nombre voulu", .required = true}},
                 .arguments = {.name = "nom", .min = 1, .max = 1}, .run = scaleDeployment},
            },
        },
    },
};

int main(int argc, char **argv) {
    return cli::execute(kubectl, argc, argv);
}
