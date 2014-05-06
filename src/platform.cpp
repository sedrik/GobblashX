#include "platform.h"
#include "graphics.h"
#include "settings.h"

signed long long Platform::nextid = 0;

int Platform::getid(){
  return id;
}

Platform::Platform(int x, int y, int w, int h){
  id = nextid;
  nextid ++;

  set_pos(x,y,w,h);

  x1 = 0;
  x2 = 0;
  y1 = 0;
  y2 = 0;
}

void Platform::set_pos(int x, int y, int w, int h){
  set_pos_x(x);
  set_pos_y(y);
  set_pos_w(w);
  set_pos_h(h);
}
void Platform::set_pos_x(int x){
  pos.x = x;
}
void Platform::set_pos_y(int y){
  pos.y = y;
}
void Platform::set_pos_w(int w){
  if (w < 1)
    w = 1;
  pos.w = w;
}
void Platform::set_pos_h(int h){
  if (h < 1)
    h = 1;
  pos.h = h;
}

void Platform::generate_graphics(){
}

Platform::~Platform(){
}

void Platform::paint2(SDL_Surface **screen){
}

void Platform::paint1(SDL_Surface **screen){
  SDL_Rect target;
  prepare_to_show(target, pos);

  SDL_FillRect(*screen, &target,
               SDL_MapRGB((*screen)->format, 0, 0,0));
}
