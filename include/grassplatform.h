#ifndef __GRASSPLATFORM_H__
#define __GRASSPLATFORM_H__

#include "SDL/SDL.h"
#include "platform.h"
#include "map.h"

#define SPECNONE 574 /* special pics, this one symbolizes nothing special */
/*
  every squares conrner is of one of theese, they are here to make grass and
  mud special paintings, when appropriate. mapeditor will control them
*/

/* the upper-left corner */
#define SPEC_UL_F 575 /* foot of wall */
#define SPEC_UL_E 576 /* edge */

/* the upper corners */
#define SPEC_U_P 577 /* plain continues (both for UL and UR) */
#define SPEC_L_W 578 /* wall continues (both for UL and DL) */

/* the upper-right corner */
#define SPEC_UR_F 579 /* foot of wall */
#define SPEC_UR_E 580 /* edge */

/* the right corners */
#define SPEC_R_W 581 /* both for UR and DR */

/* the down-left corner */
#define SPEC_DL_E 582 /* edge */
#define SPEC_DL_R 584 /* this roof is next to a wall */

/* the downcorners */
#define SPEC_D_P 583 /* plain (both for UR and UL) */

/* the down-right corner */
#define SPEC_DR_E 585 /* edge */
#define SPEC_DR_R 586 /* this roof is next to a wall */

class Map;

class Specdata{
 public:
  int specpic;
  void setspecdata(char ul, char ur, char dl, char dr); //calls from mapload
  int upleftspec; // theese are of type SPEC_*
  int uprightspec;
  int downleftspec;
  int downrightspec;
};

class Grassplatform : public Platform{
 private:
  static int grassplatform_count;
  static SDL_Surface *tilesurf;
  Specdata **paintpics;
  int paintpics_rows;
  int paintpics_cols;

  Map *world;

  SDL_Rect get_pic(int picnum);
  void setdirtpic( SDL_Rect &r, int dirtpic);
  void setspecpic( SDL_Rect &r, SDL_Rect &target, int x, int y, int specpic);
  /* specpic is SPEC_* */
 public:
  void generate_graphics();
  Grassplatform(int x, int y, int w, int h, SDL_Surface **screen, Map *world);
  ~Grassplatform();
  void set_pos(int x, int y, int w, int h);
  void set_pos_x(int x);
  void set_pos_y(int y);
  void set_pos_w(int w);
  void set_pos_h(int h);
  void paint1(SDL_Surface **screen); //rough paint
  void paint2(SDL_Surface **screen); // paint details
  int type;
};

#endif //__TILEDPLATFORM_H__
