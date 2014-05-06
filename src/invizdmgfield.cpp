#include "invizdmgfield.h"
#include "SDL/SDL.h"
#include "graphics.h"
#include "fxfield.h"
#include "settings.h"
#include "map.h"
#include "game.h"

void Invizdmgfield::update_pos(int x, int y){
  col_rect[0].x = x + xrel;
  col_rect[0].y = y + yrel;
  /* IN SWORDSLASH, ALSO UPDATE CENTER_X and CENTER_Y */
}

Invizdmgfield::Invizdmgfield(int x, int y, int w, int h, int iyrel,
                             int ilifetime, Map *iworld, int iteam,
                             double iknock_x, double iknock_y,
                             int idamage, Creature *iowner){
  world = iworld;
  owner = iowner;
  team = iteam;
  knock_x = iknock_x;
  knock_y = iknock_y;
  damage = idamage;
  col_rects = 1;
  col_rect = new SDL_Rect;
  col_rect[0].w = w;
  col_rect[0].h = h;

  yrel = iyrel;
  if (knock_x > 0){
    xrel = iowner->pos.w;
  }
  else if (knock_x < 0){
    xrel = - col_rect[0].w;
  }
  else{
    printf("Invizdmgfield::Invizdmgfield iknock_x must be nonzero!\n");
    exit(1);
  }
  update_pos(x, y);
  lifetime = ilifetime;
}

Invizdmgfield::~Invizdmgfield(){
}

void Invizdmgfield::paint(SDL_Surface **screen){
  SDL_Rect target;
  if (settings.collision_mode){
    prepare_to_show(target, col_rect[0]);
    SDL_FillRect(*screen, &target,
                 SDL_MapRGB((*screen)->format,
                            120, 0, 120));
  }
}

void Invizdmgfield::hit(Creature *c){
  if (not c->dead){
    c->aspeed_x = knock_x;
    c->aspeed_y = knock_y;
    c->safe = 5;
    c->hp = c->hp - damage;
  }
}
