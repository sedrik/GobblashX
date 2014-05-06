#include "grid.h"

Square::Square(){
  creatures = NULL;
  platforms = NULL;
  fxfields = NULL;
  locations = NULL;
}

Square::~Square(){
  Node<Creature> *tempc;
  Node<Platform> *tempp;
  Node<Fxfield> *tempf;
  Node<Location> *templ;

  while(creatures != NULL){
    tempc = creatures;
    creatures = creatures->next;
    delete tempc;
  }

  while(platforms != NULL){
    tempp = platforms;
    platforms = platforms->next;
    delete tempp;
  }

  while(fxfields != NULL){
    tempf = fxfields;
    fxfields = fxfields->next;
    delete tempf;
  }

  while(locations != NULL){
    templ = locations;
    locations = locations->next;
    delete templ;
  }
}

int Grid::get_rows(){
  return rows;
}

int Grid::get_cols(){
  return cols;
}

Grid::Grid(){
  box = NULL;
  first_x = 0;
  first_y = 0;
  rows = 0;
  cols = 0;
}

Grid::~Grid(){
  for(int i = 0; i < cols; i++)
    delete [] box[i];
  delete [] box;
}

Square *Grid::examine(int x, int y){
  int sx, sy;
  px_to_coord(x,y,sx,sy);
  if (sx != -1 && sy != -1)
    return &box[sx][sy];
  else
    return NULL;
}

SDL_Rect Grid::coord_to_rect(int square_x, int square_y){
  SDL_Rect ret;
  ret.x = square_x * square_width + first_x;
  ret.y = square_y * square_height + first_y;
  ret.w = square_width;
  ret.h = square_height;
  return ret;
}

void Grid::px_to_coord(int x, int y, int &square_x, int &square_y){
  square_x = (x - first_x) / square_width;
  square_y = (y - first_y) / square_height;
  if(x < first_x || y < first_y || square_x >= cols || square_y >= rows){
    square_x = -1;
    square_y = -1;
  }
}

void Grid::set_size( const SDL_Rect map_boundary ){
  if (box){
    for(int i = 0; i < cols; i++){
      delete [] box[i];
    }
    delete [] box;
  }
  first_x = map_boundary.x;
  first_y = map_boundary.y;
  square_height = SQUARE_HEIGHT;
  square_width = SQUARE_WIDTH;
  rows = map_boundary.h / square_height;
  if (map_boundary.h % square_height){
    rows += 1;
  }
  cols = map_boundary.w / square_width;
  if (map_boundary.w % square_width){
    cols += 1;
  }

  box = new Square*[cols];
  for(int i = 0; i < cols; i++)
    box[i] = new Square[rows];
}
