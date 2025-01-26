#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <memory>

#include "osm_unpack/readers.h"
#include "visualizer/renderer.h"

struct AppState
{
    std::unique_ptr<Renderer> renderer;
    SDL_AppResult app_result = SDL_APP_CONTINUE;
};

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{

    SDL_Init(SDL_INIT_VIDEO);

    AppState *app = new AppState;
    *appstate = app;

    if (argc > 1) {
        std::unique_ptr<osm_unpack::Reader> reader_ptr = std::unique_ptr<osm_unpack::Reader>(
            new osm_unpack::Reader(static_cast<const char*>(argv[1])));
        
        app->renderer = std::unique_ptr<Renderer>(
            new Renderer(std::move(reader_ptr->ways()), std::move(reader_ptr->bounding_box())));

        return SDL_APP_CONTINUE;
    }

    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event* event)
{
    auto* app = (AppState*)appstate;
    
    if (event->type == SDL_EVENT_QUIT) {
        app->app_result = SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;  
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    auto* app = (AppState*)appstate;
    app->renderer->render();
    return app->app_result;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    auto* app = (AppState*)appstate;
    if (app) {
        delete app;
    }
}
