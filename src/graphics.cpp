#include "graphics.h"
#include "settings.h"

SDL_Rect look;
SDL_Rect cliprectworld;
SDL_Rect cliprectmenu;

void prepare_to_show(SDL_Rect & target, const SDL_Rect & source){
  target.x = source.x - look.x;
  target.y = source.y - look.y;
  target.w = source.w;
  target.h = source.h;
}

SDL_Surface *load_optimized_pic( string file_name, SDL_Surface **screen,
                                 Uint8 ck_red, Uint8 ck_green, Uint8 ck_blue )
{
  SDL_Surface *image = SDL_LoadBMP(file_name.c_str());
  SDL_Surface *optimizedImage = NULL;
  Uint32 colkey;

  if (image == NULL) {
    fprintf(stderr, "Couldn't load %s: %s\n", file_name.c_str(),
            SDL_GetError());
    exit(1);
  }

#ifdef EMSCRIPTEN
  optimizedImage = image;
#else
  optimizedImage = SDL_DisplayFormat( image );
#endif
  if ( optimizedImage == NULL ){
    fprintf(stderr, "Couldn't optimize %s: %s\n", file_name.c_str(),
            SDL_GetError());
    exit(1);
  }

  if (ck_red || ck_green || ck_blue){
    colkey = SDL_MapRGB( optimizedImage->format, ck_red, ck_green,
                           ck_blue );
    SDL_SetColorKey( optimizedImage, SDL_RLEACCEL | SDL_SRCCOLORKEY, colkey );
  }

#ifndef EMSCRIPTEN
  SDL_FreeSurface( image );
#endif

  return optimizedImage;
}

int prepare_graphics( SDL_Surface **screen )
{
  *screen = SDL_SetVideoMode( settings.screen_w, settings.screen_h,
                              settings.screen_bpp, settings.screen_flags );

  look.x = 0;
  look.y = 0;
  look.w = settings.screen_w;
  look.h = settings.screen_h;

  if ( *screen == NULL ) {
    fprintf(stderr, "Couldn't set %dx%dx%d video mode: %s\n",
            settings.screen_w, settings.screen_h, settings.screen_bpp,
            SDL_GetError());
    exit(1);
  }

#ifndef EMSCRIPTEN 
  SDL_GetClipRect(*screen, &cliprectworld);
#endif

  SDL_WM_SetCaption( "Gobblash-X", NULL );
  return 0;
}

int end_graphics( SDL_Surface **screen )
{
  if (screen != NULL)
    SDL_FreeSurface( *screen );
  return 0;
}

int paint_world( Map &world, SDL_Surface **screen )
{
#ifndef EMSCRIPTEN 
  SDL_SetClipRect(*screen, &cliprectworld);
#endif
  SDL_Rect target, source;

  if (settings.follow_hero && world.main_player != NULL){
    look.x = world.main_player->pos.x + (world.main_player->pos.w / 2) -
      (look.w / 2);
    look.y = world.main_player->pos.y + (world.main_player->pos.h / 2) -
      (look.h / 2);
  }

  if(not settings.collision_mode){
    target.x = 0;
    target.y = 0;
    target.w = settings.screen_w;
    target.h = settings.screen_h;
    source = world.pos;
    SDL_BlitSurface( world.background, &source , *screen, &target );
  }

  paint_boundary(world, screen);
  if (settings.collision_mode)
    paint_grid(world, screen);

  for(int i = 0; i <= world.get_platform_max(); i++)
      if (world.platform[i])
          world.platform[i]->paint1(screen);

  for(int i = 0; i <= world.get_platform_max(); i++)
      if (world.platform[i])
          world.platform[i]->paint2(screen);

  for(int i = 0; i <= world.get_creature_max(); i++){
    if (world.creature[i]){
      world.creature[i]->paint(screen);
    }
  }

  for(int i = 0; i <= world.get_fxfield_max(); i++)
    if (world.fxfield[i])
      world.fxfield[i]->paint(screen);

  for(int i = 0; i <= world.get_creature_max(); i++)
    if (world.creature[i] && not world.creature[i]->dead && settings.showhp)
      world.creature[i]->paint_lifebar(screen);

  if (settings.show_locations)
    for(int i = 0; i <= world.get_location_max(); i++)
      if (world.location[i])
        world.location[i]->paint(screen);

  return 0;
}

void paint_boundary(Map &world, SDL_Surface **screen)
{
  SDL_Rect target;
  if (world.boundary.x > look.x){
    target.x = 0;
    target.y = 0;
    target.w = world.boundary.x - look.x;
    target.h = look.h;
    SDL_FillRect(*screen, &target,
                 SDL_MapRGB((*screen)->format, 40, 0, 0));

  }
  if (world.boundary.y > look.y){
    target.x = 0;
    target.y = 0;
    target.w = look.w;
    target.h = world.boundary.y - look.y;
    SDL_FillRect(*screen, &target,
                 SDL_MapRGB((*screen)->format, 40, 0, 0));

  }
  if (world.boundary.y + world.boundary.h < look.y + look.h ){
    target.x = 0;
    target.h = (look.y + look.h) - (world.boundary.y + world.boundary.h);
    target.y = look.h - target.h;
    target.w = look.w;
    SDL_FillRect(*screen, &target,
                 SDL_MapRGB((*screen)->format, 40, 0, 0));

  }
  if (world.boundary.x + world.boundary.w < look.x + look.w ){
    target.y = 0;
    target.w = (look.x + look.w) - (world.boundary.x + world.boundary.w);
    target.x = look.w - target.w;
    target.h = look.h;
    SDL_FillRect(*screen, &target,
                 SDL_MapRGB((*screen)->format, 40, 0, 0));
  }
}

void paint_grid(Map &world, SDL_Surface **screen){

  SDL_Rect target;

  for(int x = 0; x < world.grid->get_cols(); x++)
    for(int y = 0; y < world.grid->get_rows(); y++){
      target = world.grid->coord_to_rect(x,y);
      target.x = target.x - look.x;
      target.y = target.y - look.y;
      if (world.grid->box[x][y].fxfields != NULL ){
        SDL_FillRect(*screen, &target,
                     SDL_MapRGB((*screen)->format, 50, 50, 150));
      }
      else if (world.grid->box[x][y].creatures != NULL ){
        if (world.grid->box[x][y].platforms != NULL ){
          SDL_FillRect(*screen, &target,
                       SDL_MapRGB((*screen)->format, 2*100, 2*100, 0));
        }
        else{
          SDL_FillRect(*screen, &target,
                       SDL_MapRGB((*screen)->format, 2*100, 0, 0));
        }
      }
      else if (world.grid->box[x][y].platforms != NULL ){
        SDL_FillRect(*screen, &target,
                     SDL_MapRGB((*screen)->format, 0, 2*100, 0));
      }
      else if (world.grid->box[x][y].locations != NULL ){
        SDL_FillRect(*screen, &target,
                     SDL_MapRGB((*screen)->format, 30, 30, 50));
      }
      else if ((x+y) % 2 == 0)
        SDL_FillRect(*screen, &target,
                     SDL_MapRGB((*screen)->format, 0, 100, 100));
      else
        SDL_FillRect(*screen, &target,
                     SDL_MapRGB((*screen)->format, 0, 30+100, 30+100));
    }
}
