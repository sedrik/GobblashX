#include "SDL/SDL.h"
#include "pomgob.h"
#include "graphics.h"
#include "settings.h"
#include "game.h"
#include <sstream>

int Pomgob::obj_count = 0;
SDL_Surface *Pomgob::pic = NULL;

std::string Pomgob::generate_save_string(){
  stringstream ss;
  string ret, temp;
  ss << "pomgob ";
  ss << pos.x;
  ss << " ";
  ss << pos.y;
  ss << " ";
  ss << look_right;
  ss << " ";
  ss << standing_still;
  ss << " ";
  ss << team;
  ret =ss.str();
  return ret;
}

bool Pomgob::get_stand_still(){
  return standing_still;
}

void Pomgob::set_stand_still(bool istand_still){
  standing_still = istand_still;
}

bool Pomgob::looking_right(){
  return look_right;
}
void Pomgob::set_look_right(bool ilook_right){
  look_right = ilook_right;
  if (look_right)
    set_pic(POMGOBRIGHT1);
  else
    set_pic(POMGOBLEFT1);
}

Pomgob::Pomgob( SDL_Surface **screen, Sint16 x, Sint16 y, bool ilook_right,
                bool istanding_still, int iteam, Map *iworld){
  if (obj_count == 0 && pic == NULL){
     pic = load_optimized_pic( "graphics/pomgob.bmp", screen,
                                       44, 255, 153 );
  }
  //creature specific settings
  team = iteam;
  hpmax = 9;
  hp = hpmax;
  safe = 0;
  world = iworld;
  immune_to_gravity = 0;
  pos.x = x;
  pos.y = y;
  pos.w = 55;
  pos.h = 57;

  //pomgob specific settings
  obj_count ++;
  look_right = ilook_right;
  runframe = 1;
  leaps_default = 3;
  if (istanding_still)
    leaps_left = leaps_default;
  else
    leaps_left = 0;
  legspeed = 6;
  recovery = 0;
  standing_still = istanding_still;
  steps_until_picswitch = legspeed;
  hitpower_x = 7;
  hitpower_y = 1;
  jumpwait = 10;
  last_aspeed_y = 0;
  x_maxspeed = 2.2;
  x_ret = 0.5;
  x_acc = 0.15;
  x_airacc = 0.0;
  aura = NULL;

  if (look_right)
    set_pic(POMGOBRIGHT1);
  else
    set_pic(POMGOBLEFT1);
}

void Pomgob::think(){
  if (recovery > 0){
    if (below){
      aspeed_y = -last_aspeed_y * 0.75;
      recovery --;
    }
    last_aspeed_y = aspeed_y;
    if (aspeed_x > 0){
      if (aspeed_x > x_airacc)
        aspeed_x = aspeed_x - x_airacc;
      else
        aspeed_x = 0;
    }
    else if (aspeed_x < 0){
      if (aspeed_x < -x_airacc)
        aspeed_x = aspeed_x + x_airacc;
      else
        aspeed_x = 0;
    }
    if (recovery == 0 && hp <= 0) {
      dead = 1;
      aspeed_x = 0;
      aura = NULL;
      if (look_right)
        set_pic(POMGOBDEADRIGHT);
      else
        set_pic(POMGOBDEADLEFT);
      }
  }
  else{ // recovery == 0
    if (aura == NULL){
      Fxfield **ny;
      if (world == NULL){
        printf("Pomgob: world == NULL\n");
        exit(1);
      }
      ny = world->get_free_fxfield();
      aura = new Aura(pos.x, pos.y, pos.w, pos.h, this, world, team,
                    hitpower_x, hitpower_y, 1);
      *ny = aura;
      world->register_fxfield(*ny);
    }

    if (not standing_still){
      if (look_right && rightof)
        look_right = 0;
      else if (not look_right && leftof)
        look_right = 1;
      aspeed_x = aspeed_x + x_acc * (look_right? 1: -1);
      if (aspeed_x > x_maxspeed)
        aspeed_x = x_maxspeed;
      if (aspeed_x < -x_maxspeed)
        aspeed_x = -x_maxspeed;

      if (steps_until_picswitch == 0){
        switch(runframe){
        case 0:
        case 2:
          if (look_right)
            set_pic(POMGOBRIGHT1);
          else
            set_pic(POMGOBLEFT1);
          break;
        case 1:
          if (look_right)
            set_pic(POMGOBRIGHT0);
          else
            set_pic(POMGOBLEFT0);
          break;
        case 3:
          if (look_right)
            set_pic(POMGOBRIGHT2);
          else
            set_pic(POMGOBLEFT2);
          break;
        default:
          printf("Errorous runframe on Pomgob\n");
          exit(1);
        }
        runframe = (runframe + 1) % 4;
      }
      steps_until_picswitch = (steps_until_picswitch == 0? legspeed :
                               steps_until_picswitch -1);

    } //fi not standing_still
    else{ //standing still
      if (aspeed_x > x_airacc)
        aspeed_x = aspeed_x - x_airacc;
      else if (aspeed_x < -x_airacc)
        aspeed_x = aspeed_x + x_airacc;
      else
        aspeed_x = 0;

      if (look_right)
        set_pic(POMGOBRIGHT1);
      else
        set_pic(POMGOBLEFT1);
      if (below){
        jumpwait --;
        if (jumpwait <= 0){
          jumpwait = 100;
          aspeed_y -= 7;
          leaps_left --;
          if (leaps_left == 0)
            standing_still = false;
        }
      }
    }
  }
  if (aura != NULL){
      aura->lifetime = 1;
  }
}

void Pomgob::paint(SDL_Surface **screen){
  SDL_Rect source, target;
  if (not settings.collision_mode){
    source = source_pos;
    prepare_to_show(target, target_pos);
    if(SDL_BlitSurface(pic, &source, *screen,
                       &target)){
      printf("Pomgob:paint() Error while blitting player:%s",SDL_GetError());
      exit(1);
    }
  }
  else{
    prepare_to_show(target, pos);
    SDL_FillRect(*screen, &target,
                 SDL_MapRGB((*screen)->format,
                            200, 120, 0));
  }
  return;
}

Pomgob::~Pomgob(){
  obj_count --;
  if (obj_count == 0){
    SDL_FreeSurface( pic );
    pic = NULL;
  }
  return;
}

void Pomgob::hurt(Fxfield *fxf){
  if (not dead){
    standing_still = not standing_still;
    if (standing_still)
      leaps_left = leaps_default;
    else
      leaps_left = 0;
    if (aspeed_x > 0){
      set_pic(POMGOBHITRIGHT);
      look_right = 1;
    }
    else{
      set_pic(POMGOBHITLEFT);
      look_right = 0;
    }

    last_aspeed_y = aspeed_y;
    safe = 20;
    recovery = 25;
  }
}

void Pomgob::set_pic( int pic_number ){
  switch(pic_number){
  case POMGOBLEFT0:
    source_pos.x = 2;
    source_pos.y = 3;
    source_pos.w = 83;
    source_pos.h = 60;

    target_pos.x = pos.x - 16;
    target_pos.y = pos.y;
    break;
  case POMGOBLEFT1:
    source_pos.x = 90;
    source_pos.y = 4;
    source_pos.w = 83;
    source_pos.h = 57;

    target_pos.x = pos.x - 16;
    target_pos.y = pos.y;
    break;
  case POMGOBLEFT2:
    source_pos.x = 179;
    source_pos.y = 5;
    source_pos.w = 83;
    source_pos.h = 56;

    target_pos.x = pos.x - 16;
    target_pos.y = pos.y;
    break;
  case POMGOBRIGHT0:
    source_pos.x = 5;
    source_pos.y = 72;
    source_pos.w = 83;
    source_pos.h = 56;

    target_pos.x = pos.x - 12;
    target_pos.y = pos.y;
    break;
  case POMGOBRIGHT1:
    source_pos.x = 94;
    source_pos.y = 71;
    source_pos.w = 83;
    source_pos.h = 57;

    target_pos.x = pos.x - 12;
    target_pos.y = pos.y;
    break;
  case POMGOBRIGHT2:
    source_pos.x = 182;
    source_pos.y = 70;
    source_pos.w = 83;
    source_pos.h = 60;

    target_pos.x = pos.x - 12;
    target_pos.y = pos.y;
    break;

  case POMGOBHITRIGHT:
    source_pos.x = 9;
    source_pos.y = 139;
    source_pos.w = 72;
    source_pos.h = 51;

    target_pos.x = pos.x - 20;
    target_pos.y = pos.y + 7;
    break;
  case POMGOBHITLEFT:
    source_pos.x = 114;
    source_pos.y = 196;
    source_pos.w = 72;
    source_pos.h = 51;

    target_pos.x = pos.x;
    target_pos.y = pos.y + 7;
    break;

  case POMGOBDEADRIGHT:
    source_pos.x = 94;
    source_pos.y = 146;
    source_pos.w = 84;
    source_pos.h = 45;

    target_pos.x = pos.x - 17;
    target_pos.y = pos.y + 22;
    break;
  case POMGOBDEADLEFT:
    source_pos.x = 17;
    source_pos.y = 203;
    source_pos.w = 84;
    source_pos.h = 45;

    target_pos.x = pos.x - 17;
    target_pos.y = pos.y + 22;
    break;

  default:
    printf("Pomgob set wrong pic\n");
    exit(1);
  }
  target_pos.w = source_pos.w;
  target_pos.h = source_pos.h;

}

void Pomgob::move_x(int x_diff){
  pos.x = pos.x + x_diff;
  target_pos.x = target_pos.x + x_diff;

  if (aura != NULL){
    world->unregister_fxfield(aura);
    aura->col_rect[0].x = pos.x;
    aura->col_rect[0].w = pos.w;
    world->register_fxfield(aura);
  }
}

void Pomgob::move_y(int y_diff){
  pos.y = pos.y + y_diff;
  target_pos.y = target_pos.y + y_diff;

  if (aura != NULL){
    world->unregister_fxfield(aura);
    aura->col_rect[0].y = pos.y;
    aura->col_rect[0].h = pos.h;
    world->register_fxfield(aura);
  }
}
