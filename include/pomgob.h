#ifndef __POMGOB_H__
#define __POMGOB_H__

#include "creature.h"
#include "SDL/SDL.h"
#include "aura.h"
#include <string>

#define POMGOBLEFT0 534
#define POMGOBLEFT1 522
#define POMGOBLEFT2 533
#define POMGOBRIGHT0 381
#define POMGOBRIGHT1 384
#define POMGOBRIGHT2 388

#define POMGOBHITLEFT 431
#define POMGOBHITRIGHT 433
#define POMGOBDEADLEFT 435
#define POMGOBDEADRIGHT 439

class Map;

class Pomgob : public Creature
{
 private:
  static int obj_count;
  static SDL_Surface *pic;
  SDL_Rect source_pos;
  SDL_Rect target_pos;
  void set_pic(int picnumber);
  bool standing_still;
  int runframe;
  int steps_until_picswitch;
  int legspeed;
  int jumpwait;
  int recovery;
  int leaps_default;
  int leaps_left;
  double hitpower_x;
  double hitpower_y;
  double x_acc;
  double last_aspeed_y;
  double x_ret;
  double x_airacc;
  double x_maxspeed;
  Aura *aura;
 public:
  bool looking_right();
  void set_stand_still(bool istand_still);
  bool get_stand_still();
  void set_look_right(bool ilook_right);
  void hurt(Fxfield *fxf);
  void think();
  void paint(SDL_Surface **screen);
  void move_x(int x_diff);
  void move_y(int y_diff);
  bool look_right;
  std::string generate_save_string();
  Pomgob( SDL_Surface ** screen, Sint16 x, Sint16 y, bool ilook_right,
          bool istanding_still, int iteam, Map *iworld);
  ~Pomgob();
};

#endif //__POMGOB_H__
