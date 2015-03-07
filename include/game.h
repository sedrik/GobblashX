#ifndef __GAME_H__
#define __GAME_H__

#include "SDL/SDL.h"
#include "controllerkey.h"
#include "aitools.h"

#define KEY_EXIT 0
#define KEY_PAUSE 1
#define KEY_COLLISIONMODE 2
#define KEY_SHOWSYM 3
#define KEY_SHOWPOS 4
#define KEY_STATS 5
#define KEY_SPEED 6
#define KEY_RESSURECT 7
#define KEY_SPAWN 8
#define KEY_TOTAL 9

#define MAXIMUM_KEY_VALUE 1024

class Aitools;
class Map;

enum Gamestate {GAME_COMPLETED, GAME_OVER, GAME_PLAYING};

class Game{
 private:
  Gamestate game_state;

  //variables to keep track of joystickbuttons
  int joysticks;
  SDL_Joystick **joystick;
  Sint32 **joykeydown;
  Sint32 **joykeyup;

  //varaiables to keep track of keypresses
  Sint32 keydown[MAXIMUM_KEY_VALUE]; //the last frame button index was key_downed
  Sint32 keyup[MAXIMUM_KEY_VALUE]; //the last frame button index was key_upped
  int gamekeymap[KEY_TOTAL]; /* ingame keys, not for controlling units */

  void handle_input();

  //variables to keep track of time
  int currframe; /* Logs what time each frame started */
  Sint32 framenum;
  Uint32 framelog[20];

  Uint32 frametime;
  Uint32 frametimefracup;
  Uint32 frametimefracdown;
  Uint32 nextextraframe; //hold data on when frac has looped over
  Uint32 nextframe; //hold the time when the next frame should be played
  double fps;

 public:
  bool quit;
  bool paused;
  Map *world;
  Aitools *aitools;

  /* fps related stuff */
  void set_frametime(Uint32 frametime, Uint32 frametimefrac_up,
                     Uint32 frametimefrac_down);
  void wait_until_next_frame();
  void next_frame();
  bool playing();
  void complete_game();
  Gamestate get_state();
  double get_fps();
  Sint32 get_time();

  /* input related stuff */
  void get_buttoninfo(const Controllerkey &conkey, Sint32 &uptime,
                       Sint32 &downtime);
  void get_input(); // gets input from SDL
  void set_world(Map *iworld);
  void set_aitools(Map *iworld);

  Game();
  ~Game();
};

extern Game *game;

#endif // __GAME_H__
