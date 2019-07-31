#include <Arduino.h>
#ifndef SDL_H
#define SDL_H
int sdl_update(void);
void sdl_init(void);
void sdl_frame(void);
void sdl_quit(void);
byte* sdl_get_framebuffer(void);
void sdl_clear_framebuffer(byte col);
void sdl_clear_screen(byte col);
unsigned int sdl_get_buttons(void);
unsigned int sdl_get_directions(void);
uint16_t* sdl_get_palette(void);
void sdl_set_palette(const unsigned int* col);
#endif
