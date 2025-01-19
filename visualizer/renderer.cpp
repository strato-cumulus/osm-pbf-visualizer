#include "renderer.h"

#include <thread>

#include "../osm_unpack/readers.h"

Renderer::Renderer()
{
    this->window = SDL_CreateWindow(
        "Window", 1024, 768, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    this->renderer = SDL_CreateRenderer(this->window, NULL);
}

Renderer::~Renderer()
{
    if ( ! this->closed ) {
        close();
    }
}

void Renderer::start_load_pbf(const char *file_name)
{
    std::thread load_thread([&]() {
        new osm_unpack::Reader(file_name);
    });
}

void Renderer::render()
{
    SDL_SetRenderDrawColor(this->renderer, 255., 255., 255., SDL_ALPHA_OPAQUE);
    SDL_RenderClear(this->renderer);
    SDL_SetRenderDrawColor(this->renderer, 0., 0., 0., SDL_ALPHA_OPAQUE);
    SDL_RenderPresent(this->renderer);
}

void Renderer::close()
{
    SDL_DestroyRenderer(this->renderer);
    this->closed = true;
}
