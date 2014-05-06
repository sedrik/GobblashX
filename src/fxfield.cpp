#include "fxfield.h"

Fxfield::Fxfield(){
  col_rect = NULL;
  team = 0;
  return;
}

Fxfield::~Fxfield(){
  if (col_rect != NULL)
    delete col_rect;
  return;
}

void Fxfield::hit(Creature *c){
  printf("a nonspecific fxfield hit something\n");
}

void Fxfield::think(){
  return;
}

void Fxfield::paint(SDL_Surface **screen){
  printf("fxfield painted (not any specific)\n");
  return;
}
