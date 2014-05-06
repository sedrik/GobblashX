#ifndef __ZOMBO_H__
#define __ZOMBO_H__

#include "creature.h"
#include "aitools.h"
#include "SDL/SDL.h"
#include "aura.h"
#include <string>
#include "invizdmgfield.h"

enum Zombostate {
  ZOMBO_STATE_PATROLLING,
  ZOMBO_STATE_RECOVERING,
  ZOMBO_STATE_PREPARING_ATTACK,
  ZOMBO_STATE_ATTACKING,
  ZOMBO_STATE_EXHAUSTED,
  ZOMBO_STATE_DYING,
  ZOMBO_STATE_DEAD
};

enum Zombopic {
  ZOMBO_R_WALK1,
  ZOMBO_R_WALK2,
  ZOMBO_R_WALK3,
  ZOMBO_R_ATK1,
  ZOMBO_R_ATK2,
  ZOMBO_R_ATK3,
  ZOMBO_R_HURT1,
  ZOMBO_R_HURT2,
  ZOMBO_R_HURT3,
  ZOMBO_R_HURT4,
  ZOMBO_R_HURT5,
  ZOMBO_R_DYING1,
  ZOMBO_R_DYING2,
  ZOMBO_R_DYING3,
  ZOMBO_R_DYING4,
  ZOMBO_R_DEAD,

  ZOMBO_L_WALK1,
  ZOMBO_L_WALK2,
  ZOMBO_L_WALK3,
  ZOMBO_L_ATK1,
  ZOMBO_L_ATK2,
  ZOMBO_L_ATK3,
  ZOMBO_L_HURT1,
  ZOMBO_L_HURT2,
  ZOMBO_L_HURT3,
  ZOMBO_L_HURT4,
  ZOMBO_L_HURT5,
  ZOMBO_L_DYING1,
  ZOMBO_L_DYING2,
  ZOMBO_L_DYING3,
  ZOMBO_L_DYING4,
  ZOMBO_L_DEAD
};

class Map;

class Zombo : public Creature
{
 private:
  static int obj_count;
  static SDL_Surface *pic;
  SDL_Rect source_pos;
  SDL_Rect target_pos;
  void set_pic(Zombopic zpic);
  int animframe;
  int frames_until_picswitch;
  int attackdamage;
  double aurapower_x;
  double aurapower_y;
  int auradamage;
  double patrolspeed;
  Aura *aura;
  Invizdmgfield *dmgfield;
  bool look_right;
  Zombostate state;
  void set_state(Zombostate newstate);
  void use_aura();
  Aitools *aitools;
 public:
  bool looking_right();
  void set_look_right(bool ilook_right);
  void hurt(Fxfield *fxf);
  void think();
  void paint(SDL_Surface **screen);
  void move_x(int x_diff);
  void move_y(int y_diff);
  std::string generate_save_string();
  Zombo( SDL_Surface ** screen, Sint16 x, Sint16 y, bool ilook_right,
         int iteam, Map *iworld, Aitools *aitools);
  ~Zombo();
};

#endif //__ZOMBO_H__
