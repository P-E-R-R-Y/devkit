#include "cmd.hpp"

#include <iostream>

int clean(const cli::Call &) {
    std::cout << "nettoyage..." << std::endl;
    return 0;
}
