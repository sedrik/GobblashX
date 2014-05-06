#ifndef __KEYMAPPINGS_H__
#define __KEYMAPPINGS_H__

#include "controllerkey.h"
#include <fstream>
#include <string>

enum ContButtons {BUTTON_MENU = 0,
                  BUTTON_JUMP,
                  BUTTON_ATTACK,
                  BUTTON_UP,
                  BUTTON_DOWN,
                  BUTTON_RIGHT,
                  BUTTON_LEFT
                  };
#define BUTTON_TOTAL 7

#define BUTTON_ALTERNATIVES 3

class Keymappings{
 private:
  int set_button(std::string &keystring, int button_code);
 public:
  Controllerkey key[BUTTON_ALTERNATIVES][BUTTON_TOTAL];
  void load_keymap(std::ifstream &configfile, const char filename[], int &rownum);
  Keymappings();
};

#endif // __KEYMAPPINGS_H__

