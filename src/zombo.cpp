#include "zombo.h"
#include "SDL/SDL.h"
#include "settings.h"
#include "graphics.h"
#include "game.h"
#include <sstream>
#include "invizdmgfield.h"

/* ZOMBO STATS: */
#define ZOMBO_LEGSPEED_DEFAULT 12 /* amount of frames it takes for him to
                                     switch picture while walking */
#define ZOMBO_PATROL_SPEED 1 /* his speed when he patrols */
#define ZOMBO_DIE_ANIM_SPEED 6 /* (times 4) how many frames it takes for him
                                  to die when his hp reaches 0.*/
#define ZOMBO_RECOVERY_ANIM_SPEED 4 /* (times 5) amount of frames it takes
                                        for his to retaliate after he got
                                        struck */
#define ZOMBO_HP_MAX 7 /* his hp */
#define ZOMBO_AURA_POWER_X 5 /* knockpower x of his aura */
#define ZOMBO_AURA_POWER_Y 3 /* knockpower y of his aura */
#define ZOMBO_AURA_DAMAGE 2 /* damage of his aura */
#define ZOMBO_ATTACK_DAMAGE 5 /* the damage of his stab */
#define ZOMBO_SCAN_DISTANCE 130 /* how many pixels he can scan forward */
#define ZOMBO_ATTACK_ANIM_SPEED 4 /* (times 2) amount of frames it takes for
                                      him to charge his attack */
#define ZOMBO_ATTACK_THRUST_SPEED 8 /* the speed he gets when he attacks */
#define ZOMBO_ATTACK_THRUST_TIME 100 /* amount of frames he attacks */
#define ZOMBO_ATTACK_WIDTH 37 /* the width of his invizdmgfield */
#define ZOMBO_ATTACK_HEIGHT 10 /* the height of his invizdmgfield */
#define ZOMBO_ATTACK_Y 14 /* offset y of were his invizdmgfield spawns when he
                             attacks */
#define ZOMBO_EXHAUSTED_TIME 30 /* amount of frames zombo will wait after he
                                   attacked */

int Zombo::obj_count = 0;
SDL_Surface *Zombo::pic = NULL;

double _abs(double num){
  if (num > 0)
    return num;
  else
    return -num;
}

bool Zombo::looking_right(){
  return look_right;
}

void Zombo::set_look_right(bool ilook_right){
  look_right = ilook_right;
  if (look_right)
    set_pic(ZOMBO_R_WALK1);
  else
    set_pic(ZOMBO_L_WALK1);

}

void Zombo::hurt(Fxfield *fxf){
  if (not dead){
    aspeed_y = 0;
    if (hp <= 0)
      set_state(ZOMBO_STATE_DYING);
    else{
      if (aspeed_x < 0)
        look_right = true;
      if (aspeed_x > 0)
        look_right = false;
      set_state(ZOMBO_STATE_RECOVERING);
    }
  }
}
Zombo::Zombo(SDL_Surface ** screen, Sint16 x, Sint16 y, bool ilook_right,
             int iteam, Map *iworld, Aitools *iaitools){
  if (obj_count == 0 && pic == NULL){
    pic = load_optimized_pic( "graphics/zombo.bmp", screen,
                              0, 255, 128 );
  }
  //creature specific settings
  team = iteam;
  hpmax = ZOMBO_HP_MAX;
  hp = hpmax;
  safe = 0;
  world = iworld;
  immune_to_gravity = false;
  pos.x = x;
  pos.y = y;
  pos.w = 19;
  pos.h = 51;

  //Zombo specific settings
  aitools = iaitools;
  obj_count ++;
  look_right = ilook_right;
  aura = NULL;
  dmgfield = NULL;
  aurapower_x = ZOMBO_AURA_POWER_X;
  aurapower_y = ZOMBO_AURA_POWER_Y;
  auradamage = ZOMBO_AURA_DAMAGE;
  attackdamage = ZOMBO_ATTACK_DAMAGE;
  if (look_right)
    set_pic(ZOMBO_R_WALK2);
  else
    set_pic(ZOMBO_L_WALK2);
  frames_until_picswitch = ZOMBO_LEGSPEED_DEFAULT;
  animframe = 0;
  state = ZOMBO_STATE_PATROLLING;
  patrolspeed = ZOMBO_PATROL_SPEED;
}

Zombo::~Zombo(){
  obj_count --;
  if (obj_count == 0){
    SDL_FreeSurface( pic );
    pic = NULL;
  }
  return;
}

void Zombo::set_state(Zombostate newstate){
  switch(newstate){
  case ZOMBO_STATE_EXHAUSTED:
    dmgfield = NULL;
    frames_until_picswitch = ZOMBO_EXHAUSTED_TIME;
    if (look_right)
      set_pic(ZOMBO_R_WALK2);
    else
      set_pic(ZOMBO_L_WALK2);
    break;
  case ZOMBO_STATE_RECOVERING:
    animframe = 0;
    frames_until_picswitch = ZOMBO_RECOVERY_ANIM_SPEED;
    safe = ZOMBO_RECOVERY_ANIM_SPEED * 5;
    aura = NULL;
    if (look_right)
      set_pic(ZOMBO_R_HURT1);
    else
      set_pic(ZOMBO_L_HURT1);
    break;
  case ZOMBO_STATE_DYING:
    aura = NULL;
    safe = ZOMBO_DIE_ANIM_SPEED * 5;
    aspeed_x = 0;
    aspeed_y = 0;
    animframe = 0;
    frames_until_picswitch = ZOMBO_DIE_ANIM_SPEED;
    if (look_right)
      set_pic(ZOMBO_R_DYING1);
    else
      set_pic(ZOMBO_L_DYING1);
    break;
  case ZOMBO_STATE_DEAD:
    dead = true;
    if (look_right)
      set_pic(ZOMBO_R_DEAD);
    else
      set_pic(ZOMBO_L_DEAD);
    break;
  case ZOMBO_STATE_PATROLLING:
    if (look_right)
      set_pic(ZOMBO_R_WALK2);
    else
      set_pic(ZOMBO_L_WALK2);
    frames_until_picswitch = ZOMBO_LEGSPEED_DEFAULT;
    animframe = 0;
    break;
  case ZOMBO_STATE_ATTACKING:
    Fxfield **ny;

    frames_until_picswitch = ZOMBO_ATTACK_THRUST_TIME;
    if (look_right){
      set_pic(ZOMBO_R_ATK3);
      aspeed_x = ZOMBO_ATTACK_THRUST_SPEED;
    }
    else{
      set_pic(ZOMBO_L_ATK3);
      aspeed_x = -ZOMBO_ATTACK_THRUST_SPEED;
    }

    ny = world->get_free_fxfield();
    if (look_right){
      dmgfield = new Invizdmgfield(pos.x + pos.w, pos.y + ZOMBO_ATTACK_Y,
                                   ZOMBO_ATTACK_WIDTH, ZOMBO_ATTACK_HEIGHT,
                                   ZOMBO_ATTACK_Y, 1, world, team, aspeed_x,
                                   aspeed_y, attackdamage, this);
    }
    else{
      dmgfield = new Invizdmgfield(pos.x - ZOMBO_ATTACK_WIDTH,
                                   pos.y + ZOMBO_ATTACK_Y,
                                   ZOMBO_ATTACK_WIDTH, ZOMBO_ATTACK_HEIGHT,
                                   ZOMBO_ATTACK_Y, 1, world, team, aspeed_x,
                                   aspeed_y, attackdamage, this);
    }
    *ny = dmgfield;
    world->register_fxfield(*ny);

    break;
  case ZOMBO_STATE_PREPARING_ATTACK:
    aspeed_x = 0;
    animframe = 0;
    frames_until_picswitch = ZOMBO_ATTACK_ANIM_SPEED;
    if (look_right)
      set_pic(ZOMBO_R_ATK1);
    else
      set_pic(ZOMBO_L_ATK1);
    break;
  default:
    printf("Zombo::set_state() wring state %d old state was %d\n", newstate,
           state);
    exit(1);
  }

  state = newstate;
}

void Zombo::use_aura(){
  if (not aura){
    Fxfield **ny;
    ny = world->get_free_fxfield();
    aura = new Aura(pos.x, pos.y, pos.w, pos.h, this, world, team,
                    aurapower_x, aurapower_y, auradamage);
    *ny = aura;
    world->register_fxfield(*ny);
  }
  else{
    aura->lifetime = 1;
  }
}

void Zombo::think(){
  switch (state){
  case ZOMBO_STATE_PATROLLING:
    use_aura();

    if (below){
      int left_til_edge = below->pos.x + below->pos.w - pos.x - pos.w -
        (int)aspeed_x;
      int cfid; /* creature found id */
      Creature *cf; /* creature found */
      Scanresult res;
      SDL_Rect scanarea;
      bool stopscan = false;
      if (look_right){
        if (rightof || (left_til_edge < 0 &&
            not aitools->px_pf_scan(below->pos.x + below->pos.w,
                            below->pos.y))){
          look_right = false;
          frames_until_picswitch = 1;
          animframe = 2;
        }
        scanarea.w = ZOMBO_SCAN_DISTANCE;
        scanarea.x = pos.x + pos.w;
        scanarea.y = pos.y;
        scanarea.h = pos.h;
        if(not aitools->box_scan(CREATURES, res, scanarea)){
          while( not stopscan && res.count_creatures()){
            cfid = res.takeout_creature();
            cf = aitools->identify_creature(cfid);
            if (cf){
              if (not cf->dead && cf->team != team){
                set_state(ZOMBO_STATE_PREPARING_ATTACK);
                stopscan = true;
              }
            }
            else
              printf("Wrong creature id!\n");
          }
        }
      }
      else{ /* looking left */
        int left_til_edge = pos.x - below->pos.x + (int)aspeed_x;
        if (leftof || (left_til_edge < 0 &&
            not aitools->px_pf_scan(below->pos.x - 1, below->pos.y))){
          look_right = true;
          frames_until_picswitch = 1;
          animframe = 2;
        }
        scanarea.w = ZOMBO_SCAN_DISTANCE;
        scanarea.x = pos.x - scanarea.w;
        scanarea.y = pos.y;
        scanarea.h = pos.h;
        if(not aitools->box_scan(CREATURES, res, scanarea)){
          while(not stopscan && res.count_creatures()){
            cfid = res.takeout_creature();
            cf = aitools->identify_creature(cfid);
            if (cf){
              if (not cf->dead && cf->team != team){
                set_state(ZOMBO_STATE_PREPARING_ATTACK);
                stopscan = true;
              }
            }
            else
              printf("Wrong creature id!\n");
          }
        }
      }
      if (look_right){
        if (aspeed_x < patrolspeed)
          aspeed_x = patrolspeed;
      }
      else{
        if (aspeed_x > -patrolspeed)
          aspeed_x = -patrolspeed;
      }
    }

    frames_until_picswitch --;
    if (frames_until_picswitch == 0){
      animframe = (animframe + 1) % 4;
      frames_until_picswitch = ZOMBO_LEGSPEED_DEFAULT;
      if (look_right){
        switch(animframe){
        case 0: case 2:
          set_pic(ZOMBO_R_WALK2);
          break;
        case 1:
          set_pic(ZOMBO_R_WALK1);
          break;
        case 3:
          set_pic(ZOMBO_R_WALK3);
          break;
        }
      }
      else{
        switch(animframe){
        case 0: case 2:
          set_pic(ZOMBO_L_WALK2);
          break;
        case 1:
          set_pic(ZOMBO_L_WALK1);
          break;
        case 3:
          set_pic(ZOMBO_L_WALK3);
          break;
        }
      }
    }
    break;
  case ZOMBO_STATE_EXHAUSTED:
    use_aura();
    frames_until_picswitch --;
    if (frames_until_picswitch == 0){
      set_state(ZOMBO_STATE_PATROLLING);
    }
    if (look_right && rightof)
      look_right = false;
    else if (not look_right && leftof)
      look_right = true;
    break;
  case ZOMBO_STATE_RECOVERING:
    frames_until_picswitch --;
    if (frames_until_picswitch == 0){
      frames_until_picswitch = ZOMBO_RECOVERY_ANIM_SPEED;
      animframe ++;
      switch(animframe){
      case 1:
        if (look_right)
          set_pic(ZOMBO_R_HURT2);
        else
          set_pic(ZOMBO_L_HURT2);
        break;
      case 2:
        if (look_right)
          set_pic(ZOMBO_R_HURT3);
        else
          set_pic(ZOMBO_L_HURT3);
        break;
      case 3:
        if (look_right)
          set_pic(ZOMBO_R_HURT4);
        else
          set_pic(ZOMBO_L_HURT4);
        break;
      case 4:
        if (look_right)
          set_pic(ZOMBO_R_HURT5);
        else
          set_pic(ZOMBO_L_HURT5);
        break;
      case 5:
        set_state(ZOMBO_STATE_ATTACKING);
        break;
      default:
        printf("Zombo::think() in state ZOMBO_STATE_RECOVERING: wrong"
               " animframe\n");
        exit(1);
      }
    }
    break;
  case ZOMBO_STATE_ATTACKING:
    use_aura();
    if (dmgfield != NULL){
      dmgfield->update_pos(pos.x, pos.y);
      dmgfield->lifetime = 1;
    }

    frames_until_picswitch --;
    if (frames_until_picswitch == 0){
      dmgfield = NULL;
      set_state(ZOMBO_STATE_EXHAUSTED);
    }
    break;
  case ZOMBO_STATE_PREPARING_ATTACK:
    use_aura();
    frames_until_picswitch --;
    if (frames_until_picswitch == 0){
      frames_until_picswitch = ZOMBO_ATTACK_ANIM_SPEED;
      animframe ++;
      switch(animframe){
      case 1:
        if (look_right)
          set_pic(ZOMBO_R_ATK2);
        else
          set_pic(ZOMBO_L_ATK2);
        break;
      case 2:
        set_state(ZOMBO_STATE_ATTACKING);
        break;
      }
    }
    break;
  case ZOMBO_STATE_DYING:
    frames_until_picswitch --;
    if (frames_until_picswitch == 0){
      frames_until_picswitch = ZOMBO_DIE_ANIM_SPEED;
      switch(animframe){
      case 0:
        if (look_right)
          set_pic(ZOMBO_R_DYING2);
        else
          set_pic(ZOMBO_L_DYING2);
        break;
      case 1:
        if (look_right)
          set_pic(ZOMBO_R_DYING3);
        else
          set_pic(ZOMBO_L_DYING3);
        break;
      case 2:
        if (look_right)
          set_pic(ZOMBO_R_DYING4);
        else
          set_pic(ZOMBO_L_DYING4);
        break;
      case 3:
        set_state(ZOMBO_STATE_DEAD);
        break;
      default:
        printf("Zombo::dying() wrong animframe");
        exit(1);
      }
      animframe ++;
    }
    break;
  case ZOMBO_STATE_DEAD:
    break;
  default:
    printf("Zombo::think(): Unknown state %d\n", state);
    exit(1);
  }
}

void Zombo::move_x(int x_diff){
  pos.x = pos.x + x_diff;
  target_pos.x = target_pos.x + x_diff;
  if (aura != NULL){
    world->unregister_fxfield(aura);
    aura->col_rect[0].x = pos.x;
    aura->col_rect[0].w = pos.w;
    world->register_fxfield(aura);
  }
}

void Zombo::move_y(int y_diff){
  pos.y = pos.y + y_diff;
  target_pos.y = target_pos.y + y_diff;

  if (aura != NULL){
    world->unregister_fxfield(aura);
    aura->col_rect[0].y = pos.y;
    aura->col_rect[0].h = pos.h;
    world->register_fxfield(aura);
  }
}

std::string Zombo::generate_save_string(){
  stringstream ss;
  string ret, temp;
  ss << "zombo ";
  ss << pos.x;
  ss << " ";
  ss << pos.y;
  ss << " ";
  ss << look_right;
  ss << " ";
  ss << team;
  ret = ss.str();
  return ret;
}

void Zombo::paint(SDL_Surface **screen){
  SDL_Rect source, target;
  if (not settings.collision_mode){
    source = source_pos;
    prepare_to_show(target, target_pos);
    if(SDL_BlitSurface(pic, &source, *screen,
                       &target)){
      printf("Zombo:paint() Error while blitting player:%s",SDL_GetError());
      exit(1);
    }
  }
  else{
    prepare_to_show(target, pos);
    SDL_FillRect(*screen, &target,
                 SDL_MapRGB((*screen)->format,
                            50, 120, 100));
  }
  return;
}

void Zombo::set_pic(Zombopic zpic){
  switch(zpic){
  case ZOMBO_L_WALK2:
    source_pos.x = 3;
    source_pos.y = 3;
    source_pos.w = 37;
    source_pos.h = 62;
    target_pos.x = pos.x - 14;
    target_pos.y = pos.y - 9;
    break;

  case ZOMBO_L_WALK1:
    source_pos.x = 62;
    source_pos.y = 3;
    source_pos.w = 35;
    source_pos.h = 61;
    target_pos.x = pos.x - 12;
    target_pos.y = pos.y - 9;
    break;

  case ZOMBO_L_WALK3:
    source_pos.x = 112;
    source_pos.y = 3;
    source_pos.w = 39;
    source_pos.h = 61;
    target_pos.x = pos.x - 17;
    target_pos.y = pos.y - 9;
    break;

  case ZOMBO_L_ATK1:
    source_pos.x = 172;
    source_pos.y = 3;
    source_pos.w = 50;
    source_pos.h = 62;
    target_pos.x = pos.x - 23;
    target_pos.y = pos.y - 9;
    break;

  case ZOMBO_L_ATK2:
    source_pos.x = 258;
    source_pos.y = 3;
    source_pos.w = 45;
    source_pos.h = 62;
    target_pos.x = pos.x - 12;
    target_pos.y = pos.y - 9;
    break;

  case ZOMBO_L_ATK3:
    source_pos.x = 307;
    source_pos.y = 4;
    source_pos.w = 67;
    source_pos.h = 62;
    target_pos.x = pos.x - 42;
    target_pos.y = pos.y - 9;
    break;

  case ZOMBO_L_HURT1:
    source_pos.x = 389;
    source_pos.y = 1;
    source_pos.w = 54;
    source_pos.h = 66;
    target_pos.x = pos.x - 32;
    target_pos.y = pos.y - 13;
    break;

  case ZOMBO_L_HURT2:
    source_pos.x = 473;
    source_pos.y = 2;
    source_pos.w = 38;
    source_pos.h = 65;
    target_pos.x = pos.x - 11;
    target_pos.y = pos.y - 12;
    break;

  case ZOMBO_L_HURT3:
    source_pos.w = 35;
    source_pos.h = 65;
    source_pos.x = 539;
    source_pos.y = 4;
    target_pos.x = pos.x - 3;
    target_pos.y = pos.y - 10;
    break;

  case ZOMBO_L_HURT4:
    source_pos.w = 42;
    source_pos.h = 65;
    source_pos.x = 601;
    source_pos.y = 2;
    target_pos.x = pos.x - 10;
    target_pos.y = pos.y - 12;
    break;

  case ZOMBO_L_HURT5:
    source_pos.w = 42;
    source_pos.h = 66;
    source_pos.x = 667;
    source_pos.y = 2;
    target_pos.x = pos.x - 16;
    target_pos.y = pos.y - 13;
    break;

  case ZOMBO_L_DYING1:
    source_pos.w = 33;
    source_pos.h = 66;
    source_pos.x = 6;
    source_pos.y = 72;
    target_pos.x = pos.x - 11;
    target_pos.y = pos.y - 13;
    break;

  case ZOMBO_L_DYING2:
    source_pos.w = 50;
    source_pos.h = 60;
    source_pos.x = 61;
    source_pos.y = 76;
    target_pos.x = pos.x - 10;
    target_pos.y = pos.y - 7;
    break;

  case ZOMBO_L_DYING3:
    source_pos.w = 55;
    source_pos.h = 42;
    source_pos.x = 123;
    source_pos.y = 96;
    target_pos.x = pos.x - 11;
    target_pos.y = pos.y + 11;
    break;

  case ZOMBO_L_DYING4:
    source_pos.w = 55;
    source_pos.h = 26;
    source_pos.x = 191;
    source_pos.y = 109;
    target_pos.x = pos.x - 10;
    target_pos.y = pos.y + 27;
    break;

  case ZOMBO_L_DEAD:
    source_pos.w = 57;
    source_pos.h = 22;
    source_pos.x = 266;
    source_pos.y = 116;
    target_pos.x = pos.x - 11;
    target_pos.y = pos.y + 31;
    break;
   /* RIGHT DIR */

  case ZOMBO_R_DYING1:
    source_pos.w = 33;
    source_pos.h = 66;
    source_pos.x = 676;
    source_pos.y = 78;
    target_pos.x = pos.x - 3;
    target_pos.y = pos.y - 13;
    break;

  case ZOMBO_R_DYING2:
    source_pos.w = 50;
    source_pos.h = 60;
    source_pos.x = 604;
    source_pos.y = 82;
    target_pos.x = pos.x - 21;
    target_pos.y = pos.y - 7;
    break;

  case ZOMBO_R_DYING3:
    source_pos.w = 55;
    source_pos.h = 42;
    source_pos.x = 537;
    source_pos.y = 102;
    target_pos.x = pos.x - 25;
    target_pos.y = pos.y + 11;
    break;

  case ZOMBO_R_DYING4:
    source_pos.w = 56;
    source_pos.h = 26;
    source_pos.x = 468;
    source_pos.y = 115;
    target_pos.x = pos.x - 27;
    target_pos.y = pos.y + 27;
    break;

  case ZOMBO_R_DEAD:
    source_pos.w = 57;
    source_pos.h = 22;
    source_pos.x = 392;
    source_pos.y = 122;
    target_pos.x = pos.x - 27;
    target_pos.y = pos.y + 31;
    break;

  case ZOMBO_R_HURT1:
    source_pos.w = 54;
    source_pos.h = 66;
    source_pos.x = 272;
    source_pos.y = 147;
    target_pos.x = pos.x - 3;
    target_pos.y = pos.y - 13;
    break;

  case ZOMBO_R_HURT2:
    source_pos.w = 38;
    source_pos.h = 65;
    source_pos.x = 204;
    source_pos.y = 148;
    target_pos.x = pos.x - 8;
    target_pos.y = pos.y - 12;
    break;

  case ZOMBO_R_HURT3:
    source_pos.w = 35;
    source_pos.h = 65;
    source_pos.x = 140;
    source_pos.y = 150;
    target_pos.x = pos.x - 14;
    target_pos.y = pos.y - 10;
    break;

  case ZOMBO_R_HURT4:
    source_pos.w = 42;
    source_pos.h = 65;
    source_pos.x = 72;
    source_pos.y = 148;
    target_pos.x = pos.x - 13;
    target_pos.y = pos.y - 12;
    break;

  case ZOMBO_R_HURT5:
    source_pos.w = 42;
    source_pos.h = 66;
    source_pos.x = 6;
    source_pos.y = 148;
    target_pos.x = pos.x - 12;
    target_pos.y = pos.y - 13;
    break;

  case ZOMBO_R_ATK1:
    source_pos.x = 493;
    source_pos.y = 149;
    source_pos.w = 50;
    source_pos.h = 62;
    target_pos.x = pos.x - 8;
    target_pos.y = pos.y - 9;
    break;

  case ZOMBO_R_ATK2:
    source_pos.x = 412;
    source_pos.y = 149;
    source_pos.w = 45;
    source_pos.h = 62;
    target_pos.x = pos.x - 14;
    target_pos.y = pos.y - 9;
    break;

  case ZOMBO_R_ATK3:
    source_pos.x = 341;
    source_pos.y = 150;
    source_pos.w = 67;
    source_pos.h = 62;
    target_pos.x = pos.x - 6;
    target_pos.y = pos.y - 9;
    break;

  case ZOMBO_R_WALK2:
    source_pos.x = 675;
    source_pos.y = 149;
    source_pos.w = 37;
    source_pos.h = 62;
    target_pos.x = pos.x - 4;
    target_pos.y = pos.y - 9;
    break;

  case ZOMBO_R_WALK1:
    source_pos.x = 618;
    source_pos.y = 149;
    source_pos.w = 35;
    source_pos.h = 61;
    target_pos.x = pos.x - 4;
    target_pos.y = pos.y - 9;
    break;

  case ZOMBO_R_WALK3:
    source_pos.x = 564;
    source_pos.y = 149;
    source_pos.w = 39;
    source_pos.h = 61;
    target_pos.x = pos.x - 3;
    target_pos.y = pos.y - 9;
    break;

  default:
    printf("Zombo::set_pic: wrong pic %d\n", zpic);
    exit(1);
  }
  target_pos.w = source_pos.w;
  target_pos.h = source_pos.h;
}

