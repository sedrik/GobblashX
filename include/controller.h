#ifndef __CREATURECONTROLLER_H__
#define __CREATURECONTROLLER_H__

#include "keymappings.h"
#include <SDL/SDL.h>

enum Downtype {TIMING, DOWN};
//enum ContButtons defined in keymappings.h

class Controller
{
 private:
  Keymappings *keymap;
  /* Button status, 1 if pressed else 0 */
  Sint32 button_pressed[BUTTON_TOTAL];
  Sint32 button_released[BUTTON_TOTAL];

 public:
  /* Returns button status */
  bool b(ContButtons butt, Downtype t);
  /* t=TIMING -> was button butt button just pressed?
     t=DOWN -> is button butt down now?
  */

  Controller(Keymappings *ikeymap);
  void get_input(); //gets input from the game class
};

#endif //__CREATURECONTROLLER_H__
