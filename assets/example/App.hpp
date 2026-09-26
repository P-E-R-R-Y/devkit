/**
 * @file {{appclass}}.hpp
 * @brief Le bac a sable : de quoi voir tourner {{name}}.
 *
 * Ecrit une seule fois par devkit, puis il est a toi. Taille dedans.
 */

#pragma once

#include "ICore.hpp"
#include "RayGraphicModule.hpp"

#include <memory>

class {{appclass}} : public IApp {

    public:
        {{appclass}}() {
            _window = _graphic->createWindow(900, 600, "{{name}}");
        }

        ~{{appclass}}() override {
            if (_window)
                _graphic->deleteWindow(_window);
        }

    protected:
        void event() override {
            if (_window->pollEvent())
                _window->eventClose();
        }

        void update() override {
            if (!_window->isOpen())
                stop();
            //TODO : fais avancer {{name}}
        }

        void display() override {
            _window->beginDraw();
            //TODO : dessine
            _window->endDraw();
        }

    private:
        /* Le vendor est lie en statique : on prend la CLASSE, pas le protocole
         * de module. acquire() et release() supposent un IModuleManager qui
         * n'existe pas ici - ne les appelle pas. */
        std::unique_ptr<IGraphic2Module> _graphic = std::make_unique<RayGraphicModule>();
        graphic::IWindow2 *_window = nullptr;
};
