#include <fstream>
#include <string>

#include "map.h"
#include "player.h"
#include "pomgob.h"
#include "graphics.h"
#include "grid.h"
#include "platform.h"
#include "game.h"
#include "pomgob.h"
#include "sbbflyer.h"
#include "zombo.h"
#include "blubber.h"
#include "grassplatform.h"
#include "filefunctions.h"
#include "settings.h"

using namespace std;

void Map::set_bg(const char filename[], SDL_Surface **screen){
  if (background)
    SDL_FreeSurface(background);

  bg_file = filename;
  background = load_optimized_pic(filename, screen, 0, 0, 0);
  if (background == NULL){
    printf("Map::set_bg: Failed to load pic:\"%s\"!\n", bg_file.c_str());
    exit(1);
  }
}

int Map::save_map(const char filename[]){
  ofstream mapfile;

  mapfile.open( filename );
  if ( !mapfile ) {
    printf("failed to open file: %s\n", filename);
    return 1;
  }


  mapfile << "boundary " << boundary.x << " " << boundary.y << " "  <<
    boundary.w << " "  << boundary.h << " "  << endl;
  mapfile << "platforms\n";
  for(int pfi = 0; pfi < PLATFORM_MAX; pfi++){
    if (platform[pfi] != NULL){
      mapfile << platform[pfi]->pos.x << " " << platform[pfi]->pos.y << " " <<
        platform[pfi]->pos.w << " " << platform[pfi]->pos.h  << " ";
      if (dynamic_cast<Grassplatform *>(platform[pfi]))
        mapfile << "g\n";
      else
        mapfile << "n\n";
    }
  }
  mapfile << "platformsout\n";
  mapfile << "creatures\n";
  for(int ci = 0; ci < CREATURE_MAX; ci++){
    if (creature[ci] != NULL){
      mapfile << creature[ci]->generate_save_string() << endl;
    }
  }
  mapfile << "creaturesout\n";
  mapfile << "locations\n";
  for(int li = 0; li < LOCATION_MAX; li++){
    if (location[li] != NULL){
      mapfile << location[li]->generate_save_string() << endl;
    }
  }

  mapfile << "locationsout\n";
  mapfile << "triggers\n";
  for(int ti = 0; ti < TRIGGER_MAX; ti++){
    if (trigger[ti] != NULL){
      mapfile << trigger[ti]->generate_save_string() << endl;
    }
  }
  mapfile << "triggersout\n";
  mapfile << "background " << bg_file << endl;

  mapfile.close();
  return 0;
}

int Map::load_mapfile(const char filename[], SDL_Surface **screen,
                      Aitools *aitools){
  string row, word;
  int rownum;
  ifstream mapfile;

  bool boundary_found = false;

  next_motion_id = 0;
  motion = NULL;
  mapfile.open( filename );
  if ( !mapfile ) {
    printf("failed to open file: %s\n", filename);
    return 1;
  }

  //load in boundaries info
  rownum = 0;
  word = skipcomments(mapfile, row, rownum);
  while( word!="<eof>" ){
    if (word == "boundary"){
      if (not boundary_found)
        boundary_found = true;
      else{
        printf("%s:%d: Error: Boundary set two times!\n", filename,
               rownum);
        return 1;
      }
      word = first_word(row);

      if (safe_strtol(word, boundary.x))
        return 1;
      word = first_word(row);
      if (safe_strtol(word, boundary.y))
        return 1;
      word = first_word(row);
      if (safe_strtol(word, boundary.w))
        return 1;
      word = first_word(row);
      if (safe_strtol(word, boundary.h))
        return 1;

      grid->set_size(boundary);
    }
    else if (word == "platforms"){
      //load in all platforms by looping
      if (not boundary_found){
        printf("%s:%d: Error: Boundary not declared yet!\n", filename,
               rownum);
        return 1;
      }
      int pfi = -1;
      word = skipcomments(mapfile, row, rownum);
      while (word != "platformsout"){
        int x, y, w, h;
        pfi ++;
        if (pfi > get_platform_max()){
          printf("%s:%d: Error: Too many platforms"
                 " in file?\n", filename, rownum);
          return 1;
        }
        if (safe_strtol(word, x))
          return 1;
        word = first_word(row);
        if (safe_strtol(word, y))
          return 1;
        word = first_word(row);
        if (safe_strtol(word, w))
          return 1;
        word = first_word(row);
        if (safe_strtol(word, h))
          return 1;
        word = first_word(row);

        if (word[0] == 'g'){
          string buff; //buffert for each square

          if(w % 20 != 0 || h % 20 != 0){
            printf("%s:%d: Error: Wrong size on platform of type grass - "
                   "w and h must be divisable by 20\n", filename, rownum);
            exit(1);
          }

          platform[pfi] = new Grassplatform(x,y,w,h, screen, this);
        }
        else if (word == "n"){
          platform[pfi] = new Platform(x, y, w, h);
        }
        else{
          printf("%s:%d: Error: Wrong platform type\n", filename, rownum);
          return 1;
        }

        if(platform_log->insert( platform[pfi]->getid(), platform[pfi] )){
          printf("Failed to insert platform with id %d into the "
                 "platform_log\n", platform[pfi]->getid());
          exit(1);
        }

        word = skipcomments(mapfile, row, rownum);
      }

      //register all platforms on the grid
      for (int i = 0; i <= pfi; i++){
        register_platform(platform[i]);
      }
      for (int i = pfi + 1; i < get_platform_max(); i++){
        platform[i] = NULL;
      }
      for (int i = 0; i <= pfi; i++){
        if (platform[i] != NULL)
        platform[i]->generate_graphics();
      }
    }
    else if (word == "creatures"){
      int ci = -1;
      if (not boundary_found){
        printf("%s:%d: Error: Boundary not declared yet!\n", filename,
               rownum);
        return 1;
      }

      word = skipcomments(mapfile, row, rownum);
      while (word != "creaturesout"){
        ci ++;
        if (ci > get_creature_max()){
          printf("%s:%d: Error: Too many creatures\n", filename, rownum);
          return 1;
        }
        if (word == "player"){
          int x, y, team, look_right, controllerid, skin;
          word = first_word(row);
          if (safe_strtol(word, x))
            return 1;
          word = first_word(row);
          if (safe_strtol(word, y))
            return 1;
          word = first_word(row);
          if (safe_strtol(word, look_right))
            return 1;
          word = first_word(row);
          if (safe_strtol(word, team))
            return 1;
          word = first_word(row);
          if (safe_strtol(word, controllerid))
            return 1;
          word = first_word(row);
          if (safe_strtol(word, skin))
            return 1;
          if (controllerid < 0 || controllerid >= TOT_CONTROLLERS){
            printf("%s:%d: Error: Errorous controllerid, "
                   "should be 0 <= controllerid < %d\n", filename, rownum,
                   TOT_CONTROLLERS);
            return 1;
          }

          creature[ci] = new Player(screen, aitools, x, y, this, look_right,
                                    team, controllerid, skin);
          main_player = creature[ci];
          if(creature_log->insert( creature[ci]->getid(), creature[ci] )){
            printf("Failed to insert creature with id %d into the "
                   "creature_log\n", creature[ci]->getid());
            exit(1);
          }

        }
        else if (word == "pomgob"){
          int x, y, team;
          int look_right, standing_still;
          word = first_word(row);
          if (safe_strtol(word, x))
            return 1;
          word = first_word(row);
          if (safe_strtol(word, y))
        return 1;
          word = first_word(row);
          if (safe_strtol(word, look_right))
            return 1;
          word = first_word(row);
          if (safe_strtol(word, standing_still))
            return 1;
          word = first_word(row);
          if (safe_strtol(word, team))
            return 1;

          creature[ci] = new Pomgob(screen, x, y, look_right, standing_still,
                                    team, this);
          if(creature_log->insert( creature[ci]->getid(), creature[ci] )){
            printf("Failed to insert creature with id %d into the "
                   "creature_log\n", creature[ci]->getid());
            exit(1);
          }
        }
        else if (word == "sbbflyer"){
          int x, y, team;
          int look_right;
          word = first_word(row);
          if (safe_strtol(word, x))
            return 1;
          word = first_word(row);
          if (safe_strtol(word, y))
            return 1;
          word = first_word(row);
          if (safe_strtol(word, look_right))
            return 1;
          word = first_word(row);
          if (safe_strtol(word, team))
            return 1;

          creature[ci] = new Sbbflyer(screen, x, y, look_right, team, this);
          if(creature_log->insert( creature[ci]->getid(), creature[ci] )){
            printf("Failed to insert creature with id %d into the "
                   "creature_log\n", creature[ci]->getid());
            exit(1);
          }
        }
        else if (word == "zombo"){
          int x, y, team;
          int look_right;
          word = first_word(row);
          if (safe_strtol(word, x))
            return 1;
          word = first_word(row);
          if (safe_strtol(word, y))
            return 1;
          word = first_word(row);
          if (safe_strtol(word, look_right))
            return 1;
          word = first_word(row);
          if (safe_strtol(word, team))
            return 1;

          creature[ci] = new Zombo(screen, x, y, look_right, team, this,
                                   aitools);
          if(creature_log->insert( creature[ci]->getid(), creature[ci] )){
            printf("Failed to insert creature with id %d into the "
                   "creature_log\n", creature[ci]->getid());
            exit(1);
          }
        }
        else if (word == "blubber"){
          creature[ci] = new Blubber(screen, aitools, this, row);
          if(creature_log->insert( creature[ci]->getid(), creature[ci] )){
            printf("Failed to insert creature with id %d into the "
                   "creature_log\n", creature[ci]->getid());
            exit(1);
          }
        }
        else{
          printf("Unknown creature %s\n", word.c_str());
          return 1;
        }
        word = skipcomments(mapfile, row, rownum);
      }
      //register all creatures on the grid
      for(int i = 0; i <= get_creature_max(); i++){
        if (creature[i] != NULL)
        register_creature(creature[i]);
      }

    }
    else if( word == "locations" ){
      int li = -1;
      if (not boundary_found){
        printf("%s:%d: Error: Boundary not declared yet!\n", filename,
               rownum);
        return 1;
      }
      word = skipcomments(mapfile, row, rownum); //skips the locations-word
      while (word != "locationsout"){
        li ++;
        if (li > get_location_max()){
          printf("%s:%d: Error: Too many locations\n", filename, rownum);
          return 1;
        }
        location[li] = new Location(row);
        register_location(location[li]);
        word = skipcomments(mapfile, row, rownum); //skips the locations-word
      }
    }
    else if (word == "triggers"){
      // load in all triggers
      if (not boundary_found){
        printf("%s:%d: Error: Boundary not declared yet!\n", filename,
               rownum);
        return 1;
      }
      int ti = -1;
      word = skipcomments(mapfile, row, rownum); //skips the triggers - line
      while (word != "triggersout"){
        ti ++;
        if (ti > get_location_max()){
          printf("%s:%d: Error: Too many triggers\n", filename, rownum);
          return 1;
        }
        if (word != ""){
          trigger[ti] = new Trigger(this, row);
          word = skipcomments(mapfile, row, rownum);
          /* now we assign the loc->act_trigptr to this trigger for all
             locations with this trigger as act_trig */
        }
      }
    }
    else if (word == "background"){
      word = first_word(row);
      if ( word != "" ){
        bg_file = word.c_str();
        background = load_optimized_pic( bg_file.c_str(), screen, 0, 0, 0 );
      }

      //set the targetrect for the background blitting
      pos.x = 0;
      pos.y = 0;
      pos.w = settings.screen_w;
      pos.h = settings.screen_h;
    }
    else{
      printf("%s,%d: Warning: %s: Unknown map-keyword\n", filename, rownum,
             word.c_str());
    }
    word = skipcomments(mapfile, row, rownum);
  }

  mapfile.close();
  return 0;
}

int Map::new_map(SDL_Surface **screen, int iwidth, int iheight,
                 string ibackgroundpic){
  next_motion_id = 0;
  motion = NULL;

  look.x = -100;
  look.y = -100;

  //load in boundaries info
  boundary.x = 0;
  boundary.y = 0;
  boundary.w = iwidth;
  boundary.h = iheight;

  grid->set_size(boundary);

  //register all platforms on the grid
  for (int i = 0; i <= get_platform_max(); i++){
    if (platform[i] != NULL)
      register_platform(platform[i]);
  }

  //register all creatures on the grid
  for(int i = 0; i <= get_creature_max(); i++){
    if (creature[i] != NULL)
      register_creature(creature[i]);
  }

  //load backgroundpicture
  background = load_optimized_pic( ibackgroundpic, screen, 0, 0, 0 );

  //set the targetrect for the background blitting
  pos.x = 0;
  pos.y = 0;
  pos.w = settings.screen_w;
  pos.h = settings.screen_h;

  return 0;
}

void Map::put_in_boundary(Platform *p){
  if (p->pos.x + p->pos.w >
      boundary.x + boundary.w)
    p->set_pos_x( boundary.x + boundary.w -
                                p->pos.w - 1);
  if (p->pos.y + p->pos.h >
      boundary.y + boundary.h)
    p->set_pos_y(boundary.x + boundary.h -
                               p->pos.h - 1);
  if (p->pos.x < boundary.x)
    p->set_pos_x(boundary.x);

  if (p->pos.y < boundary.y)
    p->set_pos_y(boundary.y);

  if (p->pos.w > boundary.w)
    p->set_pos_w(boundary.w);

  if (p->pos.h > boundary.h)
    p->set_pos_h(boundary.h);
}

void Map::put_in_boundary(Location *l){
  if (l->pos.x < boundary.x)
    l->pos.x = boundary.x;
  if (l->pos.w > boundary.w)
    l->pos.w = boundary.w;
  if (l->pos.w + l->pos.x > boundary.x + boundary.w)
    l->pos.x = boundary.x + boundary.w - l->pos.w;

  if (l->pos.y < boundary.y)
    l->pos.y = boundary.y;
  if (l->pos.h > boundary.h)
    l->pos.h = boundary.h;
  if (l->pos.h + l->pos.y > boundary.y + boundary.h)
    l->pos.y = boundary.y + boundary.h - l->pos.h;
}

void Map::generate_graphics_surroundings(const SDL_Rect &r){
  int xslo, xshi, yslo, yshi;
  grid->px_to_coord(r.x - 1, r.y - 1, xslo, yslo);
  grid->px_to_coord(r.x + r.w, r.y + r.h, xshi, yshi);

  if (xslo < 0)
    xslo = 0;
  if (yslo < 0)
    yslo = 0;
  if (xshi >= grid->get_cols() || xshi == -1)
    xshi = grid->get_cols() - 1;
  if (yshi >= grid->get_rows() || yshi == -1)
    yshi = grid->get_rows() - 1;

  for(int x = xslo; x <= xshi; x++)
    for(int y = yslo; y <= yshi; y++){
      for(Node<Platform> *pf = grid->box[x][y].platforms;
          pf != NULL;pf = pf->next){
        pf->link->generate_graphics();
      }
    }

}

int Map::free_map(){
  for (int i = 0; i <= get_fxfield_max(); i++)
    if (fxfield[i] != NULL){
      delete fxfield[i];
      fxfield[i] = NULL;
    }

  for (int i = 0; i <= get_platform_max(); i++)
    if (platform[i] != NULL){
      delete platform[i];
      platform[i] = NULL;
    }

  for (int i = 0; i <= get_creature_max(); i++)
    if (creature[i] != NULL){
      delete creature[i];
      creature[i] = NULL;
    }

  for (int i = 0; i <= get_trigger_max(); i++)
    if (trigger[i] != NULL){
      delete trigger[i];
      trigger[i] = NULL;
    }

  for (int i = 0; i <= get_location_max(); i++)
    if (location[i] != NULL){
      delete location[i];
      location[i] = NULL;
    }

  SDL_FreeSurface(background);
  background = NULL;
  return 0;
};

Fxfield **Map::get_free_fxfield(){
  for(int i = 0; i <= get_fxfield_max(); i++){
    if (fxfield[i] == NULL)
      return &fxfield[i];
  }
  return NULL;
}

Map::Map(){
  next_motion_id = -1;
  main_player = NULL;
  motion = NULL;

  fxfield = new Fxfield*[FXFIELD_MAX +1];
  platform = new Platform*[PLATFORM_MAX +1];
  creature = new Creature*[CREATURE_MAX +1];
  trigger = new Trigger*[TRIGGER_MAX +1];
  location = new Location*[LOCATION_MAX +1];

  creature_log = new Hashtable_gen<Creature *>(CREATURE_MAX);
  platform_log = new Hashtable_gen<Platform *>(PLATFORM_MAX);

  for (int i = 0; i <= PLATFORM_MAX; i++)
    platform[i] = NULL;
  for (int i = 0; i <= CREATURE_MAX; i++)
    creature[i] = NULL;
  for (int i = 0; i <= FXFIELD_MAX; i++)
    fxfield[i] = NULL;
  for (int i = 0; i <= TRIGGER_MAX; i++)
    trigger[i] = NULL;
  for (int i = 0; i <= LOCATION_MAX; i++)
    location[i] = NULL;

  background = NULL;
  grid = new Grid;
}

Map::~Map(){
  delete grid;
  delete [] fxfield;
  delete [] platform;
  delete [] creature;
  delete [] trigger;
  delete [] location;

  delete creature_log;
  delete platform_log;

}

int Map::get_fxfield_max(){
  return FXFIELD_MAX;
}

int Map::get_platform_max(){
  return PLATFORM_MAX;
}

int Map::get_creature_max(){
  return CREATURE_MAX;
}

int Map::get_trigger_max(){
  return TRIGGER_MAX;
}

int Map::get_location_max(){
  return LOCATION_MAX;
}

void Map::register_platform(Platform *p){
  int x,y;
  int x1, x2, y1, y2;
  Square *square;
  Node<Platform> *insert;
  Node<Platform> *last;
  grid->px_to_coord(p->pos.x, p->pos.y, x1, y1);
  grid->px_to_coord(p->pos.x + p->pos.w - 1, p->pos.y + p->pos.h - 1, x2, y2);
  if(x1 == -1 || x2 == -1 || y1 == -1 || y2 == -1){
    printf("Error registrating platform - errorous squares\n");
    printf("square   x,y,w,h:%d,%d,%d,%d\n", p->pos.x, p->pos.y, p->pos.w,
           p->pos.h);
    printf("boundary x,y,w,h:%d,%d,%d,%d\n", boundary.x, boundary.y,
           boundary.w, boundary.h);
    exit(1);
  }
  for(x = x1; x <= x2; x++)
    for(y = y1; y <= y2; y++){
      square = &grid->box[x][y];
      insert = square->platforms;
      last = NULL;
      while(insert != NULL){
        last = insert;
        insert = insert->next;
      }
      insert = new Node<Platform>();
      insert->link = p;
      insert->next = NULL;
      if (last != NULL){
          last->next = insert;
      }
      else{
        square->platforms = insert;
      }
      insert->prev = last;
   }
  p->x1 = x1;
  p->x2 = x2;
  p->y1 = y1;
  p->y2 = y2;
}


void Map::unregister_platform(Platform *p){
  int x,y;

  Square *square;
  Node<Platform> *search;
  Node<Platform> *prev = NULL;

  for(x = p->x1; x <= p->x2; x++)
    for(y = p->y1; y <= p->y2; y++){
      square = &grid->box[x][y];
      search = square->platforms;
      prev = NULL;

      while(search != NULL && search->link != p){
        prev = search;
        search = search->next;
      }
      if (search == NULL){
        printf("Failed to unregister platform on %d,%d\n", x, y);
        exit(1);
      }
      if (prev == NULL)
        square->platforms = search->next;
      else
        prev->next = search->next;

      if (search->next != NULL)
        search->next->prev = prev;
    }
}


void Map::register_creature(Creature *p){
  int x,y;
  int x1, x2, y1, y2;
  Square *square;
  Node<Creature> *insert;
  Node<Creature> *last;
  grid->px_to_coord(p->pos.x, p->pos.y, x1, y1);
  grid->px_to_coord(p->pos.x + p->pos.w, p->pos.y + p->pos.h, x2, y2);
  if(x1 == -1 || x2 == -1 || y1 == -1 || y2 == -1){
    printf("Error registrating creature - errorous squares\n"
           "creaturepos (%d,%d) w:%d h:%d\n"
           "boundary (%d,%d) w:%d h:%d\n"
           "x1:%d x2:%d y1:%d y2:%d\n", p->pos.x, p->pos.y, p->pos.w,
           p->pos.h, boundary.x, boundary.y, boundary.w, boundary.h,
           x1, x2, y1, y2);
    exit(1);
  }
  for(x = x1; x <= x2; x++)
    for(y = y1; y <= y2; y++){
      square = &grid->box[x][y];
      insert = square->creatures;
      last = NULL;
      while(insert != NULL){
        last = insert;
        insert = insert->next;
      }
      insert = new Node<Creature>();
      insert->link = p;
      insert->next = NULL;
      if (last != NULL){
          last->next = insert;
      }
      else{
        square->creatures = insert;
      }
      insert->prev = last;
    }
  p->x1 = x1;
  p->x2 = x2;
  p->y1 = y1;
  p->y2 = y2;

}

void Map::unregister_creature(Creature *p){
  int x,y;


  Square *square;
  Node<Creature> *search;
  Node<Creature> *prev = NULL;

  for(x = p->x1; x <= p->x2; x++)
    for(y = p->y1; y <= p->y2; y++){
      square = &grid->box[x][y];
      search = square->creatures;
      prev = NULL;

      while(search != NULL && search->link != p){
        prev = search;
        search = search->next;
      }
      if (search == NULL){
        printf("Failed to unregister creature on %d,%d\n", x, y);
        exit(1);
      }
      if (prev == NULL)
        square->creatures = search->next;
      else
        prev->next = search->next;

      if (search->next != NULL)
        search->next->prev = prev;
    }
}

void Map::register_fxfield(Fxfield *p){
  int x,y;
  int x1, x2, y1, y2;
  int xlo, xhi, ylo, yhi;
  Square *square;
  Node<Fxfield> *insert;
  Node<Fxfield> *last;
  grid->px_to_coord(p->col_rect[0].x, p->col_rect[0].y, x1, y1);
  grid->px_to_coord(p->col_rect[0].x + p->col_rect[0].w -1,
                    p->col_rect[0].y + p->col_rect[0].h -1, x2, y2);
  if(x1 == -1 || x2 == -1 || y1 == -1 || y2 == -1){
    printf("Error registrating fxfield - errorous squares\n");
    exit(1);
  }
  xlo = x1;
  xhi = x2;
  ylo = y1;
  yhi = y2;
  for (int i = 1; i < p->col_rects; i++){
    grid->px_to_coord(p->col_rect[0].x, p->col_rect[0].y, x1, y1);
    grid->px_to_coord(p->col_rect[0].x + p->col_rect[0].w,
                      p->col_rect[0].y + p->col_rect[0].h, x2, y2);
    if(x1 == -1 || x2 == -1 || y1 == -1 || y2 == -1){
      printf("Error registrating fxfield - errorous squares\n");
      exit(1);
    }
    if (x1 < xlo)
      xlo = x1;
    if (x2 > xhi)
      xhi = x2;
    if (y1 < ylo)
      ylo = y1;
    if (y2 > yhi)
      yhi = y2;
  }

  for(x = xlo; x <= xhi; x++)
    for(y = ylo; y <= yhi; y++){
      square = &grid->box[x][y];
      insert = square->fxfields;
      last = NULL;
      while(insert != NULL){
        last = insert;
        insert = insert->next;
      }
      insert = new Node<Fxfield>();
      insert->link = p;
      insert->next = NULL;
      if (last != NULL){
          last->next = insert;
      }
      else{
        square->fxfields = insert;
      }
      insert->prev = last;
    }
  p->x1 = xlo;
  p->x2 = xhi;
  p->y1 = ylo;
  p->y2 = yhi;
}

void Map::unregister_fxfield(Fxfield *p){
  int x,y;

  Square *square;
  Node<Fxfield> *search;
  Node<Fxfield> *prev = NULL;
  for(x = p->x1; x <= p->x2; x++)
    for(y = p->y1; y <= p->y2; y++){
      square = &grid->box[x][y];
      search = square->fxfields;
      prev = NULL;

      while(search != NULL && search->link != p){
        prev = search;
        search = search->next;
      }
      if (search == NULL){
        printf("Failed to unregister fxfield \"%p\" on %d,%d\n", p, x, y);
        exit(1);
      }
      if (prev == NULL)
        square->fxfields= search->next;
      else
        prev->next = search->next;

      if (search->next != NULL)
        search->next->prev = prev;
    }
}

MovingCreature *Map::new_moving_creature(){
  if (next_motion_id > get_creature_max()) {
    printf("creaturemotion list is full! next_motion_id %d, creature_max %d",
           next_motion_id, get_creature_max());
    return NULL;
  }
  else{
    MovingCreature *ret = potential_motion + next_motion_id;
    next_motion_id ++;
    return ret;
  }
}

void Map::reset_motion(){
  motion = NULL;
  next_motion_id = 0;
}

/* for location!!! FIX */
void Map::register_location(Location *p){
  int x,y;
  int x1, x2, y1, y2;
  Square *square;
  Node<Location> *insert;
  Node<Location> *last;
  grid->px_to_coord(p->pos.x, p->pos.y, x1, y1);
  grid->px_to_coord(p->pos.x + p->pos.w - 1, p->pos.y + p->pos.h - 1, x2, y2);
  if(x1 == -1 || x2 == -1 || y1 == -1 || y2 == -1){
    printf("Error registrating location - errorous squares\n");
    printf("square   x,y,w,h:%d,%d,%d,%d\n", p->pos.x, p->pos.y, p->pos.w,
           p->pos.h);
    printf("boundary x,y,w,h:%d,%d,%d,%d\n", boundary.x, boundary.y,
           boundary.w, boundary.h);
    exit(1);
  }
  for(x = x1; x <= x2; x++)
    for(y = y1; y <= y2; y++){
      square = &grid->box[x][y];
      insert = square->locations;
      last = NULL;
      while(insert != NULL){
        last = insert;
        insert = insert->next;
      }
      insert = new Node<Location>();
      insert->link = p;
      insert->next = NULL;
      if (last != NULL){
          last->next = insert;
      }
      else{
        square->locations = insert;
      }
      insert->prev = last;
   }
  p->x1 = x1;
  p->x2 = x2;
  p->y1 = y1;
  p->y2 = y2;
}


void Map::unregister_location(Location *p){
  int x,y;

  Square *square;
  Node<Location> *search;
  Node<Location> *prev = NULL;

  for(x = p->x1; x <= p->x2; x++)
    for(y = p->y1; y <= p->y2; y++){
      square = &grid->box[x][y];
      search = square->locations;
      prev = NULL;

      while(search != NULL && search->link != p){
        prev = search;
        search = search->next;
      }
      if (search == NULL){
        printf("Failed to unregister location on %d,%d\n", x, y);
        exit(1);
      }
      if (prev == NULL)
        square->locations = search->next;
      else
        prev->next = search->next;

      if (search->next != NULL)
        search->next->prev = prev;
    }
}
