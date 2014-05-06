#ifndef __TEXT_H__
#define __TEXT_H__

#include <string>
#include <SDL/SDL.h>

using namespace std;

class Text{
 private:
  static SDL_Surface *tilesurf;
  static int count;
  int getpos(char letter); //gets the x-coord where the letter is in the surf
 public:
  Text(SDL_Surface **screen, int x, int y, const char *def, int imenu);
  ~Text();
  void paint(SDL_Surface **screen);
  void calc_size(Uint16 &w, Uint16 &h); //returns the width and height
  bool visible;
  int menu;
  string str;
  int x,y;
};

#endif //__FONT_H__
