/**
 * @file symbole.cpp
 * @brief The symbols the loader sees of {{name}}.
 *
 * This file belongs to the shared library alone: it is the only surface
 * dlsym reaches. A raw symbol name is what it looks for, it knows neither
 * namespaces nor C++ mangling, hence the extern "C" and the global scope.
 *
 * Everything else of {{name}} lives in includes/, compiled once by the
 * object stage, and ends up in the static as well as in the shared one.
 *
 * @addtogroup {{name}}
 * @{
 */

extern "C" {

    //TODO: what the loader must find here

}

/** @} */
