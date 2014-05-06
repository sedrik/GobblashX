#include "swordslash.h"
#include "SDL/SDL.h"
#include "graphics.h"
#include "fxfield.h"
#include "settings.h"
#include "map.h"
#include "game.h"

int Swordslash::spawners = 0;
SDL_Surface *Swordslash::pic = NULL;

void Swordslash::update_pos(int x, int y){
  col_rect[0].x = x + xrel;
  col_rect[0].y = y + yrel;
  target_pos.x = x + xrelt;
  target_pos.y = y + yrelt;
}

Swordslash::Swordslash(int x, int y, Playerswordpic attack,
                       int ilifetime, Map *iworld, int iteam, double iknock_x,
                       double iknock_y, int idamage, Creature *iowner){
  world = iworld;
  owner = iowner;
  team = iteam;
  knock_x = iknock_x;
  knock_y = iknock_y;
  damage = idamage;
  col_rects = 1;
  col_rect = new SDL_Rect;
  switch (attack){
  case PSW_RIGHT:
    source_pos.x = 19;
    source_pos.y = 10;
    source_pos.w = 60;
    source_pos.h = 22;
    col_rect[0].w = source_pos.w;
    col_rect[0].h = source_pos.h;
    xrel = iowner->pos.w;
    yrel = 7;
    xrelt = iowner->pos.w - 14;
    yrelt = 7;
    update_pos(x, y);
    break;

  case PSW_LEFT:
    source_pos.x = 15;
    source_pos.y = 43;
    source_pos.w = 60;
    source_pos.h = 22;
    col_rect[0].w = source_pos.w;
    col_rect[0].h = source_pos.h;
    xrel = -source_pos.w;
    yrel = 7;
    xrelt = -source_pos.w + 14;
    yrelt = 7;
    update_pos(x, y);
    break;

  case PSW_UPPERCUTRIGHT:
    source_pos.x = 96;
    source_pos.y = 2;
    source_pos.w = 26;
    source_pos.h = 46;
    col_rect[0].w = 47;
    col_rect[0].h = 60;
    xrel = 30;
    yrel = 0;
    xrelt = 51;
    yrelt = 9;
    update_pos(x, y);
    break;

  case PSW_UPPERCUTLEFT:
    source_pos.x = 96;
    source_pos.y = 49;
    source_pos.w = 26;
    source_pos.h = 46;
    col_rect[0].w = 47;
    col_rect[0].h = 60;

    xrel = -47;
    yrel = 0;
    xrelt = -47;
    yrelt = 9;
    update_pos(x, y);
    break;
  case PSW_UPSWINGRIGHT:
    source_pos.x = 12;
    source_pos.y = 78;
    source_pos.w = 80;
    source_pos.h = 54;
    col_rect[0].w = 79;
    col_rect[0].h = 45;

    xrel = -6;
    yrel = -40;
    xrelt = -3;
    yrelt = -38;
    update_pos(x, y);
    break;
  case PSW_UPSWINGLEFT:
    source_pos.x = 12;
    source_pos.y = 133;
    source_pos.w = 80;
    source_pos.h = 54;
    col_rect[0].w = 79;
    col_rect[0].h = 45;

    xrel = 30 - 1 - col_rect[0].w - 1 + 6;
    yrel = -40;
    xrelt = 30 - 1 - col_rect[0].w - 1 + 3;
    yrelt = -38;
    update_pos(x, y);
    break;
  case PSW_DOWNSLASHRIGHT:
    source_pos.x = 132;
    source_pos.y = 105;
    source_pos.w = 31;
    source_pos.h = 46;
    col_rect[0].w = 45;
    col_rect[0].h = 38;

    xrel = 30;
    yrel = + 10;
    xrelt = 30 - 1 + 15;
    yrelt = + 2;
    update_pos(x, y);
    break;
  case PSW_DOWNSLASHLEFT:
    source_pos.x = 133;
    source_pos.y = 153;
    source_pos.w = 31;
    source_pos.h = 46;
    col_rect[0].w = 45;
    col_rect[0].h = 38;

    xrel = + 1 - source_pos.w - 15;
    yrel = + 10;
    xrelt = + 1 - source_pos.w + 1 - 15;
    yrelt = + 2;
    update_pos(x, y);
    break;
  default:
    printf("Swordslash::Swordslash() Errorous attacktype %d\n",
           attack);
    exit(1);
  }
  target_pos.w = source_pos.w;
  target_pos.h = source_pos.h;
  center_x = target_pos.x + (target_pos.w / 2);
  center_y = target_pos.y + (target_pos.h / 2);
  lifetime = ilifetime;
}

Swordslash::~Swordslash(){
  Player *p;
  p = dynamic_cast<Player*>(owner);
  if (p){
    p->lost_slash(this);
  }
}

void Swordslash::paint(SDL_Surface **screen){
  SDL_Rect source, target;
  source = source_pos;
  if (settings.collision_mode){
    prepare_to_show(target, col_rect[0]);
    SDL_FillRect(*screen, &target,
                 SDL_MapRGB((*screen)->format,
                            120, 0, 120));
  }
  else{
    prepare_to_show(target, target_pos);
    if(SDL_BlitSurface(pic, &source, *screen,
                       &target)){
      printf("Player:paint() Error while blitting player:%s",SDL_GetError());
      exit(1);
    }
  }
}

void Swordslash::hit(Creature *c){
  if (not c->dead){
    c->aspeed_x = knock_x;
    c->aspeed_y = knock_y;
    c->safe = 5;
    c->hp = c->hp - damage;
  }
}

void Swordslash::add_spawner(SDL_Surface **screen){
  if (spawners == 0 && pic == NULL){
      pic = load_optimized_pic( "graphics/swordslash.bmp", screen,
                                       44, 255, 153 );
  }
  spawners ++;
}

void Swordslash::remove_spawner(){
  spawners --;
  if (spawners == 0 && pic != NULL){
      SDL_FreeSurface(pic);
      pic = NULL;
  }
}
