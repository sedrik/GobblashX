#include "location.h"
#include "graphics.h"
#include <string>
#include <sstream>
#include "filefunctions.h"

Location::Location(){
  tag = 0;
  totalcol = false;
  act_trigptr = NULL;
  act_trigger = 0;
  act_team = 0;
  act_pomgob = false;
  act_player = false;
  act_sbbflyer = false;
  act_zombo = false;
}

Location::Location(string initline){
  std::string word;
  act_trigptr = NULL;

  word = first_word(initline);
  safe_strtol(word, pos.x);
  word = first_word(initline);
  safe_strtol(word, pos.y);
  word = first_word(initline);
  safe_strtol(word, pos.w);
  word = first_word(initline);
  safe_strtol(word, pos.h);

  word = first_word(initline);
  safe_strtol(word, totalcol);
  word = first_word(initline);
  safe_strtol(word, act_trigger);
  word = first_word(initline);
  safe_strtol(word, act_team);
  word = first_word(initline);
  safe_strtol(word, act_pomgob);
  word = first_word(initline);
  safe_strtol(word, act_player);
  word = first_word(initline);
  safe_strtol(word, act_sbbflyer);
  word = first_word(initline);
  safe_strtol(word, act_zombo);
  word = first_word(initline);
  safe_strtol(word, tag);
}

string Location::generate_save_string(){
  stringstream ss;
  string ret, temp;
  ss << "location ";
  ss << pos.x;
  ss << " ";
  ss << pos.y;
  ss << " ";
  ss << pos.w;
  ss << " ";
  ss << pos.h;
  ss << " ";
  ss << totalcol;
  ss << " ";
  ss << act_trigger;
  ss << " ";
  ss << act_team;
  ss << " ";
  ss << act_pomgob;
  ss << " ";
  ss << act_player;
  ss << " ";
  ss << act_sbbflyer;
  ss << " ";
  ss << act_zombo;
  ss << " ";
  ss << tag;

  ret =ss.str();
  return ret;
}

void Location::paint(SDL_Surface **screen){
  SDL_Rect target;
  prepare_to_show(target, pos);
  SDL_FillRect(*screen, &target,
               SDL_MapRGB((*screen)->format,
                          200, 0, 200));
}
