/**
 * @file Cli.hpp
 * @brief A cobra-like command line, declared as data.
 *
 * The whole tree is written in a single initializer: each Command carries its
 * options, its arguments and its sub-commands. cli::execute() walks it.
 *
 * - a word naming a sub-command, or one of its aliases, descends the tree;
 *   every other word is an argument;
 * - an option holds for the command declaring it and for its descendants,
 *   and may sit anywhere after it;
 * - Flag: no value, One: exactly one, Many: until the next option;
 * - repeated, an option overrides the previous one; "-5" is a value.
 *
 * @addtogroup devkit
 * @{
 */

#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <type_traits>
#include <map>
#include <string>
#include <vector>

namespace cli {

    /** @brief No limit, for Arguments::max. */
    inline constexpr std::size_t many = SIZE_MAX;

    enum class Arity { 
        Flag, //< no value, presence = true
        One,  //< one value, presence = true
        Many  //< zero or more values, presence = true
    };

    struct Option {
        std::string name;
        std::string alias = "";                  ///< "n" for -n
        std::string help = "";
        Arity arity = Arity::One;
        std::vector<std::string> choices = {};   ///< empty: any value goes
        std::string fallback = "";               ///< the value when the option is absent
        bool required = false;
    };

    /** @brief The free words a command accepts. None by default. */
    struct Arguments {
        std::string name = "argument";           ///< for the help: <name>
        std::size_t min = 0;
        std::size_t max = 0;
        std::vector<std::string> choices = {};
    };

    /** @brief What a command receives when it runs. */
    struct Call {
        std::vector<std::string> path;           ///< get pods, aliases resolved
        std::vector<std::string> arguments;      ///< nginx redis
        std::map<std::string, std::vector<std::string>> options;

        bool has(const std::string &name) const { return options.count(name) != 0; }

        const std::vector<std::string> &values(const std::string &name) const {
            static const std::vector<std::string> none;
            const auto found = options.find(name);

            return found == options.end() ? none : found->second;
        }

        std::string value(const std::string &name) const {
            const auto &all = values(name);

            return all.empty() ? "" : all.front();
        }
    };

    using Function = std::function<int(const Call &)>;

    struct Command {
        std::string name;
        std::string help = "";
        std::vector<std::string> aliases = {};
        std::vector<Option> options = {};
        Arguments arguments = {};
        std::vector<Command> commands = {};
        Function run = nullptr;
    };

    namespace detail {

        using Path = std::vector<const Command *>;

        inline bool isOption(const std::string &token) {
            return token.size() > 1 && token[0] == '-' &&
                   !std::isdigit(static_cast<unsigned char>(token[1])) && token[1] != '.';
        }

        inline bool contains(const std::vector<std::string> &list, const std::string &word) {
            return std::find(list.begin(), list.end(), word) != list.end();
        }

        inline std::string join(const std::vector<std::string> &list, const char *separator) {
            std::string out;

            for (const std::string &word : list)
                out += (out.empty() ? "" : separator) + word;
            return out;
        }

        inline std::string fullName(const Path &path) {
            std::string out;

            for (const Command *c : path)
                out += (out.empty() ? "" : " ") + c->name;
            return out;
        }

        inline const Command *child(const Command &parent, const std::string &word) {
            for (const Command &c : parent.commands)
                if (c.name == word || contains(c.aliases, word))
                    return &c;
            return nullptr;
        }

        /** @brief The help, known to every command. */
        inline const Option help{.name = "help", .alias = "h", .help = "this help", .arity = Arity::Flag};

        /**
         * @brief From the nearest to the farthest, then the help.
         *
         * The help comes last: a command declaring its own -h keeps it, and
         * the help stays reachable through --help.
         */
        inline const Option *find(const Path &path, const std::string &key) {
            const bool brief = key.rfind("--", 0) != 0;
            const std::string bare = key.substr(brief ? 1 : 2);

            for (auto node = path.rbegin(); node != path.rend(); ++node)
                for (const Option &o : (*node)->options)
                    if ((brief ? o.alias : o.name) == bare)
                        return &o;
            return (brief ? help.alias : help.name) == bare ? &help : nullptr;
        }

        /** @brief Sort the words. Returns the first error, "" otherwise. */
        inline std::string parse(int argc, const char *const *argv, Path &path, Call &call) {
            const auto isValue = [&](int at) { return at < argc && !isOption(argv[at]); };

            for (int i = 1; i < argc; i++) {
                const std::string token = argv[i];

                if (!isOption(token)) {
                    //we descend as long as no argument has shown up
                    const Command *next = call.arguments.empty() ? child(*path.back(), token) : nullptr;

                    if (next) {
                        path.push_back(next);
                        call.path.push_back(next->name);
                    } else
                        call.arguments.push_back(token);
                    continue;
                }

                const std::size_t equal = token.find('=');
                const std::string key = token.substr(0, equal);
                const Option *option = find(path, key);

                if (!option)
                    return "unknown option: " + key;

                auto &values = call.options[option->name];

                values.clear();
                if (equal != std::string::npos)
                    values.push_back(token.substr(equal + 1));
                else if (option->arity == Arity::One) {
                    if (!isValue(i + 1))
                        return "missing value: " + key;
                    values.push_back(argv[++i]);
                }
                if (option->arity == Arity::Many)
                    while (isValue(i + 1))
                        values.push_back(argv[++i]);
            }
            return "";
        }

        /** @brief The first value outside the allowed choices, "" if all is well. */
        inline std::string outside(const std::vector<std::string> &values,
                                   const std::vector<std::string> &choices) {
            if (choices.empty())
                return "";
            for (const std::string &value : values)
                if (!contains(choices, value))
                    return value;
            return "";
        }

        /** @brief Required options, imposed values, defaults, argument count. */
        inline std::string check(const Path &path, Call &call) {
            for (const Command *node : path)
                for (const Option &o : node->options) {
                    const auto given = call.options.find(o.name);

                    if (given == call.options.end()) {
                        if (o.required)
                            return "required option: --" + o.name;
                        if (!o.fallback.empty())
                            call.options[o.name] = {o.fallback};
                        continue;
                    }
                    const std::string bad = outside(given->second, o.choices);

                    if (!bad.empty())
                        return "--" + o.name + " does not take " + bad + " (expected: " + join(o.choices, ", ") + ")";
                }

            const Arguments &expected = path.back()->arguments;
            const std::vector<std::string> &given = call.arguments;

            if (given.size() < expected.min)
                return path.back()->name + " expects at least " + std::to_string(expected.min) + " <" + expected.name + ">";
            if (given.size() > expected.max)
                return "extra argument: " + given[expected.max];

            const std::string bad = outside(given, expected.choices);

            return bad.empty() ? "" : bad + " is not a valid " + expected.name +
                                      " (expected: " + join(expected.choices, ", ") + ")";
        }

        inline std::string signature(const Option &o) {
            if (o.arity == Arity::Flag)
                return "";
            const std::string inside = o.choices.empty() ? "value" : join(o.choices, "|");

            return " <" + inside + (o.arity == Arity::Many ? "...>" : ">");
        }

        inline void usage(const Path &path) {
            const Command &c = *path.back();
            const Arguments &a = c.arguments;
            const std::string name = fullName(path);

            std::printf("%s%s%s\n\nusage: %s", name.c_str(), c.help.empty() ? "" : " - ", c.help.c_str(), name.c_str());
            if (!c.commands.empty())
                std::printf(" <command>");
            if (a.max > 0)
                std::printf(a.min > 0 ? " <%s%s>" : " [%s%s]", a.name.c_str(), a.max > 1 ? "..." : "");
            std::printf(" [options]\n");

            if (!c.commands.empty())
                std::printf("\ncommands:\n");
            for (const Command &sub : c.commands) {
                const std::string label = sub.name + (sub.aliases.empty() ? "" : " (" + join(sub.aliases, ", ") + ")");

                std::printf("  %-24s %s\n", label.c_str(), sub.help.c_str());
            }

            std::printf("\noptions:\n");
            for (const Command *node : path)
                for (const Option &o : node->options) {
                    const std::string alias = o.alias.empty() ? "" : "-" + o.alias + ",";
                    const std::string note = o.required ? " (required)"
                                           : o.fallback.empty() ? "" : " (default: " + o.fallback + ")";

                    std::printf("  %-3s --%-28s %s%s\n", alias.c_str(), (o.name + signature(o)).c_str(),
                                o.help.c_str(), note.c_str());
                }
            std::printf("  %-3s --%-28s %s\n", find(path, "-h") == &help ? "-h," : "", "help", "this help");
        }

        inline int fail(const Path &path, const std::string &error) {
            std::fprintf(stderr, "error: %s\nsee: %s --help\n", error.c_str(), fullName(path).c_str());
            return 1;
        }
    }

    /**
     * @brief Parse, check, then run the command that was reached.
     *
     * @return the action's code, 1 on error
     */
    inline int execute(const Command &root, int argc, const char *const *argv) {
        detail::Path path{&root};
        Call call;

        if (const std::string error = detail::parse(argc, argv, path, call); !error.empty())
            return detail::fail(path, error);

        const Command &target = *path.back();

        if (call.has("help")) {
            detail::usage(path);
            return 0;
        }
        if (!target.run) {
            if (!call.arguments.empty())
                return detail::fail(path, "unknown command: " + call.arguments.front());
            detail::usage(path);
            return argc > 1 ? 1 : 0;   //the bare command prints the help without complaining
        }
        if (const std::string error = detail::check(path, call); !error.empty())
            return detail::fail(path, error);
        return target.run(call);
    }
}

/** @} */
