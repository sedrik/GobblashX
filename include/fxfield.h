#ifndef __FXFIELD_H__
#define __FXFIELD_H__

#include "SDL/SDL.h"
#include "creature.h"

class Map;
class Creature;

class Fxfield{
 private:
 public:
  int x1, x2, y1, y2;
  int center_x, center_y;
  int col_rects;
  int team;
  SDL_Rect *col_rect;

  Map *world;
  Creature *owner;

  virtual void think();
  virtual void hit(Creature *c);
  virtual void paint(SDL_Surface **screen);
  virtual ~Fxfield();
  Fxfield();
  int lifetime;
};

#endif // __FXFIELD_H__
