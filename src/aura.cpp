#include "aura.h"
#include "settings.h"
#include "graphics.h"

class Creature;

Aura::~Aura(){
  //  printf("aura dies\n");
}

void Aura::paint(SDL_Surface **screen){
  SDL_Rect target;
  if (settings.collision_mode){
    prepare_to_show(target, col_rect[0]);
    SDL_FillRect(*screen, &target,
                 SDL_MapRGB((*screen)->format,
                            70, 70, 40));
  }
}

void Aura::hit(Creature *c){
  if (not c->dead){
    if (owner->pos.x + owner->pos.w / 2 > c->pos.x + c->pos.w / 2)
      c->aspeed_x = -knock_x;
    else
      c->aspeed_x = knock_x;

    if (owner->pos.y + owner->pos.h / 2 > c->pos.y + c->pos.h / 2)
      c->aspeed_y = -knock_y;
    else
      c->aspeed_y = knock_y;

    c->safe = 5;
    c->hp = c->hp - damage;
  }
}

Aura::Aura(int x, int y, int w, int h, Creature *iowner, Map *iworld,
           int iteam, double iknock_x, double iknock_y, int idamage){
  //printf("created aura!\n");
  col_rects = 1;
  col_rect = new SDL_Rect;

  col_rect->x = x;
  col_rect->y = y;
  col_rect->w = w;
  col_rect->h = h;
  center_x = col_rect->x + col_rect->w / 2;
  center_y = col_rect->y + col_rect->h / 2;

  knock_x = iknock_x;
  knock_y = iknock_y;

  damage = idamage;

  team = iteam;
  lifetime = 1;

  owner = iowner;
  world = iworld;
}
