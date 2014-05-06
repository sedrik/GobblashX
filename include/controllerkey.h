#ifndef __CONTROLLERKEY_H__
#define __CONTROLLERKEY_H__

enum Inputdevice{KEYBOARD, JOYSTICK, NOTUSED};

class Controllerkey{
 public:
  Inputdevice inputdevice;
  int keycode; /* this is the keycode for the button if its a keyboard or if
                  its a joystickbutton */
  int joyid; /* if it's a joystick, this is the id, if it's a keyboard this is
                not used... */
};


#endif // __CONTROLLERKEY_H__
