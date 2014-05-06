#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <SDL/SDL.h>

class Platform
{
 private:
  static signed long long nextid;
  int id;
 public:
  Platform(int x, int y, int w, int h);

  virtual ~Platform();
  virtual void paint1(SDL_Surface **screen);
  virtual void paint2(SDL_Surface **screen);
  virtual void set_pos(int x, int y, int w, int h); //also adjusts precision
  virtual void set_pos_x(int x);
  virtual void set_pos_y(int y);
  virtual void set_pos_w(int w);
  virtual void set_pos_h(int h);
  virtual void generate_graphics();
  SDL_Rect pos; //don't set this drectly, use set_pos instead!
  int type;
  int getid();
  int x1,x2,y1,y2;
};

#endif // __PLATFORM_H__
