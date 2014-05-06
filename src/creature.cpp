#include "creature.h"
#include "graphics.h"

signed long long Creature::nextid = 1;

int Creature::getid(){
  return id;
}

void Creature::think(){
  printf("Creature thinking... (not any specific creature)\n");
  return;
}

void Creature::teleported(){
  return;
}

void Creature::paint(SDL_Surface **screen){
  return;
};

void Creature::load_from_string(string loadstring){
  printf(
    "Creature %p tried to load from string and it was not overloaded: %s\n",
    this, loadstring.c_str());

}

string Creature::generate_save_string(){
  return "# This is a creatures without a overloaded generate_save_string\n";
}

void Creature::paint_lifebar(SDL_Surface **screen){
  SDL_Rect target;
  SDL_Rect bar, life;
  double percent;
  Uint32 color;

  bar.x = pos.x - 15;
  bar.y = pos.y - 20;
  bar.h = 5;
  bar.w = pos.w + 30;

  if (hp< 0)
    percent = 0;
  else
    percent = ((double)hp) / hpmax;

  life.x = bar.x + 1;
  life.y = bar.y + 1;
  life.h = bar.h - 2;
  life.w = (int)(percent * (bar.w - 2));

  prepare_to_show(target, bar);
  SDL_FillRect(*screen, &target,
               SDL_MapRGB((*screen)->format,
                          0, 0, 0));

  prepare_to_show(target, life);
  if (percent >= 1.0)
    color = SDL_MapRGB((*screen)->format, 140, 250, 140);
  else if (percent > 0.5)
    color = SDL_MapRGB((*screen)->format, 0, 250, 0);
  else if (percent < 0.25)
    color = SDL_MapRGB((*screen)->format, 230, 0, 0);
  else
    color = SDL_MapRGB((*screen)->format, 210, 210, 0);

  SDL_FillRect(*screen, &target,color);

  return;
};

Creature::Creature(){
  id = nextid;
  nextid++; /* automatically resets itself ? */
  team = 0;
  safe = 0;
  dead = 0;
  aspeed_x = 0;
  rspeed_x = 0;
  aspeed_y = 0;
  rspeed_y = 0;
  safe = 0;
  team = 1;

  hp = 1;
  hpmax = 1;
  world = NULL;

  immune_to_gravity = false;

  above = NULL;
  below = NULL;
  leftof = NULL;
  rightof = NULL;
}

Creature::~Creature(){
  return;
}

void Creature::hurt(Fxfield *fxf){
}

void Creature::move_x(int x_diff){
}

void Creature::move_y(int y_diff){
}
