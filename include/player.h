#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "controller.h"
#include "SDL/SDL.h"
#include "map.h"
#include "creature.h"
#include <string>

class Swordslash;

enum Playerpic {
  PP_RIGHT1, // run right - legs together
  PP_RIGHT0, // stand - look right
  PP_RIGHT2, // run right - step 1
  PP_RIGHT3, // run right - step 2
  PP_LEFT3, // run left - step 2
  PP_LEFT2, // run left step 1
  PP_LEFT0, // stand - look left
  PP_LEFT1, // run left - legs together

  PP_FLYRIGHT, // flying right
  PP_FLYUPRIGHT, // flying up-right
  PP_FLYDOWNRIGHT, // flying down-right
  PP_FLYLEFT, // flying right
  PP_FLYUPLEFT, // flying up-left
  PP_FLYDOWNLEFT, // flying down-left

  PP_BREAKRIGHT, // breaking in right
  PP_BREAKLEFT, // breaking in left
  PP_ATTACKRIGHT, // attacking to the right
  PP_ATTACKLEFT, // attacking to the right
  PP_UPSWINGRIGHT, //airbourne uppercut
  PP_UPSWINGLEFT, //airbourne uppercut
  PP_UPPERCUTRIGHT, //airbourne uppercut
  PP_UPPERCUTLEFT, //airbourne uppercut
  PP_DOWNSLASHRIGHT, //airbourne downslash
  PP_DOWNSLASHLEFT, //airbourne downslash
  PP_DOWNSLASHRECOVERYLEFT, //recovery from downslash
  PP_DOWNSLASHRECOVERYRIGHT, //recovery from downslash

  PP_HURTUPLEFT,
  PP_HURTLEFT,
  PP_HURTDOWNLEFT,
  PP_HURTSLIDELEFT,
  PP_SITLEFT,
  PP_DEADLEFT,

  PP_HURTUPRIGHT,
  PP_HURTRIGHT,
  PP_HURTDOWNRIGHT,
  PP_HURTSLIDERIGHT,
  PP_SITRIGHT,
  PP_DEADRIGHT,

  PP_LWALL, //sliding on left wall
  PP_LWALLATK, //attack while sliding on wall
  PP_LWALLATK2, //attack, followup.

  PP_RWALL, //sliding on right wall
  PP_RWALLATK, //attack while sliding on wall
  PP_RWALLATK2, //attack, followup.

};

enum Playerswordpic{
  PSW_NONE, //Sword is invisible
  PSW_LEFT, //normal standing slash to the left
  PSW_RIGHT, //same but right
  PSW_UPPERCUTLEFT,
  PSW_UPPERCUTRIGHT,
  PSW_UPSWINGRIGHT,
  PSW_UPSWINGLEFT,
  PSW_DOWNSLASHRIGHT,
  PSW_DOWNSLASHLEFT,
  PSW_LANDRECOVERYRIGHT,
  PSW_LANDRECOVERYLEFT,
  PSW_LWALL,
  PSW_RWALL
};

//playerstates (see player_moves diagram)
enum Playerstate{
  PS_STANDING,
  PS_CONTROLLED_RUNNING,
  PS_CONTROLLED_AIRBOURNE,
  PS_RECOVERING_FROM_SLASH,
  PS_UNCONTROLLED_AIRBOURNE,
  PS_UNCONTROLLED_SLIDING,
  PS_SITTING,
  PS_DEAD,
  PS_AIRBOURNE_UPPERCUT,
  PS_AIRBOURNE_UPSWING,
  PS_AIRBOURNE_DOWNSLASH,
  PS_LANDED_RECOVERY,
  PS_WALLSLIDING,
  PS_WALLSLIDINGATTREC //recovering from wallslide attack
};

class Aitools;

class Player:public Creature{
 private:
  static int obj_count;
  static SDL_Surface *pic;
  int skinnum; /* 0 or 1 depending on what skin is beeing used */
  int skinoffset; /* Offset value Y for player skin */
  void prepare_state(Playerstate newstate); //prepares a state
  void set_pic(enum Playerpic picnumber);
  SDL_Rect swordpic_source;
  SDL_Rect swordpic_target;
  SDL_Rect source_pos;
  SDL_Rect target_pos;
  void set_swordpic(enum Playerswordpic swordnumber);
  bool show_sword;
  Swordslash *currslash; //the current slash, or uppercut or even downslash
  Swordslash *upswing; //the current upswing
  double downslashpower_x; /* theese are add-ons to the current speed of
                              the attacker */
  double downslashpower_y;
  int downslashdmg;
  double upswingpower_x;
  double upswingpower_y;
  int upswingdmg;
  signed int upswingdmgtime;
  double slashpower_x;
  double slashpower_y;
  int slashdmg;
  int controllerid; //what controller's id that's used for input
  signed int slashdmgtime;
  double uppercutpower_x; /* theese are add-ons to the current speed of the
                             attacker */
  double uppercutpower_y;
  int uppercutdmg;
  signed int uppercutdmgtime;
  int runframe;
  int wallslidetimeleft;
  int wallslidetimedefault;
  double walleappowerx;
  double walleappowery;
  double wallslidespeed;
  int wsattrec; //wallslide attack recovery
  int wsattrecdef; //wallslide attack recovery default
  int steps_until_picswitch;
  int legspeed;
  signed int uppercutdeftime; //default amount of frames to show uppercut
  signed int uppercuttimeleft; //time left until move ends
  double land_penalty; //a number to decrease the aspeed_x with when landed
  int recovery; //recovery left
  int recovery_hurt; //default recovery if hurt
  int safe_default; //default time to get "safe"
  enum Playerstate state; //(see PS_* for possible states)
  double x_acc;
  double x_ret;
  double x_airacc;
  double x_airret; //this is an extension of the general air_break, should be 0
  double x_maxspeed;

  double leap_power; /* the power of the leap! */
  double leap_breakin; /* a multiplier on how fast the player breaks in if
                          jump-button is released */
  bool leap_breakingin; /* as long as the jump button is down, there's no need
                         to break in */

  signed int slash_stun; //default stun for slash
  signed int downslash_stun; //default stun for downslash
  signed int downslash_stunbig; /* default stun for downslash if uppercut was
                                   also used */
  signed int stun;
  Creature *last_attacker;
  bool look_right;
  int attacktimes; //amount of attacks made during one leap

  double lastaspeed_x;
  double lastaspeed_y;

  Aitools *aitools;
 public:
  void set_look_right(bool ilook_right);
  void lost_slash(Swordslash *ss);
  bool looking_right();
  int get_controllerid();
  void set_controllerid(int id);
  void set_skin(int id);
  int get_skin();
  Controller *input;
  void hurt(Fxfield *fxf);
  void think();
  void paint(SDL_Surface **screen);
  void move_x(int x_diff);
  void move_y(int y_diff);
  std::string generate_save_string();
  Player( SDL_Surface ** screen, Aitools *iaitools, Sint16 x, Sint16 y,
          Map *mapworld, int ilook_right, int iteam, int controllerid,
          int skin);
  ~Player();
};

#endif //__PLAYER_H__
