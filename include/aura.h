#ifndef __AURA_H__
#define __AURA_H__

#include "fxfield.h"
#include "SDL/SDL.h"
#include "creature.h"

class Map;

class Aura:public Fxfield{
 private:
  double knock_x;
  double knock_y;
  int damage;
 public:
  void paint(SDL_Surface **screen);
  void hit(Creature *c);
  Aura(int x, int y, int w, int h, Creature *iowner, Map *iworld, int iteam,
       double iknock_x, double iknock_y, int idamage);
  ~Aura();
};

#endif //__AURA_H__
