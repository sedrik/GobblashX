
#ifndef __LOCATION_H__
#define __LOCATION_H__

#include "SDL/SDL.h"
#include <string>

class Trigger;

class Location{
 public:
  int x1,x2,y1,y2;
  bool totalcol; /* Specifies whether the creature in question needs to be
                    inside this location totally or if it's enough with just
                    a touch */
  int act_trigger; /* what trigger to activate, this variable is the tag of
                    the trigger */
  Trigger *act_trigptr; /* pointer to the trigger to activate! -
                         this is only used in the game for fast access to it */
  int act_team; /* what team that may touch this location, -1 means any team */

  int tag; //a way to identify a location. 0 means "not used"
  bool act_pomgob; /* activates for pomgobs */
  bool act_player; /* activates for player */
  bool act_sbbflyer; /* activates for sbbflyer */
  bool act_zombo; /* activates for zombo */

  SDL_Rect pos;
  void paint(SDL_Surface **screen);

  std::string generate_save_string();

  Location();
  Location(std::string initline);
};

#endif //__LOCATION_H__
