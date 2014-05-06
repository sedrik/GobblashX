#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include "keymappings.h"
#include "SDL/SDL.h"

#define TOT_CONTROLLERS 8

class Settings{
 public:
  double gravity_y;
  double air_break;
  bool collision_mode;
  bool follow_hero;
  bool debug;
  bool show_keysyms;
  bool show_locations;
  bool showhp;
  signed int screen_w;
  signed int screen_h;
  signed int screen_bpp;
  Uint32 screen_flags;
  void load_settings(const char filename[]);
  Keymappings conkeymap[TOT_CONTROLLERS];
  int joysticks;
  Settings();
};

extern Settings settings;

#endif // __SETTINGS_H__
