#ifndef __SCANRESULT_H__
#define __SCANRESULT_H__

#include "map.h"
#include "platform.h"
#include "creature.h"
#include "scanresult.h"
#include "SDL/SDL.h"
#include "hashtable.h"

class Scanresult{
 private:
  Hashtable *creatureIDs;
  Hashtable *platformIDs;
 public:
  Scanresult();
  ~Scanresult();
  void reset();

  int add_creature(int cid); /* adds cid to CreatureIDs, returns 0 if it
                                were ok, and returns nonzero if something
                                failed. Note that when this function is used,
                                duplicates in the array are discarded */
  int add_platform(int pfid); /* same as add_creautre but for platforms */

  int count_creatures(); /* returns the creatureamount from a scan */
  int count_platforms(); /* returns the platformamount from a scan */
  int takeout_creature(); /* returns the id of a creature from the scan,
                           returns -1 if nothing is left */
  int takeout_platform(); /* returns the id of a platform from the scan,
                           returns -1 if nothing is left */
};

#endif // __SCANRESULT_H__
