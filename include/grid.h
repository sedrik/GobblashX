#ifndef __GRID_H__
#define __GRID_H__

#include "player.h"
#include "creature.h"
#include "platform.h"
#include "fxfield.h"
#include "location.h"

#define SQUARE_WIDTH 100
#define SQUARE_HEIGHT 100

class Creature;

template <class TYP>
class Node{
 public:
  TYP *link;
  Node<TYP> *next;
  Node<TYP> *prev;
};

class Square{
 public:
  Node<Creature> *creatures;
  Node<Platform> *platforms;
  Node<Fxfield> *fxfields;
  Node<Location> *locations;
  Square();
  ~Square();
};

class Grid{
 private:
  int first_x; /* x-value for the first(upper-right) Square */
  int first_y; /* y-value for the first Square */
  int rows; /* amount of Rows */
  int cols; /* amount of Squares in each row */

  int square_width;
  int square_height;

 public:
  int get_rows();
  int get_cols();
  void set_size( const SDL_Rect map_borders );
  Square *examine(int px_x, int px_y); /* in-arguments are in pixels
                                             returns a pointer to the square */
  void px_to_coord(int x, int y, int &square_x, int &square_y);
  SDL_Rect coord_to_rect(int square_x, int square_y);
  Grid();
  ~Grid();

  Square **box; /* box is the coordinate system where box[i] is a pointer to
                   a column which is an array with the Squares in the row */

};

#endif //__GRID_H__
