#ifndef __INVIZDMGFIELD_H__
#define __INVIZDMGFIELD_H__

#include "fxfield.h"
#include "SDL/SDL.h"
#include "creature.h"

class Map;

class Invizdmgfield : public Fxfield{
 private:
  double knock_x;
  double knock_y;
  int damage;
  int xrel, yrel; /* theese are coords which describes how long away from the
                     given x and y the col_rect acctually are */
 public:
  void paint(SDL_Surface **screen);
  void hit(Creature *c);
  void update_pos(int x, int y);
  Invizdmgfield(int x, int y, int w, int h, int y_rel, int lifetime,
             Map *world, int iteam, double iknock_x, double iknock_y,
             int idamage, Creature *iowner);
  ~Invizdmgfield();
};

#endif //__INVIZDMGFIELD_H__
