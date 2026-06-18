#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

void cleanup(SDL_Window* window) {
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    // Init SDL
    if(!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        // SDL_Quit() is safe even after init errors, but it's not necessary here.
        return 1;
    }

    // Create SDL window
    SDL_Window* window = SDL_CreateWindow("CHIP-8 Emulator", 800, 600, SDL_WINDOW_RESIZABLE);

    // Handle window creation failure
    if(window == nullptr) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Loop to handle incoming events from SDL
    bool running = true;

    while(running) {
        SDL_Event event;

        while(SDL_PollEvent(&event)) {
            // Closing the window exits the application.
            if(event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
    }

    // Cleanup
    cleanup(window);
    
    return 0;
}