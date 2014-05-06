#include "keymappings.h"
#include "filefunctions.h"
#include "settings.h"

using namespace std;

int Keymappings::set_button(string &keystring, int button_code){
  string word;
  int altkey;
  word = first_word(keystring);
  if (safe_strtol(word, altkey) || altkey < 0 || altkey >= BUTTON_ALTERNATIVES)
    return 1;

  word = first_word(keystring);
  if (word == "kb"){
    int keycode;
    word = first_word(keystring);
    if (safe_strtol(word, keycode))
      return 1;
    key[altkey][button_code].inputdevice = KEYBOARD;
    key[altkey][button_code].keycode = keycode;
    key[altkey][button_code].joyid = -1;
  }
  else if (word == "joy"){
    int jid, keycode;
    word = first_word(keystring);
    if (safe_strtol(word, jid))
      return 1;
    word = first_word(keystring);
    if (safe_strtol(word, keycode))
      return 1;
    key[altkey][button_code].inputdevice = JOYSTICK;
    key[altkey][button_code].keycode = keycode;
    key[altkey][button_code].joyid = jid;
  }
  else{
    return 1;
  }
  return 0;
}

void Keymappings::load_keymap(std::ifstream &configfile, const char filename[],
                              int &rownum){
  string row, word;
  int buttonaction;
  word = skipcomments(configfile, row, rownum);
  do{
    if (word == "endcontroller"){
      break;
    }
    else if (word == "left")
      buttonaction = BUTTON_LEFT;
    else if (word == "down")
      buttonaction = BUTTON_DOWN;
    else if (word == "right")
      buttonaction = BUTTON_RIGHT;
    else if (word == "up")
      buttonaction = BUTTON_UP;
    else if (word == "jump")
      buttonaction = BUTTON_JUMP;
    else if (word == "attack")
      buttonaction = BUTTON_ATTACK;
    else if (word == "menu")
      buttonaction = BUTTON_MENU;
    else{
      printf("load_keymap: file %s:%d - unhandeled keyword: %s\n", filename,
             rownum, word.c_str());
      exit(1);
    }

    if (set_button(row, buttonaction)){
      printf("load_keymap: file %s:%d - unhandeled keyword\n", filename,
             rownum);
      exit(1);
    }
    word = skipcomments(configfile, row, rownum);
  }while(not configfile.eof());

}

Keymappings::Keymappings(){
  for(int u = 0; u < BUTTON_ALTERNATIVES; u++){
    for(int i = 0; i < BUTTON_TOTAL; i++){
      key[u][i].inputdevice = NOTUSED;
      key[u][i].keycode = -1;
      key[u][i].joyid = -1;
    }
  }
}
