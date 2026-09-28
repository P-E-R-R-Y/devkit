/**
 * @file {{appclass}}.hpp
 * @brief The {{example}} sandbox: enough to watch {{name}} run.
 *
 * Written once by devkit, then it is yours. Carve into it.
 */

#pragma once

#include "ICore.hpp"
#include "RayGraphicModule.hpp"

#include <memory>

class {{appclass}} : public IApp {

    public:
        {{appclass}}() {
            _window = _graphic->createWindow(900, 600, "{{name}} - {{example}}");
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
            //TODO: step {{name}} forward
        }

        void display() override {
            _window->beginDraw();
            //TODO: draw
            _window->endDraw();
        }

    private:
        /* The vendor is linked statically: we take the CLASS, not the module
         * protocol. acquire() and release() assume an IModuleManager that does
         * not exist here - do not call them. */
        std::unique_ptr<IGraphic2Module> _graphic = std::make_unique<RayGraphicModule>();
        graphic::IWindow2 *_window = nullptr;
};
