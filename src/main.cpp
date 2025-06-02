#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include <iostream>

#include "grid_map.hpp"
#include "grid_tree.hpp"
#include "utils.hpp"


SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
double lastFrame = 0.0;

GridMap map;
GridTree grid;
struct {
    SDL_FPoint pos = {0.0, 0.0};
    float angle = 0.0; // In radians

    float wall;
    float fov = M_PI_2; // 90 degrees

    float speed;
    float rot_speed = 2.0 * M_PI_2;
} camera;

SDL_AppResult SDL_AppInit(void** app_state, int argc, char** argv) {
    // Initializes SDL along with a window
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Unable to initialize SDL: " << SDL_GetError() << '\n';
        return SDL_APP_FAILURE;
    }
    if (!SDL_CreateWindowAndRenderer("Quadcaster", 800, 600, SDL_WINDOW_RESIZABLE, &window,
                                     &renderer)) {
        std::cerr << "Unable to create window/renderer: " << SDL_GetError() << '\n';
        return SDL_APP_FAILURE;
    }

    // Enables v-sync
    SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE);

    // Loads map
    map = GridMap(argc >= 2 ? argv[1] : "maps/a.txt");
    grid = map.treeify();
    std::cout << grid.graphviz();

    // Adjusts wall height and player speed according to the map size
    camera.wall = map.width > map.height ? 2.0 / map.width : 2.0 / map.height;
    camera.speed = camera.wall * 2.0;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* app_state) {
    /****★**********/
    /* Diagnostics */
    /****★**********/

    double deltaTime = (double)SDL_GetPerformanceCounter() / SDL_GetPerformanceFrequency() - lastFrame;
    lastFrame += deltaTime;

    std::string title = "Quadcaster (x: " + std::to_string(camera.pos.x) +
                        ", y: " + std::to_string(camera.pos.y) +
                        ", angle: " + std::to_string((int)(camera.angle / M_PI * 180.0)) +
                        ", fov: " + std::to_string((int)(camera.fov / M_PI * 180.0)) + ") at " +
                        std::to_string((int)(1.0 / deltaTime)) + " FPS";
    SDL_SetWindowTitle(window, title.c_str());

    /****★*************/
    /* Input handling */
    /****★*************/

    const bool* keyboard = SDL_GetKeyboardState(nullptr);

    // Boosted speed factor upon sprinting
    float sprint = keyboard[SDL_SCANCODE_LSHIFT] ? 2.0 : 1.0;

    // Camera movement
    if (keyboard[SDL_SCANCODE_W]) {
        camera.pos.x += std::sin(camera.angle) * camera.speed * sprint * deltaTime;
        camera.pos.y += std::cos(camera.angle) * camera.speed * sprint * deltaTime;
    }
    if (keyboard[SDL_SCANCODE_A]) {
        camera.pos.x += std::sin(camera.angle - M_PI_2) * camera.speed * sprint * deltaTime;
        camera.pos.y += std::cos(camera.angle - M_PI_2) * camera.speed * sprint * deltaTime;
    }
    if (keyboard[SDL_SCANCODE_S]) {
        camera.pos.x += std::sin(camera.angle + M_PI) * camera.speed * sprint * deltaTime;
        camera.pos.y += std::cos(camera.angle + M_PI) * camera.speed * sprint * deltaTime;
    }
    if (keyboard[SDL_SCANCODE_D]) {
        camera.pos.x += std::sin(camera.angle + M_PI_2) * camera.speed * sprint * deltaTime;
        camera.pos.y += std::cos(camera.angle + M_PI_2) * camera.speed * sprint * deltaTime;
    }

    // Camera rotation
    if (keyboard[SDL_SCANCODE_LEFT]) {
        camera.angle -= camera.rot_speed * deltaTime;
    }
    if (keyboard[SDL_SCANCODE_RIGHT]) {
        camera.angle += camera.rot_speed * deltaTime;
    }

    /****★********/
    /* Rendering */
    /****★********/

    int width, height;
    SDL_GetCurrentRenderOutputSize(renderer, &width, &height);
    float midpoint = height / 2.0;

    // Sky color
    SDL_SetRenderDrawColor(renderer, 128, 224, 255, 255);
    SDL_FRect skyRect = {.x = 0, .y = 0, .w = (float)width, .h = midpoint};
    SDL_RenderFillRect(renderer, &skyRect);

    // Ground color
    SDL_SetRenderDrawColor(renderer, 64, 128, 64, 255);
    SDL_FRect groundRect = {.x = 0, .y = midpoint, .w = (float)width, .h = midpoint};
    SDL_RenderFillRect(renderer, &groundRect);

    float cameraField = std::tan(camera.fov / 2.0);
    camera.angle = std::fmod(camera.angle, 2.0 * M_PI) + (camera.angle < 0.0 ? 2.0 * M_PI : 0.0);

    // Raycasts for each pixel column in the screen
    for (int x = 0; x < width; x++) {
        // Calculates the appropriate ray angle for each pixel column, taking account perspective
        float cameraX = remap(x, 0.0, width, -1.0, 1.0);
        float rayAngle = camera.angle + std::atan(cameraX * cameraField);

        // Casts the ray and checks for its success
        RayHit ray = grid.cast(camera.pos, rayAngle);
        if (ray.hit) {
            float distance = std::sqrt(std::pow(ray.locus.x - camera.pos.x, 2.0) +
                                       std::pow(ray.locus.y - camera.pos.y, 2.0)) *
                             std::cos(rayAngle - camera.angle); // Undoes the fish-eye effect
            float length = camera.wall / distance * (width / 2.0) / cameraField;

            // Draws the pixel column whose length is determined on the distance inverse
            SDL_SetRenderDrawColor(renderer, ray.color.r, ray.color.g, ray.color.b, ray.color.a);
            SDL_RenderLine(renderer, x, midpoint - length / 2.0, x, midpoint + length / 2.0);
        }
    }

    SDL_RenderPresent(renderer);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* app_state, SDL_Event* event) {
    switch (event->type) {
        // Concludes the program on quitting
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;

        case SDL_EVENT_KEY_DOWN:
            // Handles FOV changes
            switch (event->key.scancode) {
                case SDL_SCANCODE_UP:
                    camera.fov -= M_PI / 180.0 * 5.0;
                    break;

                case SDL_SCANCODE_DOWN:
                    camera.fov += M_PI / 180.0 * 5.0;
                    break;

                default:
                    // Appeases compiler warnings
                    break;
            }
            break;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* app_state, SDL_AppResult result) {}
