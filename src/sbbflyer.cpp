#include "SDL/SDL.h"
#include "sbbflyer.h"
#include "graphics.h"
#include "settings.h"
#include "game.h"
#include "aura.h"
#include <sstream>

int Sbbflyer::obj_count = 0;
SDL_Surface *Sbbflyer::pic = NULL;

void Sbbflyer::teleported(){
  spawn_height = pos.y;
  aspeed_y = 0.6;
}

std::string Sbbflyer::generate_save_string(){
  stringstream ss;
  string ret, temp;
  ss << "sbbflyer ";
  ss << pos.x;
  ss << " ";
  ss << pos.y;
  ss << " ";
  ss << look_right;
  ss << " ";
  ss << team;
  ret =ss.str();
  return ret;
}

Sbbflyer::Sbbflyer( SDL_Surface **screen, Sint16 x, Sint16 y, bool ilook_right,
                int iteam, Map *iworld){
  if (obj_count == 0 && pic == NULL){
     pic = load_optimized_pic( "graphics/sbbflyer.bmp", screen,
                                       44, 255, 153 );
  }
  //creature specific settings
  team = iteam;
  hp = hpmax = 5;
  safe = 0;
  spawn_height = y;
  aspeed_y = 0.6;
  world = iworld;
  immune_to_gravity = 1;
  pos.x = x;
  pos.y = y;
  pos.w = 74;
  pos.h = 16;
  flaxx_speed = 10;
  time = flaxx_speed;
  state = FLYING;
  hit_pic_timer = 0;

  //sbbflyer specific settings
  obj_count ++;
  look_right = ilook_right;
  aurapower_x = 1;
  aurapower_y = 1;
  aura = NULL;
  goingdown = false;
  show_hit_pic = false;

  if (look_right){
    set_pic(SBBFLYERRIGHT0);
    curr_pic = 0;
  }else{
    set_pic(SBBFLYERLEFT0);
    curr_pic = 0;
  }
}

void Sbbflyer::think(){
  int k = spawn_height - pos.y;
  double speed = aspeed_x;

  switch(state){
  case FLYING:
    if(rightof){
      look_right = false;
      set_pic(SBBFLYERLEFT0);
      curr_pic = 0;
    }else if(leftof){
      look_right = true;
      set_pic(SBBFLYERRIGHT0);
      curr_pic = 0;
    }
    if(not look_right){ // Sett new x speed
      if(aspeed_x <= -1){ //If maxpeed reached, set speed to maxspeed
        aspeed_x = -1;
      }else{ //accelerate
        aspeed_x += -0.01;
      }
    }else{
      if(aspeed_x >= 1){ //If maxpeed reached, set speed to maxspeed
        aspeed_x = 1;
      }else{ //accelerate
        aspeed_x += 0.01;
      }
    }

    if(k <= -30 || below){ // Switching to going down
      immune_to_gravity = true;
      goingdown = false;
      if(aspeed_y <= 0.6){ //If maxpeed reached, set speed to maxspeed
        aspeed_y = -0.6;
      }else{ //accelerate
        aspeed_y += -0.01;
      }
    }else if(k >= 30 || above){ // Swithing to upwards movement
      immune_to_gravity = true;
      goingdown = true;
      if(aspeed_y >= 0.6){ //If maxpeed reached, set speed to maxspeed
        aspeed_y = 0.6;
      }else{ //accelerate
        aspeed_y += 0.01;
      }
    }
    //(goingdown?aspeed_y=0.6:aspeed_y=-0.6); // Sett new y speed

    // Animation code
    if(goingdown){ //glidflyg
      if(look_right){
        set_pic(SBBFLYERRIGHT1);
        curr_pic = 1;
      }else{
        set_pic(SBBFLYERLEFT1);
        curr_pic = 1;
      }
      if(time == 0){
        time = flaxx_speed;
      }
    }else{ //flaxxa
      if(time == 0){ //Dags att byta bild?
        curr_pic = (curr_pic + 1 ) % 4; //Beräkna nästa bild
        if(look_right){ //Sätt bild åt höger
          switch(curr_pic){
          case 0:
            set_pic(SBBFLYERRIGHT0);
            break;
          case 1:
            set_pic(SBBFLYERRIGHT1);
            break;
          case 2:
            set_pic(SBBFLYERRIGHT2);
            break;
          case 3:
            set_pic(SBBFLYERRIGHT1);
            break;
          }
        }else{ //Sätt bild åt vänster
          switch(curr_pic){
          case 0:
            set_pic(SBBFLYERLEFT0);
           break;
          case 1:
            set_pic(SBBFLYERLEFT1);
            break;
          case 2:
            set_pic(SBBFLYERLEFT2);
            break;
          case 3:
            set_pic(SBBFLYERLEFT1);
            break;
          }
        }
        time = flaxx_speed;
      }
    }
    time--; //Minska tiden med 1

    //Aura code
    if(aura == NULL){
      Fxfield **ny;
      ny = world->get_free_fxfield();
      aura = new Aura(pos.x, pos.y, pos.w, pos.h, this, world, team,
                    aurapower_x, aurapower_y, 1);
      *ny = aura;
      world->register_fxfield(*ny);
    }
    if (aura != NULL){
      aura->lifetime = 1;
    }
    break;
  case DYING:
    if(speed < 0 && below){ //LEFT
      set_pic(SBBFLYERLEFT1);
      aspeed_x = speed + 0.2;
      if (aspeed_x > 0){
        aspeed_x = 0;
      }
    }else if(speed > 0 && below){ //RIGHT
      set_pic(SBBFLYERRIGHT1);
      aspeed_x = speed - 0.2;
      if( aspeed_x < 0){
        aspeed_x = 0;
      }
    }else if(speed == 0 && below){ //DEAD
      immune_to_gravity = false;
      show_hit_pic = false;
      aspeed_x = 0;
      aura = NULL;
      dead = true;
    }
   break;
  case HURT:
    if(hit_pic_timer <= 0){
      show_hit_pic = false;
    }else{
      show_hit_pic = true;
      hit_pic_timer--;
    }
    if(safe == 0 && aspeed_x < 1 && aspeed_x > -1 ){
      state = FLYING;
      show_hit_pic = false;
    }else{
      if(aspeed_x < 0){
        if(below){
          aspeed_x += 0.3;
        }else{
          aspeed_x += 0.07;
        }
      }else{
        if(below){
          aspeed_x += -0.3;
        }else{
          aspeed_x += -0.07;
        }
      }
    }
    break;
  default:
    printf("SBBFLYER entered abnormal stage");
    break;
  }
}

void Sbbflyer::paint(SDL_Surface **screen){
  SDL_Rect source, target;
  if (not settings.collision_mode){
    source = source_pos;
    prepare_to_show(target, target_pos);
    if(SDL_BlitSurface(pic, &source, *screen,
                       &target)){
      printf("Sbbflyer:paint() Error while blitting player:%s",SDL_GetError());
      exit(1);
    }
  }
  else{
    prepare_to_show(target, pos);
    SDL_FillRect(*screen, &target, SDL_MapRGB((*screen)->format, 200, 120, 0));
  }
  if(show_hit_pic){
    paint_hit_pic(screen);
  }
}

void Sbbflyer::paint_hit_pic(SDL_Surface **screen){
  SDL_Rect source, target, pow;
  if (not settings.collision_mode){
    source.x = 15;
    source.y = 113;
    source.w = 46;
    source.h = 27;

    pow.x = pos.x - 2;
    pow.y = pos.y - 2;

    prepare_to_show(target, pow);
    if(SDL_BlitSurface(pic, &source, *screen,
                       &target)){
      printf("Sbbflyer:paint_hit_pic() Error while blitting pow:%s",SDL_GetError());
      exit(1);
    }
  }
  else{
    prepare_to_show(target, pos);
    SDL_FillRect(*screen, &target, SDL_MapRGB((*screen)->format, 200, 120, 0));
  }
}

Sbbflyer::~Sbbflyer(){
  obj_count --;
  if (obj_count == 0){
    SDL_FreeSurface( pic );
    pic = NULL;
  }
  return;
}

void Sbbflyer::hurt(Fxfield *fxf){
    immune_to_gravity = false;
    aura = NULL;
    hit_pic_timer = 15;
  if(hp <= 0){
    state = DYING;
  }else{
    safe = 10;
    state = HURT;
  }
}

void Sbbflyer::set_pic( int pic_number ){
  switch(pic_number){
  case SBBFLYERLEFT0:
    source_pos.x = 4;
    source_pos.y = 5;
    source_pos.w = 74;
    source_pos.h = 36;

    target_pos.x = pos.x;
    target_pos.y = pos.y - 19;
    break;
  case SBBFLYERLEFT1:
    source_pos.x = 91;
    source_pos.y = 14;
    source_pos.w = 76;
    source_pos.h = 30;

    target_pos.x = pos.x;
    target_pos.y = pos.y - 9;
    break;
  case SBBFLYERLEFT2:
    source_pos.x = 180;
    source_pos.y = 25;
    source_pos.w = 74;
    source_pos.h = 27;

    target_pos.x = pos.x;
    target_pos.y = pos.y;
    break;
  case SBBFLYERRIGHT0:
    source_pos.x = 180;
    source_pos.y = 56;
    source_pos.w = 75;
    source_pos.h = 36;

    target_pos.x = pos.x;
    target_pos.y = pos.y -19;
    break;
  case SBBFLYERRIGHT1:
    source_pos.x = 91;
    source_pos.y = 65;
    source_pos.w = 76;
    source_pos.h = 30;

    target_pos.x = pos.x;
    target_pos.y = pos.y - 9;
    break;
  case SBBFLYERRIGHT2:
    source_pos.x = 4;
    source_pos.y = 76;
    source_pos.w = 77;
    source_pos.h = 27;

    target_pos.x = pos.x;
    target_pos.y = pos.y;
    break;
  /*case SBBFLYERHITRIGHT:
    source_pos.x = 15;
    source_pos.y = 113;
    source_pos.w = 46;
    source_pos.h = 27;

    target_pos.x = pos.x;
    target_pos.y = pos.y;
    break;
  case SBBFLYERHITLEFT:
    source_pos.x = 15;
    source_pos.y = 113;
    source_pos.w = 46;
    source_pos.h = 27;

    target_pos.x = pos.x;
    target_pos.y = pos.y;
    break;
  case SBBFLYERDEADRIGHT:
    source_pos.x = 94;
    source_pos.y = 146;
    source_pos.w = 84;
    source_pos.h = 45;

    target_pos.x = pos.x - 17;
    target_pos.y = pos.y + 22;
    break;
  case SBBFLYERDEADLEFT:
    source_pos.x = 17;
    source_pos.y = 203;
    source_pos.w = 84;
    source_pos.h = 45;

    target_pos.x = pos.x - 17;
    target_pos.y = pos.y + 22;
    break;
*/
  default:
    printf("Sbbflyer set wrong pic\n");
    exit(1);
  }
  target_pos.w = source_pos.w;
  target_pos.h = source_pos.h;

}

void Sbbflyer::move_x(int x_diff){
  pos.x = pos.x + x_diff;
  target_pos.x = target_pos.x + x_diff;

  if (aura != NULL){
    world->unregister_fxfield(aura);
    aura->col_rect[0].x = pos.x;
    aura->col_rect[0].w = pos.w;
    world->register_fxfield(aura);
  }
}

void Sbbflyer::move_y(int y_diff){
  pos.y = pos.y + y_diff;
  target_pos.y = target_pos.y + y_diff;

  if (aura != NULL){
    world->unregister_fxfield(aura);
    aura->col_rect[0].y = pos.y;
    aura->col_rect[0].h = pos.h;
    world->register_fxfield(aura);
  }
}

void Sbbflyer::set_look_right(bool ilook_right){
  look_right = ilook_right;
  if (look_right){
    set_pic(SBBFLYERRIGHT0);
    curr_pic = 0;
  }else{
    set_pic(SBBFLYERLEFT0);
    curr_pic = 0;
  }
}

bool Sbbflyer::looking_right(){
  return look_right;
}
