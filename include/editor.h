#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "SDL/SDL.h"
#include "platform.h"
#include "grassplatform.h"
#include <string>
#include "text.h"
#include "player.h"
#include "creature.h"
#include "pomgob.h"
#include "sbbflyer.h"
#include "zombo.h"
#include "blubber.h"

#define MAPHEIGHTSTEP 100
#define MAPWIDTHSTEP 100

#define LOCATIONTAGMAX 50
#define TRIGGERTAGMAX 50

#define EKEY_EXIT 0
#define EKEY_COLLISIONMODE 1
#define EKEY_GRAVITY 2
#define EKEY_GOTOHERO 3
#define EKEY_LEFT 4
#define EKEY_RIGHT 5
#define EKEY_DOWN 6
#define EKEY_UP 7
#define EKEY_CLEARMARK 8
#define EKEY_SHIFT 9
#define EKEY_RESIZEPLATFORM 10
#define EKEY_TOTAL 11

#define MAXIMUM_KEY_VALUE 1024

enum Menus {
  MENU_MAIN,
  MENU_EDITORMAIN,
  MENU_CREATUREMAIN,
  MENU_MAP,
  MENU_PLATFORM,
  MENU_CREATUREPOMGOB,
  MENU_CREATUREPLAYER,
  MENU_CREATURESBBFLYER,
  MENU_CREATUREZOMBO,
  MENU_CREATUREBLUBBER,
  MENU_PLATFORM_NONE,
  MENU_PLATFORM_GRASS,
  MENU_TRIGGER,
  MENU_LOCATION
};

#define MENUBUTTONSTOT 200
#define MENUTEXTTOT 100

#define BGPICSTOT 2

class Button{
 private:
  static SDL_Surface *tilesurf;
  static int buttoncount;
 public:
  void paint(SDL_Surface **screen);
  Button(SDL_Surface **screen, int srcx, int srcy, int w, int h, int posx,
         int posy, int imenu, int ievent);
  ~Button();

  SDL_Rect pos;
  SDL_Rect src;
  SDL_Rect srcdown;

  bool visible; // decides whether this button is visible/clickable */
  int menu; /* what menu this button will appear on */
  int event; /* what event will happen when this button is pressed */
  bool down;
};

class Editor{
 private:
  Map *world;
  SDL_Surface **screen;

  int buttondownindex; /* what button that the mouse on when the mousebutton
                          was downed */
  Button *button[MENUBUTTONSTOT];
  Text *text[MENUTEXTTOT];
  int mousex, mousey, mousebuttons;
  int mapmousexdown, mapmouseydown, mapmousex, mapmousey;
  int lastmapmousex, lastmapmousey, lastmousebuttons;
  bool resizing;
  SDL_Rect downrect; /* pos for the pf/location that is beeing moved at the
                        time when the moving was started */
  bool worldmouse; // true if mouse was downed on cliprectworld

  int tool; // EDITORTOOL_PLATFORM

  int menuwidth;
  bool menuvisible;
  int currmenu; // the current menu that's showing
  int mo_mapwidth; //mo_ menu-option
  int mo_mapheight;
  string mo_file;
  string mo_folder;
  string mo_bgpic[BGPICSTOT];
  int mo_currbgpic;

 //the default value for newly created creatures def*
  bool defplalr; //default player look right
  int defplateam; //default player team
  int defplacon; //default player controller
  int defplaskin; //default player skin

  int defpomteam; //default pomgob team
  bool defpomlr; //default pomgob look right
  bool defpomjump; //default pomgob jump

  int defsbbteam; //default sbbflyer teal
  bool defsbblr; //default sbbflyer look right

  int defzomteam; //default sbbflyer teal
  bool defzomlr; //default sbbflyer look right

  int defbluteam; //blubbers stats - team
  bool defblulr;  // look right
  bool defblugoup; // falling

  int messagetime;
  Text *message;

  Sint32 mouselastdown;
  Sint32 keydown[MAXIMUM_KEY_VALUE]; //the last frame button index was key_downed
  Sint32 keyup[MAXIMUM_KEY_VALUE]; //the last frame button index was key_upped
  int scrollspeed;
  int keymap[EKEY_TOTAL];

  void handle_input();
  void toggle_menu();

  void button_event(int eventnum);
  void cib(Creature *c); /* checks if given creature is
     inside the boundary of the map, if that's not the case, it gets moved
     inside of the boundary */

  Creature *marked_creature; //a creature is marked while the mouse is down
  Creature *selected_creature; /* a creature is selected from the time mouse
              was downed on it until the mouse was downed on something else */
  Platform *marked_platform;
  Platform *selected_platform; // same as selected_creature but for platforms

  Location *marked_location; // same as above...
  Location *selected_location;

  int selected_trigger; /* the id of the selected trigger, -1 if none is
                           selected */

  template <class TYP>
  void add_to_string(string &s, TYP t);
  void submenu_show(int menu);
  void show_message(string msg, int time);

  int get_next_ltag(int starttag, bool next);
  /* if next -> returns the first free loctag > starttag
     else -> returns the first free loctag < starttag
             returns 0 if there was no free or if
             starttag = 1 */
  int get_next_ttag(int starttag, bool next); // same as above, but for trigger
  int get_next_trigger(int starttag, bool next); /* returns the INDEX of
                                                      the trigger with the next
                                                      tag */
  bool control_shrink(SDL_Rect sr); /* Checks if there are any creature or
     platform on the given rect, if that's the case message and messagetime
     is set and false is returned. true is returned when nothing is in the
     rect */
  void unregister_everything();
  void register_everything();
 public:
  void get_input();
  void paint_menu(SDL_Surface **screen);
  void paint_message(SDL_Surface **screen);
  bool quit;
  bool gamecontrollers;
  bool creatures_think;
  bool gravity;
  Editor(SDL_Surface **screen, Map &iworld, string mapfolder, string mapfile);
  ~Editor();
};

extern Editor *editor;

#endif // __EDITOR_H__
