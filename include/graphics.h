#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include "SDL/SDL.h"
#include "map.h"
#include <string>

/*
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define SCREEN_BPP 24
#define SCREEN_VIDEOFLAGS SDL_SWSURFACE | SDL_ANYFORMAT
*/

SDL_Surface *load_optimized_pic( string file_name, SDL_Surface **screen,
                                 Uint8 ck_red, Uint8 ck_green, Uint8 ck_blue );
int prepare_graphics( SDL_Surface **screen );
int end_graphics( SDL_Surface **screen );
int paint_world( Map &world, SDL_Surface **screen);
void paint_boundary( Map &world, SDL_Surface **screen);
void paint_grid(Map &world, SDL_Surface **screen);
void prepare_to_show(SDL_Rect &target, const SDL_Rect &source);
/* prepare_to_show
  given a source rectangle in the map, source. target becomes where this
  rectangle is painted on the screen. (this might result in the target beeing
  outside the screen, but SDL solves this automagically
*/

extern SDL_Rect look;
extern SDL_Rect cliprectworld; //clip rect for painting world
extern SDL_Rect cliprectmenu;  //clip rect for painting menus
#endif //__GRAPHICS_H__
