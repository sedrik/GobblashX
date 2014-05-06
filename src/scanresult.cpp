#include "scanresult.h"

Scanresult::Scanresult(){
  creatureIDs = new Hashtable(CREATURE_MAX);
  platformIDs = new Hashtable(PLATFORM_MAX);
}

Scanresult::~Scanresult(){
  if (creatureIDs != NULL)
    delete creatureIDs;
  if (platformIDs != NULL)
    delete platformIDs;
}

void Scanresult::reset(){
  if (creatureIDs != NULL)
    delete creatureIDs;
  if (platformIDs != NULL)
    delete platformIDs;

  creatureIDs = new Hashtable(CREATURE_MAX);
  platformIDs = new Hashtable(PLATFORM_MAX);
}

int Scanresult::add_creature(int cid){
  creatureIDs->insert(cid);
  return 0;
}

int Scanresult::add_platform(int pfid){
  platformIDs->insert(pfid);
  return 0;
}

int Scanresult::count_creatures(){
  return creatureIDs->count();
}

int Scanresult::count_platforms(){
  return platformIDs->count();
}

int Scanresult::takeout_creature(){
  int ret, success;
  success = creatureIDs->takeout(ret);
  if (success == 0)
    return ret;
  else
    return -1;
}

int Scanresult::takeout_platform(){
  int ret, success;
  success = platformIDs->takeout(ret);
  if (success == 0)
    return ret;
  else
    return -1;
}
