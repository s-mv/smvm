#include <SDL2/SDL.h>
#include <smvm/smvm.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

// API function declarations
void init_sdl(smvm* vm);
void close_sdl(smvm* vm);
void clear_screen(smvm* vm);
void draw_rect(smvm* vm);
void draw_line(smvm* vm);
void draw_point(smvm* vm);
void set_color(smvm* vm);
void refresh_screen(smvm* vm);
void sleep_ms(smvm* vm);
void get_key_state(smvm* vm);
void poll_events(smvm* vm);
void get_ticks(smvm* vm);
void draw_circle(smvm* vm);
char* read_file(const char* filename);

// pong
// memory layout in the VM:
// @0  = left paddle Y
// @1  = right paddle Y
// @2  = ball X
// @3  = ball Y
// @4  = ball velocity X
// @5  = ball velocity Y
// @6  = left score
// @7  = right score
// @8  = game running flag
// @9  = temp variable
// @10 = paddle speed
// @11 = ball speed
// @12 = paddle width
// @13 = paddle height
// @14 = ball size

int main() {
  smvm vm;
  smvm_init(&vm);

  char* code = read_file("examples/api/pong.asmv");
  if (!code) {
    fprintf(stderr, "Failed to read assembly file\n");
    return 1;
  }

  smvm_link_syscall(&vm, init_sdl, "init_sdl");
  smvm_link_syscall(&vm, close_sdl, "close_sdl");
  smvm_link_syscall(&vm, clear_screen, "clear_screen");
  smvm_link_syscall(&vm, draw_rect, "draw_rect");
  smvm_link_syscall(&vm, draw_line, "draw_line");
  smvm_link_syscall(&vm, draw_point, "draw_point");
  smvm_link_syscall(&vm, set_color, "set_color");
  smvm_link_syscall(&vm, refresh_screen, "refresh_screen");
  smvm_link_syscall(&vm, sleep_ms, "sleep");
  smvm_link_syscall(&vm, get_key_state, "get_key_state");
  smvm_link_syscall(&vm, poll_events, "poll_events");
  smvm_link_syscall(&vm, get_ticks, "get_ticks");
  smvm_link_syscall(&vm, draw_circle, "draw_circle");

  smvm_assemble(&vm, code);

  smvm_execute(&vm);

  free(code);
  smvm_free(&vm);

  return 0;
}

// Initialize SDL and create window/renderer
void init_sdl(smvm* vm) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n",
            SDL_GetError());
    return;
  }

  window = SDL_CreateWindow("SMVM Pong", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                            SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (window == NULL) {
    fprintf(stderr, "Window could not be created! SDL_Error: %s\n",
            SDL_GetError());
    return;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == NULL) {
    fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n",
            SDL_GetError());
    return;
  }

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  // Return success (1) in register A
  vm->registers[reg_a] = 1;
}

// Close SDL and cleanup
void close_sdl(smvm* vm) {
  if (renderer) {
    SDL_DestroyRenderer(renderer);
    renderer = NULL;
  }
  if (window) {
    SDL_DestroyWindow(window);
    window = NULL;
  }
  SDL_Quit();
}

// Clear screen with current color (default black)
void clear_screen(smvm* vm) { SDL_RenderClear(renderer); }

// Draw filled rectangle: x, y, width, height (popped from stack)
void draw_rect(smvm* vm) {
  int64_t height = *(int64_t*)smvm_pop(vm, 8);
  int64_t width = *(int64_t*)smvm_pop(vm, 8);
  int64_t y = *(int64_t*)smvm_pop(vm, 8);
  int64_t x = *(int64_t*)smvm_pop(vm, 8);

  SDL_Rect rect = {(int)x, (int)y, (int)width, (int)height};
  SDL_RenderFillRect(renderer, &rect);
}

// Draw line: x1, y1, x2, y2 (popped from stack)
void draw_line(smvm* vm) {
  int64_t y2 = *(int64_t*)smvm_pop(vm, 8);
  int64_t x2 = *(int64_t*)smvm_pop(vm, 8);
  int64_t y1 = *(int64_t*)smvm_pop(vm, 8);
  int64_t x1 = *(int64_t*)smvm_pop(vm, 8);

  SDL_RenderDrawLine(renderer, (int)x1, (int)y1, (int)x2, (int)y2);
}

// Draw single point: x, y (popped from stack)
void draw_point(smvm* vm) {
  int64_t y = *(int64_t*)smvm_pop(vm, 8);
  int64_t x = *(int64_t*)smvm_pop(vm, 8);

  SDL_RenderDrawPoint(renderer, (int)x, (int)y);
}

// Set drawing color: r, g, b, a (popped from stack)
void set_color(smvm* vm) {
  int64_t a = *(int64_t*)smvm_pop(vm, 8);
  int64_t b = *(int64_t*)smvm_pop(vm, 8);
  int64_t g = *(int64_t*)smvm_pop(vm, 8);
  int64_t r = *(int64_t*)smvm_pop(vm, 8);

  SDL_SetRenderDrawColor(renderer, (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a);
}

// Present the rendered frame
void refresh_screen(smvm* vm) { SDL_RenderPresent(renderer); }

// Sleep for milliseconds (from register A)
void sleep_ms(smvm* vm) {
  int64_t ms = vm->registers[reg_a];
  SDL_Delay((Uint32)ms);
}

// Check if key is pressed - key scancode in reg A, result in reg A
void get_key_state(smvm* vm) {
  SDL_Scancode scancode = (SDL_Scancode)vm->registers[reg_a];
  const Uint8* state = SDL_GetKeyboardState(NULL);
  vm->registers[reg_a] = state[scancode] ? 1 : 0;
}

// Poll for quit event - returns 1 if quit requested, 0 otherwise
void poll_events(smvm* vm) {
  SDL_Event event;
  int should_quit = 0;

  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      should_quit = 1;
      break;
    }
  }

  vm->registers[reg_a] = should_quit;
}

// Get current time in milliseconds
void get_ticks(smvm* vm) { vm->registers[reg_a] = SDL_GetTicks(); }

// Draw circle (outline): x, y, radius (popped from stack)
void draw_circle(smvm* vm) {
  int64_t radius = *(int64_t*)smvm_pop(vm, 8);
  int64_t cy = *(int64_t*)smvm_pop(vm, 8);
  int64_t cx = *(int64_t*)smvm_pop(vm, 8);

  // Simple circle drawing algorithm
  int r = (int)radius;
  int x = r;
  int y = 0;
  int err = 0;

  while (x >= y) {
    SDL_RenderDrawPoint(renderer, (int)cx + x, (int)cy + y);
    SDL_RenderDrawPoint(renderer, (int)cx + y, (int)cy + x);
    SDL_RenderDrawPoint(renderer, (int)cx - y, (int)cy + x);
    SDL_RenderDrawPoint(renderer, (int)cx - x, (int)cy + y);
    SDL_RenderDrawPoint(renderer, (int)cx - x, (int)cy - y);
    SDL_RenderDrawPoint(renderer, (int)cx - y, (int)cy - x);
    SDL_RenderDrawPoint(renderer, (int)cx + y, (int)cy - x);
    SDL_RenderDrawPoint(renderer, (int)cx + x, (int)cy - y);

    if (err <= 0) {
      y += 1;
      err += 2 * y + 1;
    }
    if (err > 0) {
      x -= 1;
      err -= 2 * x + 1;
    }
  }
}

// File reading utility
char* read_file(const char* filename) {
  FILE* file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "Error opening file: %s\n", filename);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* code = (char*)malloc(file_size + 1);
  if (!code) {
    fprintf(stderr, "Memory allocation failed\n");
    fclose(file);
    return NULL;
  }

  size_t read_size = fread(code, 1, file_size, file);
  code[read_size] = '\0';
  fclose(file);

  return code;
}