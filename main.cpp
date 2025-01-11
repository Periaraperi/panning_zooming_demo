#include <cstdint>

#include <SDL2/SDL.h>
#include <stb_image.h>

using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

/*
 * A quick demo of panning and zooming in 'pure' 2d.
 * In 3d panning and zooming at a point will be done by modifying transformation matrices. Especially view matrix.
 * In the following demo things are a little bit different.
 *
 * Since this is a quick demo, I don't bother checking errors and success codes for resource creation, or using smart pointers and abstracting stuff.
 */

namespace {

struct Texture_Info {
    SDL_Texture* tex;
    i32 width {};
    i32 height {};
    i32 channel_count {};
};

[[nodiscard]]
Texture_Info load_texture(const char* path, SDL_Renderer* renderer)
{
    i32 width {};
    i32 height {};
    i32 channel_count {};

    u8* data {stbi_load(path, &width, &height, &channel_count, 0)};
    if (data == nullptr) {
        return {nullptr, {}, {}, {}};
    }

    SDL_Surface* surface {SDL_CreateRGBSurfaceWithFormat(0, width, height, 8, SDL_PIXELFORMAT_RGBA32)};
    std::copy(data, data+(width*height*channel_count), static_cast<u8*>(surface->pixels));
    SDL_Texture* tex {SDL_CreateTextureFromSurface(renderer, surface)};
    SDL_FreeSurface(surface);

    stbi_image_free(data); 
    data = nullptr;

    return {tex, width, height, channel_count};
}

// We can view this either moving all objects by world_offset or moving "camera" in -world_offset
float world_offset_x {0.0f};
float world_offset_y {0.0f};

// Since this demo is purely 2D and we are not dealing with projection or view matrices, 
// we need to manually multiply SDL_Rect destination dimension by zoom_scale to simulate zoom in or zoom out effect for sprites/textures.
float zoom_scale {2.0f};

[[nodiscard]]
float world_to_screen_x(float x) noexcept
{ return (x - world_offset_x)*zoom_scale; }

[[nodiscard]]
float world_to_screen_y(float y) noexcept
{ return (y - world_offset_y)*zoom_scale; }

[[nodiscard]]
float screen_to_world_x(float x) noexcept
{ return ((x / zoom_scale) + world_offset_x); }

[[nodiscard]]
float screen_to_world_y(float y) noexcept
{ return ((y / zoom_scale) + world_offset_y); }

}

i32 main([[maybe_unused]] i32 argc, [[maybe_unused]] char** argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    i32 screen_width  {800};
    i32 screen_height {600};

    SDL_Window* window {SDL_CreateWindow(
            "texture_sampling",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            screen_width,
            screen_height,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)};
    SDL_Renderer* renderer {SDL_CreateRenderer(window, -1, 0)};

    Texture_Info perebi {load_texture("../assets/perebi.png", renderer)};
    Texture_Info sashishi {load_texture("../assets/sashishi.png", renderer)};
    Texture_Info person {load_texture("../assets/person.png", renderer)};

    // position is in world space
    SDL_FRect sashishi_dst {screen_width*0.5f-5.0f*sashishi.width, screen_height*0.5f - 5.0f*sashishi.width, 10.0f*sashishi.width, 10.0f*sashishi.height};
    SDL_FRect perebi_dst   {0.0f,                                  0.0f,                                     50.0f*perebi.width,   25.0f*perebi.height};
    SDL_FRect person_dst   {-300.0f,                               500.0f,                                   1.0f*person.width,    1.0f*person.height};

    i32 mouse_x {};
    i32 mouse_y {};

    bool mouse_held {false};

    bool running {true};
    while (running) {
        SDL_GetMouseState(&mouse_x, &mouse_y);

        float mouse_world_x {screen_to_world_x(mouse_x)};
        float mouse_world_y {screen_to_world_y(mouse_y)};

        for (SDL_Event ev; SDL_PollEvent(&ev);) {
            if (ev.type == SDL_QUIT) {
                running = false;
                break;
            }
            if (ev.type == SDL_WINDOWEVENT) {
                if (ev.window.event == SDL_WINDOWEVENT_RESIZED) {
                    screen_width = ev.window.data1;
                    screen_height = ev.window.data2;
                }
            }

            if (ev.type == SDL_MOUSEWHEEL) {
                bool zoom {false};
                if (ev.wheel.preciseY > 0.0f) {
                    zoom_scale *= 2.0f;
                    zoom = true;
                    zoom_scale = std::min(zoom_scale, 512.0f);
                }
                if (ev.wheel.preciseY < 0.0f) {
                    zoom_scale *= 0.5f;
                    zoom = true;
                    zoom_scale = std::max(zoom_scale, 0.01f);
                }

                if (zoom) { // zoom happens at current mouse position. This means that after zoom, mouse position in world space will be scaled as well. 
                            // Which means that we need to offset world by delta of (world mouse before zoom) and (world mouse after zoom).
                    float mouse_world_after_zoom_x {screen_to_world_x(mouse_x)};
                    float mouse_world_after_zoom_y {screen_to_world_y(mouse_y)};

                    world_offset_x -= (mouse_world_after_zoom_x - mouse_world_x);
                    world_offset_y -= (mouse_world_after_zoom_y - mouse_world_y);
                }
            }

            if (ev.type == SDL_MOUSEBUTTONDOWN) {
                mouse_held = true;
            }

            if (ev.type == SDL_MOUSEBUTTONUP) {
                mouse_held = false;
            }

            if (ev.type == SDL_MOUSEMOTION && mouse_held) {
                // note: when panning take zoom_scale into consideration.
                // this basically keeps panning speed the same (relative offset). 
                // When zoomed in, offset delta is smaller, when zoomed out it is larger.
                world_offset_x -= (ev.motion.xrel / zoom_scale);
                world_offset_y -= (ev.motion.yrel / zoom_scale);
            }

        }

        SDL_SetRenderDrawColor(renderer, 128, 178, 168, 255);
        SDL_RenderClear(renderer);

        SDL_FRect perebi_d {world_to_screen_x(perebi_dst.x), world_to_screen_y(perebi_dst.y), perebi_dst.w*zoom_scale, perebi_dst.h*zoom_scale};
        SDL_RenderCopyF(renderer, perebi.tex, nullptr, &perebi_d);

        SDL_FRect sashishi_d {world_to_screen_x(sashishi_dst.x), world_to_screen_y(sashishi_dst.y), sashishi_dst.w*zoom_scale, sashishi_dst.h*zoom_scale};
        SDL_RenderCopyF(renderer, sashishi.tex, nullptr, &sashishi_d);

        SDL_FRect person_d {world_to_screen_x(person_dst.x), world_to_screen_y(person_dst.y), person_dst.w*zoom_scale, person_dst.h*zoom_scale};
        SDL_RenderCopyF(renderer, person.tex, nullptr, &person_d);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(perebi.tex);
    SDL_DestroyTexture(sashishi.tex);
    SDL_DestroyTexture(person.tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
