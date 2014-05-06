#include "controller.h"
#include "game.h"
#include "settings.h"

bool Controller::b(ContButtons butt, Downtype t){
  bool down = (button_released[butt] < button_pressed[butt]);
  if (t == TIMING)
    return (down && button_pressed[butt] == game->get_time());
  else if (t == DOWN)
    return down;
  return false;
}

Controller::Controller(Keymappings *ikeymap){
  int i;
  keymap = ikeymap;

  for(i = 0; i < BUTTON_TOTAL; i++){
    button_released[i] = 1;
    button_pressed[i] = 0;
  }
}

void Controller::get_input(){
  int down, downmru, up, upmru; /* we're interrested in the alternative
                                   key that's been used most recently
                                   mru - most recently used
                                */
  for(int i = 0; i < BUTTON_TOTAL; i++){
      downmru = 0;
      upmru = 0;

      for (int u = 0; u < BUTTON_ALTERNATIVES; u++){
        if (keymap->key[u][i].inputdevice != NOTUSED){
          game->get_buttoninfo(keymap->key[u][i], up, down);
          if (down > downmru)
            downmru = down;
          if (up > upmru)
            upmru = up;
        }
      }
      button_pressed[i] = downmru;
      button_released[i] = upmru;
  }
  return;
}
