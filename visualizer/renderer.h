#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include <SDL3/SDL.h>

class Renderer
{
    SDL_Window* window;
    SDL_Renderer* renderer;

    bool closed = false;

public:

    Renderer();
    virtual ~Renderer();
    
    void start_load_pbf(const char* file);
    void render();
    void close();
};

#endif