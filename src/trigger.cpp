#include "trigger.h"
#include "stdio.h"
#include <sstream>
#include <string>
#include "filefunctions.h"
#include "game.h"

void Trigger::activate(Aitools *aitools, Map &world){
  Scanresult res;
  int cid;
  Creature *c;
  switch (act){
  case TA_VICTORY:
    game->complete_game();
    break;
  case TA_NONE:
    printf ("Trigger was activated, but it does nothing! %d\n",
            game->get_time());
    break;
  case TA_INTERN_TELEPORT:
    aitools->box_scan(CREATURES, res, acto.intel.lfrom->pos);
    cid = res.takeout_creature();
    while(cid != -1){
      c = aitools->identify_creature(cid);
      if (acto.intel.ctm == -1 || c->team == acto.intel.ctm){
        /* also check if he's the right type */
        world.unregister_creature(c);
        c->move_x(acto.intel.lto->pos.x + acto.intel.lto->pos.w / 2 -
                  c->pos.w / 2 - c->pos.x);
        c->move_y(acto.intel.lto->pos.y + acto.intel.lto->pos.h / 2 -
                  c->pos.h / 2 - c->pos.y);
        c->aspeed_x = 0;
        c->aspeed_y = 0;
        c->rspeed_x = 0;
        c->rspeed_y = 0;
        c->teleported();
        world.register_creature(c);
      }
      cid = res.takeout_creature();
    }
    break;
  default:
    printf ("Trigger was activated, but the action is not handeled %d!\n",
            game->get_time());
  }
}

void Trigger::set_cond(TriggerCondition newcond){
  switch (newcond){
  case TC_ENTER_LOCATION:
    condo.entloc.ltag = 0;
    break;
  default: ;
  }
  cond = newcond;
}

void Trigger::set_act(TriggerAction newact){
  switch (newact){
  case TA_INTERN_TELEPORT:
    acto.intel.frltag = -1;
    acto.intel.toltag = -1;
    acto.intel.ctm = -1;
    acto.intel.lfrom = NULL;
    acto.intel.lto = NULL;
    acto.intel.csbb = false;
    acto.intel.cpom = false;
    acto.intel.cpla = false;
    acto.intel.cblu = false;
    acto.intel.czom = false;
    break;
  default: ;
  }
  act = newact;
}

void Trigger::think(){
  printf ("trigger thinks...\n");
}

Trigger::Trigger(TriggerCondition itc, TriggerAction ita){
  cond = itc;
  act = ita;
  tag = 0;
}

Trigger::Trigger(Map *world, std::string initline){
  std::string word;
  int temp;
  TriggerCondition tc;
  TriggerAction ta;
  word = first_word(initline);
  safe_strtol(word, tag);
  word = first_word(initline);
  safe_strtol(word, temp);
  tc = (TriggerCondition)temp;
  cond = tc;
  switch (cond){
  case TC_NONE:
    break;
  case TC_ENTER_LOCATION:
    word = first_word(initline);
    safe_strtol(word, condo.entloc.ltag);
    break;
  default:
    printf("Error loading trigger: Undefined Condition %d\n", tc);
  }

  word = first_word(initline);
  safe_strtol(word, temp);
  ta= (TriggerAction)temp;
  act = ta;
  switch (act){
  case TA_NONE:
    break;
  case TA_INTERN_TELEPORT:
    word = first_word(initline); safe_strtol(word, acto.intel.frltag);
    word = first_word(initline); safe_strtol(word, acto.intel.toltag);
    word = first_word(initline); safe_strtol(word, acto.intel.ctm);

    word = first_word(initline); safe_strtol(word, acto.intel.csbb);
    word = first_word(initline); safe_strtol(word, acto.intel.cpom);
    word = first_word(initline); safe_strtol(word, acto.intel.cpla);
    word = first_word(initline); safe_strtol(word, acto.intel.cblu);
    word = first_word(initline); safe_strtol(word, acto.intel.czom);

    for (int iloc = 0; iloc < LOCATION_MAX; iloc ++){
      if (world->location[iloc] &&
          world->location[iloc]->tag == acto.intel.frltag)
        acto.intel.lfrom = world->location[iloc];
      if (world->location[iloc] &&
          world->location[iloc]->tag == acto.intel.toltag)
        acto.intel.lto = world->location[iloc];
    }
    break;
  case TA_VICTORY:
    break;
  default:
    printf("Error loading trigger: Undefined Action %d\n", ta);
  }

  for (int iloc = 0; iloc < LOCATION_MAX; iloc ++){
    if (world->location[iloc] &&
        world->location[iloc]->act_trigger == tag)
      world->location[iloc]->act_trigptr = this;
  }
}

std::string Trigger::generate_save_string(){
  stringstream ss;
  string ret, temp;
  ss << "trigger " << tag << " " << cond;
  switch (cond){
  case TC_NONE:
    break;
  case TC_ENTER_LOCATION:
    ss << " " << condo.entloc.ltag;
    break;
  /*
  case TC_ALL_ENEMIES_DEAD:
    break;
  */
  default:
    ss << " Cond_not_implemented";
  }

  ss << " " << act;
  switch (act){
  case TA_NONE:
    break;
  case TA_INTERN_TELEPORT:
    ss << " " << acto.intel.frltag;
    ss << " " << acto.intel.toltag;
    ss << " " << acto.intel.ctm;
    ss << " " << acto.intel.csbb;
    ss << " " << acto.intel.cpom;
    ss << " " << acto.intel.cpla;
    ss << " " << acto.intel.cblu;
    ss << " " << acto.intel.czom;
    break;
  case TA_VICTORY:
    break;
  default:
    ss << " Act_not_implemented";
  }

  ret = ss.str();
  return ret;
}
