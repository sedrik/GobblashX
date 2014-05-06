#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include "map.h"
#include "SDL/SDL.h"

class Aitools;

void gravity(Map &world);
void physics(Map &world);
void checkfxfields(Map &world);
void checklocations(Aitools *aitools, Map &world);
bool intersect_rect(const SDL_Rect &a, const SDL_Rect &b, SDL_Rect &intersect);


#endif // __PHYSICS_H__
