#include "blubber.h"
#include "graphics.h"
#include "settings.h"
#include <sstream>
#include <string>
#include "filefunctions.h"

#define SIZEOFPIC 65
#define WALK_SPEED 2
#define ANIM_SPEED 4

#define BLUBBER_SIZE_W 29 //hitbox size
#define BLUBBER_SIZE_H 29
#define BLUBBER_PIC_SIZE_W 65 //picture size
#define BLUBBER_PIC_SIZE_H 65
#define BLUBBER_PIC_MOVE_LEFT 18 //difference between picturepos and blubberpos
#define BLUBBER_PIC_MOVE_UP 18
#define BLUBBER_CENTRE_X 15 //centre of the blubber

#define BLUBBER_MAX_HP 7
#define BLUBBER_AURA_POWER_X 5
#define BLUBBER_AURA_POWER_Y 3
#define BLUBBER_AURA_DAMAGE 3
#define BLUBBER_INITIAL_FALL_SPEED 4

int Blubber::obj_count = 0;
SDL_Surface *Blubber::pic = NULL;

Blubber::~Blubber(){
  obj_count --;
  if (obj_count == 0){
    SDL_FreeSurface( pic );
    pic = NULL;
  }
}

std::string Blubber::generate_save_string(){
  stringstream ss;
  string ret, temp;
  ss << "blubber ";
  ss << pos.x;
  ss << " ";
  ss << pos.y;
  ss << " ";
  ss << team;
  ss << " ";
  ss << look_right;
  ss << " ";
  ss << get_falling();
  ss << " ";

  ret =ss.str();
  return ret;
}

Blubber::Blubber(SDL_Surface **screen, Aitools *iaitools, Map *iworld,
                std::string initline){
  std::string word;
  bool falling;
  word = first_word(initline);
  safe_strtol(word, pos.x);
  word = first_word(initline);
  safe_strtol(word, pos.y);
  word = first_word(initline);
  safe_strtol(word, team);
  word = first_word(initline);
  safe_strtol(word, look_right);
  word = first_word(initline);
  safe_strtol(word, falling);

  init_blubber(not falling, screen, iaitools, iworld);
}

Blubber::Blubber(int posx, int posy, int iteam, 
		 bool blook_right, bool start_on_roof, 
		 SDL_Surface **screen, Aitools *iaitools, Map *iworld ) {
  pos.x = posx;
  pos.y = posy;
  look_right = blook_right;
  team = iteam;

  init_blubber(start_on_roof, screen, iaitools, iworld);
}

void Blubber::init_blubber( bool start_on_roof, SDL_Surface **screen,
                            Aitools *iaitools, Map *iworld ) {

  if (obj_count == 0 && pic == NULL) {
    pic = load_optimized_pic( "graphics/blubber.bmp", screen,
                              0, 0, 255 );
  }
  obj_count ++;

  world = iworld;
  pos.w = BLUBBER_SIZE_W;
  pos.h = BLUBBER_SIZE_H;
  hp = BLUBBER_MAX_HP;
  hpmax = hp;
  anim = 0;

  aitools = iaitools;

  if (start_on_roof)
    set_state(SET_TO_ROOF);
  else
    if (below) 
      if (look_right)
	set_state(MOVE_R);
      else
	set_state(MOVE_L);
    else
      set_state(FALLING);
  
  source_pos.x = 0;
  source_pos.y = 0;
  source_pos.w = BLUBBER_PIC_SIZE_W;
  source_pos.h = BLUBBER_PIC_SIZE_H;
  target_pos.x = pos.x - BLUBBER_PIC_MOVE_LEFT;
  target_pos.y = pos.y - BLUBBER_PIC_MOVE_UP;
  target_pos.w = source_pos.w;
  target_pos.h = source_pos.h;
  aura = NULL;
  aurapower_x = BLUBBER_AURA_POWER_X;
  aurapower_y = BLUBBER_AURA_POWER_Y;
  
  time = 0;
}

void Blubber::set_falling(bool fall_down) {
  if (fall_down)
    set_state(FALLING);
  else
    set_state(SET_TO_ROOF);
}

bool Blubber::get_falling(void) {
  return (state == FALLING);
}

void Blubber::paint(SDL_Surface **screen){
  SDL_Rect target;
  if (not settings.collision_mode){
    prepare_to_show(target, target_pos);

    source_pos.x = anim * SIZEOFPIC;

    if(SDL_BlitSurface(pic, &source_pos, *screen,
                       &target)){
      printf("error: Blubber::paint\n");
      exit(1);
    }
  }
  else{
    prepare_to_show(target, pos);
    SDL_FillRect(*screen, &target,
                 SDL_MapRGB((*screen)->format,
                            60, 230, 200));
  }
}

void Blubber::hurt(Fxfield *fxf) {
  set_state(HURTING);
}

void Blubber::set_state(blubberstate nstate) {
  if (nstate != state) {
    time = 0;
    anim = 0;
  }
  state = nstate;
  Scanresult lookup;

  switch(state) {
  case SET_TO_ROOF:
    immune_to_gravity = true;
    aspeed_y = -10000;
    aspeed_x = 0;
    break;

  case WAITING:
    break;

  case FALLING:
    //Pictures
    if (look_right)
      source_pos.y = SIZEOFPIC * 1; 
    else 
      source_pos.y = 0;
    //UnPictures
    aspeed_x = 0;
    aspeed_y = BLUBBER_INITIAL_FALL_SPEED;
    immune_to_gravity = false;
    break;

  case LANDING:
    //Pictures
    if (look_right)
      source_pos.y = SIZEOFPIC * 2;
    else
      source_pos.y = SIZEOFPIC * 3;
    //UnPictures
    safe = 1;
    aura = NULL; // no damage but during falling
    aspeed_x = 0; // no sliding
    break;

  case STANDING:
    //Pictures
    source_pos.y = SIZEOFPIC * 12;
    //UnPictures
    immune_to_gravity = true;
    safe = 1;
    //This is DEATH!
    if (hp <= 0) {
      dead = true;
      aspeed_x = 0;
      aspeed_y = 0;
    }
    break;

  case HURTING:
    //Pictures
    if (aspeed_x < 0) {
      look_right = true; 
      source_pos.y = SIZEOFPIC * 11;
    }
    else {
      look_right = false;
      source_pos.y = SIZEOFPIC * 10;
    }
    //UnPictures
    aspeed_y -= 1.0;
    immune_to_gravity = false;
    aura = NULL;
    safe = 12;
    break;

  case MOVE_L:
    //Pictures
    source_pos.y = SIZEOFPIC * 4;
    //UnPictures
    look_right = false;
    aspeed_x = -WALK_SPEED;
    aspeed_y = 1;
    break;

  case MOVE_LU:
    //Pictures
    source_pos.y = SIZEOFPIC * 6;
    //UnPictures
    look_right = false;
    aspeed_y = -WALK_SPEED;
    aspeed_x = -1;
    break;

  case MOVEU_R:
    //Pictures
    source_pos.y = SIZEOFPIC * 9;
    //UnPictures
    look_right = false;
    aspeed_x = WALK_SPEED;
    aspeed_y = -1;
    break;

  case MOVE_R:
    //Pictures
    source_pos.y = SIZEOFPIC * 5;
    //UnPictures
    look_right = true;
    aspeed_x = WALK_SPEED;
    aspeed_y = 1;
    break;

  case MOVE_RU:
    //Pictures
    source_pos.y = SIZEOFPIC * 8;
    //UnPictures
    look_right = true;
    aspeed_x = 1;
    aspeed_y = -WALK_SPEED;
    break;

  case MOVEU_L:
    //Pictures
    source_pos.y = SIZEOFPIC * 7;
    //UnPictures
    look_right = true;
    aspeed_y = -1;
    aspeed_x = -WALK_SPEED;
    break;
  }
}

void Blubber::set_look_right(bool slook_right) {
  look_right = slook_right;
}

bool Blubber::looking_right(void) {
  return look_right;
}

void Blubber::toggle_look_right(void) {
  look_right = !look_right;
  set_state(state);
}

void Blubber::move_x(int x_diff) {
  pos.x += x_diff;
  target_pos.x += x_diff;

  if (aura != NULL){
    world->unregister_fxfield(aura);
    aura->col_rect[0].y = pos.y;
    aura->col_rect[0].h = pos.h;
    world->register_fxfield(aura);
  }
}

void Blubber::move_y(int y_diff) {
  pos.y += y_diff;
  target_pos.y += y_diff;

  if (aura != NULL){
    world->unregister_fxfield(aura);
    aura->col_rect[0].x = pos.x;
    aura->col_rect[0].w = pos.w;
    world->register_fxfield(aura);
  }
}

void Blubber::think(void) {
  switch(state) {
  case SET_TO_ROOF:
    if (above){
      if (look_right)
	    set_state(MOVEU_L);
      else
	    set_state(MOVEU_R);
    }
    break;

  case WAITING:
    set_state(FALLING); //ändra sedan
    break;

  case FALLING:
    time = (time + 1) % (3 * ANIM_SPEED);
    anim = int(time / ANIM_SPEED);
    cause_damage();
    safe = 1;
    if (below) set_state(LANDING);
    break;

  case LANDING:
    time++;
    if (time == ANIM_SPEED)
      anim = 1;
    else if (time == 2 * ANIM_SPEED)
      set_state(STANDING);
    break;

  case STANDING:
    time++;
    if (time == ANIM_SPEED)
      anim = 1;
    else if (time == 2 * ANIM_SPEED)
      anim = 2;
    else if (time == 3 * ANIM_SPEED) {
      if (look_right)
	set_state(MOVE_R);
      else
	set_state(MOVE_L);
    }
    break;

  case HURTING:
    if (below) {
      time++;
      if (time == ANIM_SPEED)
	anim = 1;
      else if (time == 2 * ANIM_SPEED)
	anim = 2;
      else if (time == 3 * ANIM_SPEED) {
	set_state(STANDING);
	toggle_look_right();
      }
    }
    break;

  case MOVE_L:
    time = (time + 1) % (3 * ANIM_SPEED);
    anim = int(time / ANIM_SPEED);
    if (leftof) 
      set_state(MOVE_LU); 
    else if (!below)
      if (!aitools->px_pf_scan(pos.x + pos.w, pos.y + pos.h) &&
	  !aitools->px_pf_scan(pos.x, pos.y + pos.h)) {
      set_state(FALLING);
      //toggle_look_right();
      }
    break;

  case MOVE_LU:
    time = (time + 1) % (3 * ANIM_SPEED);
    anim = int(time / ANIM_SPEED);
    if (above) 
      set_state(MOVEU_R);
    else if (!leftof) 
      set_state(MOVE_L);
    break;

  case MOVEU_R:
    time = (time + 1) % (3 * ANIM_SPEED);
    anim = int(time / ANIM_SPEED);
    if (rightof)
      set_state(MOVEU_L);
    else if (!above) {
      set_state(MOVE_LU);
      //set_state(FALLING);
      //toggle_look_right();
    }
    else {
      Scanresult crbelow;
      int midx = pos.x + BLUBBER_CENTRE_X;
      int bottomy = pos.y + BLUBBER_SIZE_H;

      aitools->scan_down(midx, bottomy, crbelow);
      int totalc = crbelow.count_creatures();
      Creature *tempc;
      for (; totalc > 0; totalc--) {
	tempc = aitools->identify_creature(crbelow.takeout_creature());
	if (tempc->team != this->team && not tempc->dead) {
	  set_state(FALLING);
	  break;
	}
      }
    }
    break;

  case MOVE_R:
    time = (time + 1) % (3 * ANIM_SPEED);
    anim = int(time / ANIM_SPEED);
    if (rightof) 
      set_state(MOVE_RU);
    else if (!below) {
      if (!aitools->px_pf_scan(pos.x + pos.w, pos.y + pos.h + 1) &&
	  !aitools->px_pf_scan(pos.x, pos.y + pos.h + 1)) {
      set_state(FALLING);
      //toggle_look_right();
      }
    }
    else
      time = 0;
    break;

  case MOVE_RU:
    time = (time + 1) % (3 * ANIM_SPEED);
    anim = int(time / ANIM_SPEED);
    if (above) 
      set_state(MOVEU_L);
    else if (!rightof)
      set_state(MOVE_R);
    break;

  case MOVEU_L:
    time = (time + 1) % (3 * ANIM_SPEED);
    anim = int(time / ANIM_SPEED);
    if (leftof)
      set_state(MOVEU_R);
    else if (!above) {
      set_state(MOVE_RU);
      //set_state(FALLING);
      //toggle_look_right();
    }
    else {
      Scanresult crbelow;
      int midx = pos.x + BLUBBER_CENTRE_X;
      int bottomy = pos.y + BLUBBER_SIZE_H;

      aitools->scan_down(midx, bottomy, crbelow);
      int totalc = crbelow.count_creatures();
      Creature *tempc;
      for (; totalc > 0; totalc--) {
	tempc = aitools->identify_creature(crbelow.takeout_creature());
	if (tempc->team != team && not tempc->dead) {
	  set_state(FALLING);
	  break;
	}
      }
    }
    break;
  }
}

void Blubber::cause_damage() {
  if (not aura){
    Fxfield **ny;
    ny = world->get_free_fxfield();
    aura = new Aura(pos.x, pos.y, pos.w, pos.h, this, world, team,
                    aurapower_x, aurapower_y, BLUBBER_AURA_DAMAGE);
    *ny = aura;
    world->register_fxfield(*ny);
  }
  else{
    aura->lifetime = 1;
  }
}
