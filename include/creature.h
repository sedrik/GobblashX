#ifndef __CREATURE_H__
#define __CREATURE_H__

#include "SDL/SDL.h"
#include "platform.h"
#include <string>

class Fxfield;
class Map;

class Creature
{
 private:
  static signed long long nextid; /* a number is given to each creature, when
                                     all numbers has been used, it restarts
                                     from 0 */
  int id; /* each creature gets an unique id */
 public:
  int getid();
  virtual void think();
  virtual void hurt(Fxfield *fxf);
  virtual void paint(SDL_Surface **screen);
  virtual std::string generate_save_string();
  virtual void load_from_string(std::string);
  void paint_lifebar(SDL_Surface **screen);
  virtual void teleported(); /* informs the creature that it's been teleported
                              */
  virtual ~Creature();
  Creature();
  virtual void move_x(int x_diff); /* Moves the creature x_diff pixels in
                                      x-direction */
  virtual void move_y(int y_diff); /* same as above but for y-direction */
  int x1,x2,y1,y2; /* the squares the creatues is in (it's a range) */
  int team; /* What team the creature is in */
  int safe; /* while safe > 0 this creature can not be attacked by a fxfield */
  SDL_Rect pos; /* The collisionrectangle for the creature */
  double aspeed_x; /* acctual speed x (rounded down) */
  double aspeed_y;
  double rspeed_x; /* remaining speed since last frame, (This is used only by
                      the physics engine, and should not be interfered by
                      sub-classes of creature
                   */
  double rspeed_y;

  int hp;
  int hpmax;
  bool dead; //is set by the AI if the hp is 0 and the creature is ready to die
  Map *world;
  Platform *below; /* it's up the the phys_engine to tell what i'm
                            standing on i.e the platform one pixel below*/
  Platform *above;
  Platform *rightof;
  Platform *leftof;
  int immune_to_gravity;
};

#endif //__CREATURE_H__
