#ifndef __MAP_H__
#define __MAP_H__

#include <SDL/SDL.h>
#include "player.h"
#include "platform.h"
#include "creature.h"
#include "grid.h"
#include "fxfield.h"
#include <string>
#include "hashtable_gen.h"
#include "trigger.h"
#include "location.h"

using namespace std;

#define CREATURE_MAX 300
#define PLATFORM_MAX 150
#define FXFIELD_MAX 500
#define TRIGGER_MAX 50
#define LOCATION_MAX 50

#define MAPWIDTHMAX 10000
#define MAPWIDTHMIN 300
#define MAPHEIGHTMAX 10000
#define MAPHEIGHTMIN 300

class Creature;
class Grid;
class Player;
class Pomgob;
class Zombo;
class Aitools;

class MovingCreature
{
 public:
  int x_speed;
  int y_speed;
  Creature *creature;
  MovingCreature *next;
};

class Map
{
 private:
  MovingCreature potential_motion[CREATURE_MAX];
  int next_motion_id;
  string bg_file;
 public:
  Hashtable_gen<Creature *> *creature_log; /* index for all creatures ever been
                                              on the map */
  Hashtable_gen<Platform *> *platform_log;

  Creature *main_player;

  /* motion is a linked list of all moving creatures. the gravity function
     will add creatures here, and then physics will move them all and when they

     have no move points left, they will be removed from the list.
     potential_motion holds all space needed for motion so no new or delete is
     neccecairy. Note that every frame motion will be empty at the begninning
     next_motion_id is a slot in potential_motion
  */
  MovingCreature *motion;
  MovingCreature *new_moving_creature();

  int get_platform_max();
  int get_creature_max();
  int get_fxfield_max();
  int get_trigger_max();
  int get_location_max();

  int free_map();
  int save_map(const char filename[]);
  int new_map( SDL_Surface **screen, int iwidth, int iheight,
                    string ibackgroundpic);
  int load_mapfile( const char filename[], SDL_Surface **screen,
                    Aitools *aitools);
  void set_bg(const char filename[], SDL_Surface **screen);
  void register_platform(Platform *p);
  void unregister_platform(Platform *p);
  void generate_graphics_surroundings(const SDL_Rect &r); //generate graphics
                                         //on everythin that might touch r

  void put_in_boundary(Platform *p); //makes sure that p is inside the boundary
  void put_in_boundary(Location *l); //makes sure that l is inside the boundary

  void register_creature(Creature *c);
  void unregister_creature(Creature *c);
  void register_fxfield(Fxfield *f);
  void unregister_fxfield(Fxfield *f);
  void register_location(Location *l);
  void unregister_location(Location *l);

  Fxfield **get_free_fxfield();
  void reset_motion();
  Map();
  ~Map();
  SDL_Surface *background;
  Grid *grid;
  SDL_Rect boundary;
  SDL_Rect pos; //src-rektangeln till blittningen för bakgrunden
  Fxfield **fxfield;
  Platform **platform; //array med pekare till de platforms som finns
  Creature **creature; //array med pekare till de creatures som finns
  Trigger **trigger; //array med pekare till de triggers som finns
  Location **location; //array med pekare till de locations som finns
};

#endif //__MAP_H__
