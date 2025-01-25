#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include <SDL3/SDL.h>

#include "../osm_unpack/wrappers.h"

class Renderer
{
    SDL_Window* window;
    SDL_Renderer* renderer;

    std::vector<osm_unpack::Way> ways_;
    osm_unpack::BoundingBox bounding_box_;

    bool closed = false;

    void render_way(const osm_unpack::Way & way, const int & screen_w_px, const int & screen_h_px);

public:

    Renderer(const std::vector<osm_unpack::Way> & ways, const osm_unpack::BoundingBox & bounding_box);
    virtual ~Renderer();
    
    void start_load_pbf(const char* file);
    void render();
    void close();

    void render_ways();
};

#endif