#ifndef __SBBFLYER_H__
#define __SBBFLYER_H__

#include "creature.h"
#include "SDL/SDL.h"
#include "aura.h"
#include <string>

enum SBBFLYER_PICS{
  SBBFLYERLEFT0 = 10,
  SBBFLYERLEFT1,
  SBBFLYERLEFT2,
  SBBFLYERRIGHT0 = 20,
  SBBFLYERRIGHT1,
  SBBFLYERRIGHT2,
  SBBFLYERHITLEFT = 30,
  SBBFLYERHITRIGHT,
};

enum SBBFLYER_STATES{
  FLYING = 1,
  HURT,
  DYING,
};

class Map;

class Sbbflyer : public Creature
{
 private:
  static int obj_count;
  static SDL_Surface *pic;
  SDL_Rect source_pos;
  SDL_Rect target_pos;
  void set_pic(int picnumber);
  bool look_right;
  Aura *aura;
  Sint16 spawn_height;
  bool goingdown;
  int flaxx_speed;
  int time;
  int curr_pic;
  int state;
  bool show_hit_pic;
  int hit_pic_timer;
  void paint_hit_pic(SDL_Surface **screen);
  int aurapower_x;
  int aurapower_y;
 public:
  void teleported();
  void set_look_right(bool ilook_right);
  bool looking_right();
  void hurt(Fxfield *fxf);
  void think();
  void paint(SDL_Surface **screen);
  void move_x(int x_diff);
  void move_y(int y_diff);
  std::string generate_save_string();
  Sbbflyer( SDL_Surface ** screen, Sint16 x, Sint16 y, bool ilook_right,
          int iteam, Map *iworld);
  ~Sbbflyer();
};

#endif //__SBBFLYER_H__
