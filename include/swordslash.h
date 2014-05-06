#ifndef __SWORDSLASH_H__
#define __SWORDSLASH_H__

#include "fxfield.h"
#include "SDL/SDL.h"
#include "creature.h"
#include "player.h"

class Map;

class Swordslash: public Fxfield{
 private:
  static SDL_Surface *pic;
  SDL_Rect source_pos;
  SDL_Rect target_pos;
  static int spawners;  // keep track of how many objects that can use
                             // swordslash
  double knock_x;
  double knock_y;
  int damage;
  int xrel, yrel; /* theese are coords which describes how long away from the
                     given x and y the col_rect acctually are */
  int xrelt, yrelt; /* the same as the above, but for the paint
                                 target rectangle */

 public:
  static void add_spawner(SDL_Surface **screen);
  static void remove_spawner();
  void paint(SDL_Surface **screen);
  void hit(Creature *c);
  void update_pos(int x, int y);
  Swordslash(int x, int y, Playerswordpic attack, int lifetime,
             Map *world, int iteam, double iknock_x, double iknock_y,
             int idamage, Creature *iowner);
  ~Swordslash();
};

#endif //__SWORDSLASH_H__
