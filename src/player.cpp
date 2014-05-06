#include "SDL/SDL.h"
#include "player.h"
#include "controller.h"
#include "graphics.h"
#include "settings.h"
#include "game.h"
#include "map.h"
#include "swordslash.h"
#include <sstream>
#include <string>
#include "aitools.h"

#define WSACTSPDX 1.5 /* wallslide activation speed x. minimum x speed to start
                         sliding
                      */
#define WSACTSPDY -4.5 // same but for y
#define WALLSLIDETIME 120

#define WALLLEAPX 6.0
#define WALLLEAPY 8.6
#define WSATKREC 25 //amount of frames to recover from att ws-attack

int Player::obj_count = 0;
SDL_Surface *Player::pic = NULL;

double abs( double num ){
  if (num < 0)
    return -num;
  else
    return num;
}

int Player::get_controllerid(){
  return controllerid;
}

int Player::get_skin(){
  return skinnum;
}

void Player::set_skin(int iskinnum){
  skinnum = iskinnum;
  switch (skinnum) {
  case 0:
    skinoffset = 0;
    break;
  case 1:
    skinoffset = 313;
    break;
  default:
    printf("Player::Player: Errorous skin %d\n", skinnum);
    exit(1);
  }

  if (look_right)
    set_pic(PP_RIGHT0);
  else
    set_pic(PP_LEFT0);
}

void Player::set_controllerid( int id ){
  controllerid = id;
  if (controllerid < 0)
    controllerid = 0;
  else if (controllerid >= TOT_CONTROLLERS)
    controllerid = TOT_CONTROLLERS -1;
}

bool Player::looking_right(){
  return look_right;
}

void Player::set_look_right( bool ilook_right ){
  look_right = ilook_right;
  if (look_right)
    set_pic(PP_RIGHT0);
  else
    set_pic(PP_LEFT0);
}

string Player::generate_save_string(){
  stringstream ss;
  string ret, temp;
  ss << "player ";
  ss << pos.x;
  ss << " ";
  ss << pos.y;
  ss << " ";
  ss << look_right;
  ss << " ";
  ss << team;
  ss << " ";
  ss << controllerid;
  ss << " ";
  ss << skinnum;
  ret = ss.str();
  return ret;
}


Player::~Player(){
  obj_count --;
  if (obj_count == 0){
    SDL_FreeSurface( pic );
    pic = NULL;
  }
  Swordslash::remove_spawner();
  delete input;
  return;
}

Player::Player( SDL_Surface **screen, Aitools *iaitools, Sint16 x, Sint16 y,
                Map *mapworld, int ilook_right, int iteam, int icontrollerid,
                int skin){
  controllerid = icontrollerid;
  input = new Controller(&settings.conkeymap[controllerid]);

  if (pic == NULL){
     pic = load_optimized_pic( "graphics/player.bmp", screen,
                                         44, 255, 153 );
  }

  set_skin(skin);
  Swordslash::add_spawner(screen);

  //creature specific settings
  hpmax = 15;
  hp = hpmax;
  immune_to_gravity = 0;
  team = iteam;
  world = mapworld;

  pos.x = x;
  pos.y = y;
  pos.w = 30;
  pos.h = 60;

  //initialize a joystick??

  //player specific settings
  aitools = iaitools;

  last_attacker = NULL;
  recovery = 0;
  obj_count ++;
  look_right = ilook_right;
  show_sword = false;
  swordpic_source.x = 0;
  swordpic_source.y = 0;
  swordpic_source.w = 0;
  swordpic_source.h = 0;
  swordpic_target.x = 0;
  swordpic_target.y = 0;
  swordpic_target.w = 0;
  swordpic_target.h = 0;
  currslash = NULL;
  upswing = NULL;
  downslashpower_x = 3;
  downslashpower_y = 2;
  downslashdmg = 5;
  downslash_stun = 15;
  downslash_stunbig = 30;
  slashpower_x = 4.5;
  slashpower_y = -2;
  slashdmg = 3;
  slashdmgtime = 1;
  upswingpower_x = 3;
  upswingpower_y = -3;
  upswingdmg = 3;
  upswingdmgtime = 4;
  uppercutpower_x = 1;
  uppercutpower_y = 1;
  uppercutdmg = 2;
  uppercutdmgtime = 4;
  uppercutdeftime = 15;
  uppercuttimeleft = 0;
  safe_default = 40;
  recovery_hurt = 15;
  runframe = 0;
  legspeed = 6;
  steps_until_picswitch = legspeed;
  stun = 0;
  slash_stun = 20;
  state = PS_CONTROLLED_AIRBOURNE;
  wallslidetimeleft = 0;
  wallslidetimedefault = WALLSLIDETIME;
  wallslidespeed = 0.5;
  walleappowerx = WALLLEAPX;
  walleappowery = WALLLEAPY;
  wsattrec = 0;
  wsattrecdef = WSATKREC;
  x_maxspeed = 8;
  x_ret = 0.7;
  x_acc = 0.7;
  x_airacc = 0.3;//0.16;
  x_airret = 0.3;
  land_penalty = 2;

  leap_power = 10;
  leap_breakin = 0.9;
  leap_breakingin = false;

  attacktimes = 0;

  if (not look_right)
    set_pic(PP_LEFT0);
  else
    set_pic(PP_RIGHT0);
}

void Player::think(){
  input->get_input();
  switch(state){
  case PS_WALLSLIDINGATTREC:
    wallslidetimeleft --;
    wsattrec --;
    printf("tl %d, atrec %d\n", wallslidetimeleft, wsattrec);
    if (below){
      aspeed_x = 0;
      state = PS_STANDING;
    }
    else if (wsattrec <= 0){
      state = PS_WALLSLIDING;
      if (look_right)
        set_pic(PP_LWALL);
      else
        set_pic(PP_RWALL);
    }
    else if (wallslidetimeleft <= 0 ||
        (not leftof && not rightof)
        )
      prepare_state( PS_CONTROLLED_AIRBOURNE );
    else{
      if (wallslidespeed < aspeed_y)
        aspeed_y = wallslidespeed;
    }
    break;
  case PS_WALLSLIDING:
    wallslidetimeleft --;
    if (below){
      aspeed_x = 0;
      state = PS_STANDING;
    }
    else if (input->b(BUTTON_JUMP,TIMING)){
      if (leftof)
        aspeed_x = walleappowerx;
      else
        aspeed_x = -walleappowerx;
      aspeed_y = -walleappowery;
      prepare_state(PS_CONTROLLED_AIRBOURNE);
    }
    else if (wallslidetimeleft <= 0 ||
        (not leftof && not rightof) ||
        (leftof && not (input->b(BUTTON_LEFT,DOWN))) ||
        (rightof && not (input->b(BUTTON_RIGHT,DOWN)))
         )
      prepare_state( PS_CONTROLLED_AIRBOURNE );
    else if (wallslidetimeleft > wsattrecdef &&
             input->b(BUTTON_ATTACK, TIMING)){
      prepare_state(PS_WALLSLIDINGATTREC);
    }
    else{
      if (wallslidespeed < aspeed_y)
        aspeed_y = wallslidespeed;
    }
    break;
  case PS_LANDED_RECOVERY:
    if (stun <= 0){
      stun = 0;
      if (below){
        if (aspeed_x != 0)
          state = PS_CONTROLLED_RUNNING;
        else
          state = PS_STANDING;
      }
      else{
        prepare_state(PS_CONTROLLED_AIRBOURNE);
      }
    }
    else
      stun --;

    if (below){
      if (aspeed_x > x_ret){
        aspeed_x -= x_ret;
      }
      else if (aspeed_x < -x_ret){
        aspeed_x += x_ret;
      }
      else
        aspeed_x = 0;
    }
    else{
      prepare_state( PS_CONTROLLED_AIRBOURNE );
      attacktimes = 0;
      if (look_right)
        set_pic(PP_FLYRIGHT);
      else
        set_pic(PP_FLYLEFT);
    }
    break;
  case PS_AIRBOURNE_DOWNSLASH:
    if (upswing)
      upswing->update_pos(pos.x, pos.y);
    if (currslash){
      currslash->lifetime = 1;
      currslash->update_pos(pos.x, pos.y);
    }
    if (below){
      state = PS_LANDED_RECOVERY;
      if (attacktimes == 1)
        stun = downslash_stun;
      else //attacktimes must be 2, thus a long recovery awaits
        stun = downslash_stunbig;
      if (look_right)
        set_pic(PP_DOWNSLASHRECOVERYRIGHT);
      else
        set_pic(PP_DOWNSLASHRECOVERYLEFT);
      attacktimes = 0;
    }
    break;
  case PS_AIRBOURNE_UPSWING:
    if (upswing)
      upswing->update_pos(pos.x, pos.y);
    if (currslash)
      currslash->update_pos(pos.x, pos.y);
    if (aspeed_y > 2)
      state = PS_CONTROLLED_AIRBOURNE;
    break;
  case PS_AIRBOURNE_UPPERCUT:
    uppercuttimeleft --;
    if (currslash){
      currslash->update_pos(pos.x, pos.y);
    }
    if (uppercuttimeleft == 0){
      state = PS_CONTROLLED_AIRBOURNE;
    }
    else if (aspeed_y > -2){
      if (not input->b(BUTTON_ATTACK, DOWN)){
        if (look_right)
          set_pic(PP_FLYRIGHT);
        else
          set_pic(PP_FLYLEFT);
        state = PS_CONTROLLED_AIRBOURNE;
      }
      else if (upswing == NULL){
        //upswing!
        prepare_state(PS_AIRBOURNE_UPSWING);
      }
    }
    break;
  case PS_STANDING:
    if (aspeed_x != 0){
      state = PS_CONTROLLED_RUNNING;
      break;
    }
    if (not below)
      prepare_state( PS_CONTROLLED_AIRBOURNE );
    else{

      if (look_right)
        set_pic(PP_RIGHT0);
      else
        set_pic(PP_LEFT0);

      if(input->b(BUTTON_MENU, DOWN))
        world->main_player = this;

      if(input->b(BUTTON_LEFT,DOWN) && not input->b(BUTTON_RIGHT, DOWN)){
        look_right = 0;
        aspeed_x = -x_acc;
        state = PS_CONTROLLED_RUNNING;
      }
      else if (input->b(BUTTON_RIGHT,DOWN) && not input->b(BUTTON_LEFT,DOWN)){
        look_right = 1;
        aspeed_x = x_acc;
        state = PS_CONTROLLED_RUNNING;
      }

      if (input->b(BUTTON_JUMP,TIMING)){
        if (input->b(BUTTON_DOWN,DOWN))
          //sit down!
          prepare_state(PS_SITTING);
        else{
          //leaped!
          attacktimes = 0;
          aspeed_y = -leap_power;
          leap_breakingin = false;
          if (input->b(BUTTON_ATTACK,TIMING) && currslash == NULL){
            //uppercut!!
            prepare_state(PS_AIRBOURNE_UPPERCUT);
          }
          else{
            //ordinary leap!
            prepare_state(PS_CONTROLLED_AIRBOURNE);
          }
        }
      }
      else if (input->b(BUTTON_ATTACK,TIMING) && currslash == NULL){
        //attack!!
        prepare_state(PS_RECOVERING_FROM_SLASH);
      } //fi input->attack
    }
    break;

  case PS_CONTROLLED_RUNNING:
    if (not below){
      prepare_state( PS_CONTROLLED_AIRBOURNE );
    }
    else{
      if (input->b(BUTTON_ATTACK,TIMING) && currslash == NULL){
        prepare_state(PS_RECOVERING_FROM_SLASH);
      }
      else{
        if (aspeed_x > 0){ //running right
          if (input->b(BUTTON_RIGHT,DOWN) && not input->b(BUTTON_LEFT,DOWN) &&
              aspeed_x <= x_maxspeed){
            steps_until_picswitch --;
            if (steps_until_picswitch == 0){
              steps_until_picswitch = legspeed;
              runframe = (runframe + 1) % 4;
              switch (runframe){
              case 0:
              case 2:
                set_pic(PP_RIGHT1);
                break;
              case 1:
                set_pic(PP_RIGHT2);
                break;
              case 3:
                set_pic(PP_RIGHT3);
                break;
              }
            }
            aspeed_x += x_acc;
            if(aspeed_x > x_maxspeed)
              aspeed_x = x_maxspeed;
        }
          else{
            set_pic(PP_BREAKRIGHT);
            aspeed_x -= x_ret;
            if (aspeed_x < 0){
              state = PS_STANDING;
              aspeed_x = 0;
            }
          }
        }
        else if (aspeed_x < 0){ //running left
          if (input->b(BUTTON_LEFT,DOWN) && not input->b(BUTTON_RIGHT,DOWN) &&
              aspeed_x >= -x_maxspeed){
            steps_until_picswitch --;
            if (steps_until_picswitch == 0){
              steps_until_picswitch = legspeed;
              runframe = (runframe + 1) % 4;
              switch (runframe){
              case 0:
              case 2:
                set_pic(PP_LEFT1);
                break;
              case 1:
                set_pic(PP_LEFT2);
                break;
              case 3:
                set_pic(PP_LEFT3);
                break;
              }
            }
            aspeed_x -= x_acc;
            if(aspeed_x < -x_maxspeed)
              aspeed_x = -x_maxspeed;
          }
          else{
            set_pic(PP_BREAKLEFT);
            aspeed_x += x_ret;
            if (aspeed_x > 0){
              state = PS_STANDING;
              aspeed_x = 0;
            }
          }
        }
        else{ // aspeed_x = 0
          state = PS_STANDING;
        }

        if (input->b(BUTTON_JUMP,TIMING)){
          //jumping while running, leaped!
          aspeed_y = -leap_power;
          leap_breakingin = false;

          if (input->b(BUTTON_ATTACK,TIMING) && currslash == NULL){
            prepare_state(PS_AIRBOURNE_UPPERCUT);
          }
          else{
            prepare_state(PS_CONTROLLED_AIRBOURNE);
          }
        }
      }
    }
    break;
  case PS_CONTROLLED_AIRBOURNE:
    /* if jump-button is not holded down, break in */
    if (not input->b(BUTTON_JUMP,DOWN) || leap_breakingin){
      leap_breakingin = true;
      if (aspeed_y < 0)
        aspeed_y *= leap_breakin;
    }

    if (below){
      attacktimes = 0;
      if (aspeed_x == 0)
        state = PS_STANDING;
      else{
        //landed!
        state = PS_CONTROLLED_RUNNING;
        if (aspeed_x > 0) {
          if (aspeed_x > land_penalty)
            aspeed_x = aspeed_x - land_penalty;
          else
            aspeed_x = 0;
        } else if (aspeed_x < 0) {
          if (aspeed_x < -land_penalty)
            aspeed_x = aspeed_x + land_penalty;
          else
            aspeed_x = 0;
        }
      }
    }
    if ((leftof || rightof) &&
        (lastaspeed_x > WSACTSPDX || lastaspeed_x < -WSACTSPDX) &&
        (lastaspeed_y > WSACTSPDY)
        )
      prepare_state( PS_WALLSLIDING );
    else{
      if (input->b(BUTTON_ATTACK,TIMING) && currslash == NULL &&
          attacktimes < 2 && aspeed_y > 2){
        //downslash!
        prepare_state( PS_AIRBOURNE_DOWNSLASH );
      }
      else if (input->b(BUTTON_ATTACK,TIMING) && currslash == NULL &&
               attacktimes ==0 && aspeed_y < -2){
        //uppercut!!
        prepare_state(PS_AIRBOURNE_UPPERCUT);
      }
      else if (aspeed_y < -2)
        if (look_right)
          set_pic(PP_FLYUPRIGHT);
        else
          set_pic(PP_FLYUPLEFT);
      else if (aspeed_y > 2)
        if (look_right)
          set_pic(PP_FLYDOWNRIGHT);
        else
          set_pic(PP_FLYDOWNLEFT);
      else
        if (look_right)
          set_pic(PP_FLYRIGHT);
        else
          set_pic(PP_FLYLEFT);

      if(aspeed_x > 0){ //flying right
        if (input->b(BUTTON_RIGHT,DOWN) && not input->b(BUTTON_LEFT,DOWN) &&
            aspeed_x <= x_maxspeed){
          aspeed_x += x_airacc;
          if(aspeed_x > x_maxspeed)
            aspeed_x = x_maxspeed;
        }
        else{
          aspeed_x -= x_airret;
          if (aspeed_x < 0){
            aspeed_x = 0;
          }
        }
      }
      else if(aspeed_x < 0){ //flying left
        if (input->b(BUTTON_LEFT,DOWN) && not input->b(BUTTON_RIGHT,DOWN) &&
            aspeed_x >= -x_maxspeed){
          aspeed_x -= x_airacc;
          if(aspeed_x < -x_maxspeed)
            aspeed_x = -x_maxspeed;
        }
        else{
          aspeed_x += x_airret;
          if (aspeed_x > 0){
            aspeed_x = 0;
          }
        }
      }
      else{ //just flying, niether left nor right
        if (input->b(BUTTON_LEFT,DOWN) && not input->b(BUTTON_RIGHT,DOWN)){
          aspeed_x = -x_airacc;
          look_right = 0;
        }
        else if (input->b(BUTTON_RIGHT,DOWN) &&not input->b(BUTTON_LEFT,DOWN)){
          aspeed_x = x_airacc;
          look_right = 1;
        }
      }
    }
    break;
  case PS_RECOVERING_FROM_SLASH:
    stun --;
    if (aspeed_x < 0){
      aspeed_x += x_ret;
      if (aspeed_x > 0)
        aspeed_x = 0;
    }
    else if (aspeed_x > 0){
      aspeed_x -= x_ret;
      if (aspeed_x < 0)
        aspeed_x = 0;
    }

    if (stun <= 0){
      if (aspeed_x == 0)
        state = PS_STANDING;
      else
        state = PS_CONTROLLED_RUNNING;
    }
    break;
  case PS_UNCONTROLLED_AIRBOURNE:
    if (below){
      attacktimes = 0;
      state = PS_UNCONTROLLED_SLIDING;
    }
    else{
      if (aspeed_y > 2)
        if (look_right)
          set_pic(PP_HURTDOWNRIGHT);
        else
          set_pic(PP_HURTDOWNLEFT);
      else if (aspeed_y < -2)
        if (look_right)
          set_pic(PP_HURTUPRIGHT);
        else
          set_pic(PP_HURTUPLEFT);
      else
        if (look_right)
          set_pic(PP_HURTRIGHT);
        else
          set_pic(PP_HURTLEFT);

      if(aspeed_x > 0){ //flying right
        aspeed_x -= x_airret;
        if (aspeed_x < 0){
          aspeed_x = 0;
        }
      }
      else if(aspeed_x < 0){ //flying left
        aspeed_x += x_airret;
        if (aspeed_x > 0){
          aspeed_x = 0;
        }
      }
    }
    break;
  case PS_UNCONTROLLED_SLIDING:
    if (not below){
      state = PS_UNCONTROLLED_AIRBOURNE;
    }
    else{
      if (aspeed_x == 0){
        recovery --;
        if (recovery == 0){
          if (hp <= 0){
            state = PS_DEAD;
            if (world->main_player == this)
              world->main_player = last_attacker;
          }
          else{
            look_right = not look_right;
            prepare_state(PS_SITTING);
            break;
          }
        }
      }

      if (look_right)
        set_pic(PP_HURTSLIDERIGHT);
      else
        set_pic(PP_HURTSLIDELEFT);
    }

    if(aspeed_x > 0){ //sliding right
      aspeed_x -= x_ret;
      if (aspeed_x < 0){
        aspeed_x = 0;
      }
    }
    else if(aspeed_x < 0){ //sliding left
      aspeed_x += x_ret;
      if (aspeed_x > 0){
        aspeed_x = 0;
      }
    }
    break;
  case PS_SITTING:
    if(input->b(BUTTON_UP, TIMING)){
      state = PS_STANDING;
    }

    if(aspeed_x > 0){ //sliding right
      aspeed_x -= x_ret;
      if (aspeed_x < 0){
        aspeed_x = 0;
      }
    }
    else if(aspeed_x < 0){ //sliding left
      aspeed_x += x_ret;
      if (aspeed_x > 0){
        aspeed_x = 0;
      }
    }
    break;
  case PS_DEAD:
    dead = 1;
    if (look_right)
      set_pic(PP_DEADRIGHT);
    else
      set_pic(PP_DEADLEFT);

    break;
  default:
    printf("Player is in a undefined state: %d\n", state);
    exit(1);
  }

  lastaspeed_x = aspeed_x;
  lastaspeed_y = aspeed_y;
}

void Player::hurt(Fxfield *fxf){
  if (currslash)
    currslash->lifetime = 0;
  if (upswing)
    upswing->lifetime = 0;

  if (not dead){
    last_attacker = fxf->owner;
    state = PS_UNCONTROLLED_AIRBOURNE;
    recovery = recovery_hurt;
    safe = safe_default;
    if (aspeed_x > 0){
      look_right = 1;
      set_pic(PP_HURTUPLEFT);
    }
    else{
      look_right = 0;
      set_pic(PP_HURTUPRIGHT);
    }
  }
}

void Player::paint(SDL_Surface **screen){
  SDL_Rect source, target;
  if (not settings.collision_mode){
    source = source_pos;
    prepare_to_show(target, target_pos);
    if(SDL_BlitSurface(pic, &source, *screen,
                       &target)){
      printf("Player:paint() Error while blitting player:%s",SDL_GetError());
      exit(1);
    }
    if (show_sword){
      source = swordpic_source;
      prepare_to_show(target, swordpic_target);
      if(SDL_BlitSurface(pic, &source, *screen,
                         &target)){
        printf("Player:paint() Error while blitting playersword:%s",
               SDL_GetError());
        exit(1);
      }
    }

  }
  else{
    prepare_to_show(target, pos);
    SDL_FillRect(*screen, &target,
                 SDL_MapRGB((*screen)->format,
                            255, 0, 0));
  }
  return;
}
void Player::set_swordpic( enum Playerswordpic pic_number ){
  switch(pic_number){
  case PSW_NONE:
    show_sword = 0;
    break;
  case PSW_LANDRECOVERYRIGHT:
    show_sword = 1;
    swordpic_source.x = 536;
    swordpic_source.y = 254 + skinoffset;
    swordpic_source.w = 31;
    swordpic_source.h = 20;

    swordpic_target.x = pos.x + pos.w + 12;
    swordpic_target.y = pos.y + 41;
    break;
  case PSW_LANDRECOVERYLEFT:
    show_sword = 1;
    swordpic_source.x = 581;
    swordpic_source.y = 255 + skinoffset;
    swordpic_source.w = 31;
    swordpic_source.h = 20;

    swordpic_target.x = pos.x - swordpic_source.w - 11;
    swordpic_target.y = pos.y + 41;
    break;


  case PSW_DOWNSLASHRIGHT:
    show_sword = 1;
    swordpic_source.x = 535;
    swordpic_source.y = 233 + skinoffset;
    swordpic_source.w = 31;
    swordpic_source.h = 20;

    swordpic_target.x = pos.x + pos.w + 14;
    swordpic_target.y = pos.y + 28;
    break;
  case PSW_DOWNSLASHLEFT:
    show_sword = 1;
    swordpic_source.x = 582;
    swordpic_source.y = 234 + skinoffset;
    swordpic_source.w = 31;
    swordpic_source.h = 20;

    swordpic_target.x = pos.x - swordpic_source.w - 13;
    swordpic_target.y = pos.y + 28;
    break;
  case PSW_UPSWINGRIGHT:
    show_sword = 1;
    swordpic_source.x = 536;
    swordpic_source.y = 213 + skinoffset;
    swordpic_source.w = 31;
    swordpic_source.h = 20;

    swordpic_target.x = pos.x - 5;
    swordpic_target.y = pos.y - 23;
    break;
  case PSW_UPSWINGLEFT:
    show_sword = 1;
    swordpic_source.x = 581;
    swordpic_source.y = 214 + skinoffset;
    swordpic_source.w = 31;
    swordpic_source.h = 20;

    swordpic_target.x = pos.x + 3;
    swordpic_target.y = pos.y - 23;
    break;
  case PSW_UPPERCUTRIGHT:
    show_sword = 1;
    swordpic_source.x = 536;
    swordpic_source.y = 191 + skinoffset;
    swordpic_source.w = 31;
    swordpic_source.h = 20;

    swordpic_target.x = pos.x + pos.w + 17;
    swordpic_target.y = pos.y + 6;
    break;
  case PSW_UPPERCUTLEFT:
    show_sword = 1;
    swordpic_source.x = 581;
    swordpic_source.y = 192 + skinoffset;
    swordpic_source.w = 31;
    swordpic_source.h = 20;

    swordpic_target.x = pos.x - swordpic_source.w - 16;
    swordpic_target.y = pos.y + 6;
    break;
  case PSW_RIGHT:
    show_sword = 1;
    swordpic_source.x = 488;
    swordpic_source.y = 196 + skinoffset;
    swordpic_source.w = 33;
    swordpic_source.h = 11;

    swordpic_target.x = pos.x - 20;
    swordpic_target.y = pos.y + 19;
    break;
  case PSW_LEFT:
    show_sword = 1;
    swordpic_source.x = 492;
    swordpic_source.y = 218 + skinoffset;
    swordpic_source.w = 33;
    swordpic_source.h = 11;

    swordpic_target.x = pos.x + 17;
    swordpic_target.y = pos.y + 19;
    break;
  case PSW_LWALL:
    show_sword = 1;
    swordpic_source.x = 536;
    swordpic_source.y = 213 + skinoffset;
    swordpic_source.w = 31;
    swordpic_source.h = 20;

    swordpic_target.x = pos.x - 17;
    swordpic_target.y = pos.y + 17;
    break;
  case PSW_RWALL:
    show_sword = 1;
    swordpic_source.x = 581;
    swordpic_source.y = 214 + skinoffset;
    swordpic_source.w = 31;
    swordpic_source.h = 20;

    swordpic_target.x = pos.x + 16;
    swordpic_target.y = pos.y + 17;
    break;
  default: printf("Player:set_swordpic Errorous swordpic number\n");
    exit(1);
    break;
  }
}


void Player::set_pic( enum Playerpic pic_number ){
  switch(pic_number){
  case PP_DOWNSLASHRECOVERYLEFT:
    source_pos.x = 717;
    source_pos.y = 250 + skinoffset;
    source_pos.w = 63;
    source_pos.h = 50;

    target_pos.x = pos.x - 19;
    target_pos.y = pos.y + 12;

    set_swordpic(PSW_LANDRECOVERYLEFT);
    break;
  case PP_DOWNSLASHRECOVERYRIGHT:
    source_pos.x = 654;
    source_pos.y = 91 + skinoffset;
    source_pos.w = 63;
    source_pos.h = 50;

    target_pos.x = pos.x - 14;
    target_pos.y = pos.y + 12;

    set_swordpic(PSW_LANDRECOVERYRIGHT);
    break;
  case PP_DOWNSLASHRIGHT:
    source_pos.x = 653;
    source_pos.y = 3 + skinoffset;
    source_pos.w = 68;
    source_pos.h = 67;

    target_pos.x = pos.x - 19;
    target_pos.y = pos.y - 4;

    set_swordpic(PSW_DOWNSLASHRIGHT);

    break;
  case PP_DOWNSLASHLEFT:
    source_pos.x = 713;
    source_pos.y = 162 + skinoffset;
    source_pos.w = 68;
    source_pos.h = 67;

    target_pos.x = pos.x - 19;
    target_pos.y = pos.y - 4;

    set_swordpic(PSW_DOWNSLASHLEFT);
    break;
  case PP_UPPERCUTRIGHT:
    source_pos.x = 575;
    source_pos.y = 3 + skinoffset;
    source_pos.w = 71;
    source_pos.h = 73;

    target_pos.x = pos.x - 18;
    target_pos.y = pos.y - 4;

    set_swordpic(PSW_UPPERCUTRIGHT);
    break;
  case PP_UPPERCUTLEFT:
    source_pos.x = 633;
    source_pos.y = 156 + skinoffset;
    source_pos.w = 71;
    source_pos.h = 73;

    target_pos.x = pos.x - 23;
    target_pos.y = pos.y - 4;

    set_swordpic(PSW_UPPERCUTLEFT);
    break;
  case PP_UPSWINGRIGHT:
    source_pos.x = 581;
    source_pos.y = 79 + skinoffset;
    source_pos.w = 47;
    source_pos.h = 77;

    target_pos.x = pos.x - 11;
    target_pos.y = pos.y - 10;

    set_swordpic(PSW_UPSWINGRIGHT);
    break;
  case PP_UPSWINGLEFT:
    source_pos.x = 651;
    source_pos.y = 232 + skinoffset;
    source_pos.w = 47;
    source_pos.h = 77;

    target_pos.x = pos.x - 6;
    target_pos.y = pos.y - 10;

    set_swordpic(PSW_UPSWINGLEFT);
    break;

  case PP_RIGHT1: // run right - legs together
    source_pos.x = 15;
    source_pos.y = 15 + skinoffset;
    source_pos.w = 33;
    source_pos.h = 62;

    target_pos.x = pos.x - 1;
    target_pos.y = pos.y - 2;

    set_swordpic(PSW_NONE);
    break;
  case PP_RIGHT0: // stand - look right
    source_pos.x = 67;
    source_pos.y = 9 + skinoffset;
    source_pos.w = 35;
    source_pos.h = 68;

    target_pos.x = pos.x - 2;
    target_pos.y = pos.y - 8;

    set_swordpic(PSW_NONE);
    break;
  case PP_RIGHT2: // run right - step 1
    source_pos.x = 119;
    source_pos.y = 15 + skinoffset;
    source_pos.w = 49;
    source_pos.h = 63;

    target_pos.x = pos.x - 10;
    target_pos.y = pos.y - 2;

    set_swordpic(PSW_NONE);
    break;
  case PP_RIGHT3: // run right - step 2
    source_pos.x = 179;
    source_pos.y = 17 + skinoffset;
    source_pos.w = 50;
    source_pos.h = 59;

    target_pos.x = pos.x - 9;
    target_pos.y = pos.y + 1;

    set_swordpic(PSW_NONE);
    break;
  case PP_LEFT3: // run left - step 2
    source_pos.x = 14;
    source_pos.y = 98 + skinoffset;
    source_pos.w = 50;
    source_pos.h = 59;

    target_pos.x = pos.x - 11;
    target_pos.y = pos.y + 1;

    set_swordpic(PSW_NONE);
    break;
  case PP_LEFT2: // run left step 1
    source_pos.x = 75;
    source_pos.y = 96 + skinoffset;
    source_pos.w = 49;
    source_pos.h = 63;

    target_pos.x = pos.x - 9;
    target_pos.y = pos.y - 2;

    set_swordpic(PSW_NONE);
    break;
  case PP_LEFT0: // stand - look left
    source_pos.x = 141;
    source_pos.y = 90 + skinoffset;
    source_pos.w = 35;
    source_pos.h = 68;

    target_pos.x = pos.x - 3;
    target_pos.y = pos.y - 8;

    set_swordpic(PSW_NONE);
    break;
  case PP_LEFT1: // run left - legs together
    source_pos.x = 195;
    source_pos.y = 96 + skinoffset;
    source_pos.w = 33;
    source_pos.h = 62;

    target_pos.x = pos.x - 2;
    target_pos.y = pos.y - 2;

    set_swordpic(PSW_NONE);
    break;

  case PP_FLYUPRIGHT: // flying up-right
    source_pos.x = 310;
    source_pos.y = 14 + skinoffset;
    source_pos.w = 44;
    source_pos.h = 69;

    target_pos.x = pos.x - 8;
    target_pos.y = pos.y - 3;

    set_swordpic(PSW_NONE);
    break;

  case PP_FLYDOWNRIGHT: // flying down-right
    source_pos.x = 373;
    source_pos.y = 16 + skinoffset;
    source_pos.w = 37;
    source_pos.h = 81;

    target_pos.x = pos.x - 7;
    target_pos.y = pos.y - 18;

    set_swordpic(PSW_NONE);
    break;

  case PP_FLYRIGHT: // flying right
    source_pos.x = 251;
    source_pos.y = 17 + skinoffset;
    source_pos.w = 50;
    source_pos.h = 68;

    target_pos.x = pos.x - 12;
    target_pos.y = pos.y - 2;

    set_swordpic(PSW_NONE);
    break;

  case PP_FLYUPLEFT: // flying up-left
    source_pos.x = 243;
    source_pos.y = 93 + skinoffset;
    source_pos.w = 44;
    source_pos.h = 69;

    target_pos.x = pos.x - 6;
    target_pos.y = pos.y - 3;

    set_swordpic(PSW_NONE);
    break;

  case PP_FLYLEFT: // flying left
    source_pos.x = 296;
    source_pos.y = 96 + skinoffset;
    source_pos.w = 50;
    source_pos.h = 68;

    target_pos.x = pos.x - 8;
    target_pos.y = pos.y - 3;

    set_swordpic(PSW_NONE);
    break;

  case PP_FLYDOWNLEFT: // flying down-left
    source_pos.x = 360;
    source_pos.y = 107 + skinoffset;
    source_pos.w = 37;
    source_pos.h = 81;

    target_pos.x = pos.x - 0;
    target_pos.y = pos.y - 18;

    set_swordpic(PSW_NONE);
    break;

  case PP_BREAKRIGHT: // breaking in right
    source_pos.x = 440;
    source_pos.y = 21 + skinoffset;
    source_pos.w = 43;
    source_pos.h = 65;

    target_pos.x = pos.x - 7;
    target_pos.y = pos.y - 2;

    set_swordpic(PSW_NONE);
    break;

  case PP_BREAKLEFT: // breaking in left
    source_pos.x = 420;
    source_pos.y = 122 + skinoffset;
    source_pos.w = 43;
    source_pos.h = 65;

    target_pos.x = pos.x - 6;
    target_pos.y = pos.y - 2;

    set_swordpic(PSW_NONE);
    break;

  case PP_ATTACKLEFT: // breaking in left
    source_pos.x = 488;
    source_pos.y = 119 + skinoffset;
    source_pos.w = 45;
    source_pos.h = 66;

    target_pos.x = pos.x - 3;
    target_pos.y = pos.y - 6;

    set_swordpic(PSW_LEFT);
    break;

  case PP_ATTACKRIGHT: // breaking in left
    source_pos.x = 507;
    source_pos.y = 18 + skinoffset;
    source_pos.w = 45;
    source_pos.h = 66;

    target_pos.x = pos.x - 12;
    target_pos.y = pos.y - 6;

    set_swordpic(PSW_RIGHT);
    break;

  case PP_HURTUPRIGHT:
    source_pos.x = 10;
    source_pos.y = 163 + skinoffset;
    source_pos.w = 50;
    source_pos.h = 71;

    target_pos.x = pos.x - 6;
    target_pos.y = pos.y - 3;

    set_swordpic(PSW_NONE);
    break;

  case PP_HURTUPLEFT:
    source_pos.x = 298;
    source_pos.y = 236 + skinoffset;
    source_pos.w = 50;
    source_pos.h = 71;

    target_pos.x = pos.x - 4;
    target_pos.y = pos.y - 3;

    set_swordpic(PSW_NONE);
    break;

  case PP_HURTLEFT:
    source_pos.x = 221;
    source_pos.y = 238 + skinoffset;
    source_pos.w = 70;
    source_pos.h = 55;

    target_pos.x = pos.x - 5;
    target_pos.y = pos.y + 18;

    set_swordpic(PSW_NONE);
    break;

  case PP_HURTRIGHT:
    source_pos.x = 67;
    source_pos.y = 165 + skinoffset;
    source_pos.w = 70;
    source_pos.h = 55;

    target_pos.x = pos.x - 34;
    target_pos.y = pos.y + 18;

    set_swordpic(PSW_NONE);
    break;

  case PP_HURTDOWNLEFT:
    source_pos.x = 157;
    source_pos.y = 239 + skinoffset;
    source_pos.w = 57;
    source_pos.h = 65;

    target_pos.x = pos.x - 2;
    target_pos.y = pos.y - 6;

    set_swordpic(PSW_NONE);
    break;

  case PP_HURTDOWNRIGHT:
    source_pos.x = 145;
    source_pos.y = 166 + skinoffset;
    source_pos.w = 57;
    source_pos.h = 65;

    target_pos.x = pos.x - 22;
    target_pos.y = pos.y - 6;

    set_swordpic(PSW_NONE);
    break;

  case PP_HURTSLIDERIGHT:
    source_pos.x = 208;
    source_pos.y = 176 + skinoffset;
    source_pos.w = 73;
    source_pos.h = 38;

    target_pos.x = pos.x - 24;
    target_pos.y = pos.y + 22;

    set_swordpic(PSW_NONE);
    break;

  case PP_HURTSLIDELEFT:
    source_pos.x = 77;
    source_pos.y = 249 + skinoffset;
    source_pos.w = 73;
    source_pos.h = 38;

    target_pos.x = pos.x - 19;
    target_pos.y = pos.y + 22;

    set_swordpic(PSW_NONE);
    break;

  case PP_SITRIGHT:
    source_pos.x = 5;
    source_pos.y = 247 + skinoffset;
    source_pos.w = 69;
    source_pos.h = 52;

    target_pos.x = pos.x - 13;
    target_pos.y = pos.y + 9;

    set_swordpic(PSW_NONE);
    break;

  case PP_SITLEFT:
    source_pos.x = 284;
    source_pos.y = 174 + skinoffset;
    source_pos.w = 69;
    source_pos.h = 52;

    target_pos.x = pos.x - 26;
    target_pos.y = pos.y + 9;

    set_swordpic(PSW_NONE);
    break;

  case PP_DEADRIGHT:
    source_pos.x = 367;
    source_pos.y = 202 + skinoffset;
    source_pos.w = 70;
    source_pos.h = 35;

    target_pos.x = pos.x - 20;
    target_pos.y = pos.y + 29;

    set_swordpic(PSW_NONE);
    break;

  case PP_DEADLEFT:
    source_pos.x = 366;
    source_pos.y = 251 + skinoffset;
    source_pos.w = 70;
    source_pos.h = 35;

    target_pos.x = pos.x - 20;
    target_pos.y = pos.y + 29;

    set_swordpic(PSW_NONE);
    break;
  case PP_LWALL:
    source_pos.x = 796;
    source_pos.y = 10 + skinoffset;
    source_pos.w = 42;
    source_pos.h = 76;

    target_pos.x = pos.x - 4;
    target_pos.y = pos.y - 10;

    set_swordpic(PSW_NONE);
    break;
  case PP_RWALL:
    source_pos.x = 908;
    source_pos.y = 95 + skinoffset;
    source_pos.w = 42;
    source_pos.h = 76;

    target_pos.x = pos.x - 8;
    target_pos.y = pos.y - 10;

    set_swordpic(PSW_NONE);
    break;
  case PP_LWALLATK:
    source_pos.x = 910;
    source_pos.y = 10 + skinoffset;
    source_pos.w = 39;
    source_pos.h = 76;

    target_pos.x = pos.x - 4;
    target_pos.y = pos.y - 10;

    set_swordpic(PSW_LWALL);
    break;
  case PP_RWALLATK:
    source_pos.x = 797;
    source_pos.y = 95 + skinoffset;
    source_pos.w = 39;
    source_pos.h = 76;

    target_pos.x = pos.x - 5;
    target_pos.y = pos.y - 10;

    set_swordpic(PSW_RWALL);
    break;
  default:
    printf("Player set wrong pic\n");
    exit(1);
  }
  target_pos.w = source_pos.w;
  target_pos.h = source_pos.h;
}

void Player::lost_slash(Swordslash *ss){
  if (ss == currslash)
    currslash = NULL;
  else if (ss == upswing)
    upswing = NULL;
  else{
    printf("Player::lost_slash() player lost ss %p, but he didn't have it!\n",
           ss);
    exit(1);
  }
}

void Player::prepare_state(Playerstate newstate){
  //set_state could this function also be called
  int oldstate = state;
  switch (newstate){
  case PS_WALLSLIDINGATTREC:
    wsattrec = wsattrecdef;
    if (leftof){
      look_right = true;
      set_pic(PP_LWALLATK);
    }
    else{
      look_right = false;
      set_pic(PP_RWALLATK);
    }
    break;
  case PS_WALLSLIDING:
    wallslidetimeleft = wallslidetimedefault;
    leap_breakingin = false;
    if (leftof){
      look_right = true;
      set_pic(PP_LWALL);
    }
    else{
      look_right = false;
      set_pic(PP_RWALL);
    }

    break;
  case PS_SITTING:
    if (look_right)
      set_pic(PP_SITRIGHT);
    else
      set_pic(PP_SITLEFT);

    //change hitbox?
    break;
  case PS_RECOVERING_FROM_SLASH:
    if (look_right)
      set_pic(PP_ATTACKRIGHT);
    else
      set_pic(PP_ATTACKLEFT);

    stun = slash_stun;
    Fxfield **attack;
    attack = world->get_free_fxfield();
    if (attack == NULL){
      printf("failed to create Swordslash\n");
      exit(1);
    }
    if (look_right){
      set_pic(PP_ATTACKRIGHT);
      *attack = new Swordslash(pos.x, pos.y, PSW_RIGHT, slashdmgtime,
                               world, team, slashpower_x, slashpower_y,
                               slashdmg, this);
    }
    else{
      set_pic(PP_ATTACKLEFT);
      *attack = new Swordslash(pos.x, pos.y, PSW_LEFT, slashdmgtime, world,
                               team, -slashpower_x, slashpower_y,slashdmg,
                               this);
    }
    currslash = dynamic_cast<Swordslash*>(*attack);
    world->register_fxfield(*attack);
    break;
  case PS_AIRBOURNE_DOWNSLASH:
    attacktimes ++;
    attack = world->get_free_fxfield();
    if (attack == NULL){
      printf("failed to create Uppercut\n");
      exit(1);
    }
    if (look_right){
      set_pic(PP_DOWNSLASHRIGHT);
      *attack = new Swordslash(pos.x, pos.y, PSW_DOWNSLASHRIGHT, 1, world,
                               team, aspeed_x + downslashpower_x,
                               aspeed_y + downslashpower_y,
                               downslashdmg, this);
    }
    else{
      set_pic(PP_DOWNSLASHLEFT);
      *attack = new Swordslash(pos.x, pos.y, PSW_DOWNSLASHLEFT, 1, world,
                               team, aspeed_x - downslashpower_x,
                               aspeed_y + downslashpower_y,
                               downslashdmg, this);
    }
    world->register_fxfield(*attack);

    currslash = dynamic_cast<Swordslash*>(*attack);
    break;
  case PS_AIRBOURNE_UPSWING:
    attacktimes ++;

    attack = world->get_free_fxfield();
    if (attack == NULL){
      printf("failed to create Uppercut\n");
      exit(1);
    }
    if (look_right){
      set_pic(PP_UPSWINGRIGHT);
      *attack = new Swordslash(pos.x, pos.y,
                               PSW_UPSWINGRIGHT,
                               upswingdmgtime, world,
                               team, aspeed_x + upswingpower_x,
                               aspeed_y + upswingpower_y,
                               upswingdmg, this);
    }
    else{
      set_pic(PP_UPSWINGLEFT);
      *attack = new Swordslash(pos.x, pos.y, PSW_UPSWINGLEFT,
                               upswingdmgtime, world,
                               team, aspeed_x - upswingpower_x,
                               aspeed_y + upswingpower_y,
                               upswingdmg, this);
    }
    world->register_fxfield(*attack);

    upswing = dynamic_cast<Swordslash*>(*attack);
    break;
  case PS_CONTROLLED_AIRBOURNE:
    attacktimes = 0;
    break;
  case PS_AIRBOURNE_UPPERCUT:
    //uppercut!
    if (oldstate == PS_STANDING || oldstate == PS_CONTROLLED_RUNNING){
      //leaps!
      aspeed_y = -leap_power;
      leap_breakingin = false;
      attacktimes = 0;
    }
    attacktimes ++;

    uppercuttimeleft = uppercutdeftime;

    attack = world->get_free_fxfield();
    if (attack == NULL){
      printf("failed to create Uppercut\n");
      exit(1);
    }
    if (look_right){
      set_pic(PP_UPPERCUTRIGHT);
      *attack = new Swordslash(pos.x, pos.y,
                               PSW_UPPERCUTRIGHT,
                               uppercutdmgtime, world,
                               team, aspeed_x * uppercutpower_x,
                               aspeed_y * uppercutpower_y, uppercutdmg, this);
    }
    else{
      set_pic(PP_UPPERCUTLEFT);
      *attack = new Swordslash(pos.x, pos.y, PSW_UPPERCUTLEFT,
                               uppercutdmgtime, world,
                               team, aspeed_x * uppercutpower_x,
                               aspeed_y * uppercutpower_y, uppercutdmg, this);
    }
    world->register_fxfield(*attack);

    currslash = dynamic_cast<Swordslash*>(*attack);
    break;
  default:
    printf("Player::prepare_state() unhandeled state %d old state %d\n",
           newstate, oldstate);
    exit(1);
  }

  state = newstate;
}

void Player::move_x(int x_diff){
  pos.x = pos.x + x_diff;
  target_pos.x = target_pos.x + x_diff;
  if (show_sword)
    swordpic_target.x = swordpic_target.x + x_diff;
}

void Player::move_y(int y_diff){
  pos.y = pos.y + y_diff;
  target_pos.y = target_pos.y + y_diff;
  if (show_sword)
    swordpic_target.y = swordpic_target.y + y_diff;
}
