#ifndef __BLUBBER_H__
#define __BLUBBER_H__

#include "creature.h"
#include "SDL/SDL.h"
#include "aitools.h"
#include "aura.h"
#include "string.h"

enum blubberstate {
  SET_TO_ROOF, //just an initial state, sets creature to roof
  WAITING,
  FALLING,
  LANDING,
  STANDING,
  HURTING,
  MOVE_L,
  MOVE_LU,
  MOVEU_R,
  MOVE_R,
  MOVE_RU,
  MOVEU_L,
};

class Blubber : public Creature
{
 private:
  static int obj_count;
  static SDL_Surface *pic;
  SDL_Rect source_pos;
  SDL_Rect target_pos;
  bool look_right;
  blubberstate state;
  int anim; //animation nr
  int time; //frame clock
  Aitools *aitools;
  Aura *aura;
  int aurapower_x;
  int aurapower_y;
  void cause_damage(void);

 public:
  Blubber(SDL_Surface **screen, Aitools *iaitools, Map *iworld,
          std::string loadstring);
  Blubber(int posx, int posy, int iteam, bool blook_right,
          bool start_on_roof, SDL_Surface **screen,
          Aitools *iaitools, Map *iworld);
  ~Blubber(void);
  void init_blubber(bool start_on_roof,SDL_Surface **screen, Aitools *iaitools,
                    Map *iworld);
  void set_state(blubberstate nstate);
  void set_falling(bool fall_down);
  bool get_falling(void);
  void set_look_right(bool slook_right);
  void toggle_look_right(void);
  bool looking_right(void);
  void hurt(Fxfield *fxf);
  void think(void);
  void paint(SDL_Surface **screen);
  void move_x(int x_diff);
  void move_y(int y_diff);
  std::string generate_save_string();
};
#endif //__BLUBBER_H__
