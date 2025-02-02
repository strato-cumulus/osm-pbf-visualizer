#include "renderer.h"

#include <thread>

#include "../osm_unpack/readers.h"

void Renderer::render_way(const osm_unpack::Way &way, const int &screen_w_px, const int &screen_h_px)
{
    auto nodes = way.nodes();
    if ( nodes.size() < 2 ) {
        return;
    }

    int64_t bb_heigth_span = bounding_box_.bottom - bounding_box_.top;
    int64_t bb_width_span = bounding_box_.right - bounding_box_.left;

    auto node_it = nodes.begin();

    SDL_FPoint line_points[nodes.size()];

    int counter = 0;
    while ( node_it != nodes.end() ) {
        auto node = *node_it++;

        line_points[counter].x = ( node.lon() - bounding_box_.top ) * screen_h_px / bb_heigth_span;
        // the 3/4 is a magic constant for now until I figure out how to center the view properly
        line_points[counter].y = ( screen_w_px * 3/4 ) - ( node.lat() - bounding_box_.left ) * screen_w_px / bb_width_span;

        counter += 1;
    }

    SDL_RenderLines(this->renderer, line_points, nodes.size());
}

Renderer::Renderer(const std::vector<osm_unpack::Way> & ways, const osm_unpack::BoundingBox & bounding_box):
    ways_(ways), bounding_box_(bounding_box)
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

void Renderer::render()
{
    SDL_SetRenderDrawColor(this->renderer, 255., 255., 255., SDL_ALPHA_OPAQUE);
    SDL_RenderClear(this->renderer);
    SDL_SetRenderDrawColor(this->renderer, 0., 0., 0., SDL_ALPHA_OPAQUE);
    this->render_ways();
    SDL_RenderPresent(this->renderer);
}

void Renderer::close()
{
    SDL_DestroyRenderer(this->renderer);
    this->closed = true;
}

void Renderer::render_ways()
{
    int w, h;
    SDL_GetWindowSizeInPixels(this->window, &w, &h);

    for ( auto const& way : ways_ ) {
        render_way(way, w, h);
    }
}
