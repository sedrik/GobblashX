#ifndef __AITOOLS_H__
#define __AITOOLS_H__

#include "map.h"
#include "platform.h"
#include "creature.h"
#include "scanresult.h"

enum Scanfor { CREATURES = 0x1, PLATFORMS = 0x2 };

class Map;

class Aitools{
 private:
  Map *world;
 public:
  Aitools(Map *iworld);

  Creature *identify_creature(int id); /* returns a pointer to the creature
                                              with the given id */
  Platform *identify_platform(int id); /* returns a pointer to the platform
                                              with the given id */
  int random(int min, int max); /* returns a random number x, where
                                   min <= x <= max */

  int box_scan( Scanfor scan_for, Scanresult &res, const SDL_Rect &box );
  /* Scans for scan_for in box and putting the result in res
     returns 0 if everything was ok, otherwise returns something nonzero */

  bool px_pf_scan( int x, int y); /* is there a platform on the given coords?*/

  /* scan for creatures and places then in the &res variable*/
  void scan_down(int xstart, int ystart, Scanresult &res);
  void scan_up(int xstart, int ystart, Scanresult &res);
};


#endif // __AITOOLS_H__
