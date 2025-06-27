#include <SDL2/SDL.h>
#include <smvm/smvm.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int init_sdl() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n",
            SDL_GetError());
    return 0;
  }

  window = SDL_CreateWindow("SMVM Pong", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                            SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (window == NULL) {
    fprintf(stderr, "Window could not be created! SDL_Error: %s\n",
            SDL_GetError());
    return 0;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == NULL) {
    fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n",
            SDL_GetError());
    return 0;
  }

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);

  return 1;
}

// Clean up SDL resources
void close_sdl() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

// SDL implementation for clear_screen
void clear_screen_fn(smvm* vm) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
}

// SDL implementation for draw_rect
void draw_rect_fn(smvm* vm) {
  // Get parameters from VM registers
  int x = vm->registers[reg_a];
  int y = vm->registers[reg_b];
  int width = vm->registers[reg_c];
  int height = vm->registers[reg_d];

  SDL_Rect rect = {x, y, width, height};
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderFillRect(renderer, &rect);
  SDL_RenderPresent(renderer);
}

// Implementation for sleep function
void sleep_fn(smvm* vm) {
  int ms = vm->registers[reg_a];
  SDL_Delay(ms);

  // Process any pending SDL events to keep the window responsive
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      // Set a trap flag to exit the VM
      smvm_set_flag(vm, flag_t);
    }
  }
}

int main() {
  // Initialize SDL
  if (!init_sdl()) { return 1; }

  // Initialize VM
  smvm vm;
  smvm_init(&vm);

  // Link external functions
  smvm_link_call(&vm, clear_screen_fn, "clear_screen");
  smvm_link_call(&vm, draw_rect_fn, "draw_rect");
  smvm_link_call(&vm, sleep_fn, "sleep");

  // Load the assembly code
  FILE* file = fopen("examples/api/pong.asmv", "r");
  if (!file) {
    fprintf(stderr, "Error opening file\n");
    close_sdl();
    return 1;
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* code = (char*)malloc(file_size + 1);
  if (!code) {
    fprintf(stderr, "Memory allocation failed\n");
    fclose(file);
    close_sdl();
    return 1;
  }

  size_t read_size = fread(code, 1, file_size, file);
  code[read_size] = '\0';
  fclose(file);

  smvm_assemble(&vm, code);
  smvm_execute(&vm);

  free(code);
  smvm_free(&vm);
  close_sdl();

  return 0;
}
