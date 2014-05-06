#include "game.h"
#include "settings.h"
#include "map.h"
#include "aitools.h"

Game *game;

void Game::complete_game(){
  game_state = GAME_COMPLETED;
}

Gamestate Game::get_state(){
  return game_state;
}

bool Game::playing(){
  return game_state == GAME_PLAYING;
}

void Game::get_buttoninfo(const Controllerkey &conkey, Sint32 &uptime,
                           Sint32 &downtime){
  if (conkey.inputdevice == KEYBOARD){
    uptime = keyup[conkey.keycode];
    downtime = keydown[conkey.keycode];
  }
  else if (conkey.inputdevice == JOYSTICK){
    if (conkey.joyid < 0 || conkey.joyid >= SDL_NumJoysticks()){
      printf("Errorous way of using joystick.\n"
             "Used joystick: %d. "
             "settings.joysticks: %d\n", conkey.joyid, SDL_NumJoysticks());
      exit(1);
    }
    uptime = joykeyup[conkey.joyid][conkey.keycode];
    downtime = joykeydown[conkey.joyid][conkey.keycode];
  }
}

void Game::set_aitools(Map *iworld){
  aitools = new Aitools(iworld);
}

void Game::set_world(Map *iworld){
  world = iworld;
}

Game::~Game(){
  if (aitools)
    delete aitools;

  for (int i = 0; i < joysticks; i++){
    SDL_JoystickClose(joystick[i]);
    printf("Closing joystick %d\n", i);
    delete [] joykeyup[i];
    delete [] joykeydown[i];
  }
  delete [] joystick;
  delete [] joykeyup;
  delete [] joykeydown;
}

Game::Game(){
  aitools = NULL;
  frametime = 16;     /* 16 + 2/3  ticks mellan varje frame */
  frametimefracup = 2;    /* dessa tre för att åstakomma de 2/3 extra tid */
  frametimefracdown = 3;  /* 60 fps = 16.66666667 */
  nextextraframe = 0;
  nextframe = 0;

  game_state = GAME_PLAYING;

  for (int i = 0; i < 10; i++)
    framelog[i] = 0;

  framenum = 1;
  quit = false;
  paused = false;
  gamekeymap[KEY_EXIT] = 9;  //escape
  gamekeymap[KEY_PAUSE] = 33; //p
  gamekeymap[KEY_COLLISIONMODE] = 67; //F1
  gamekeymap[KEY_SHOWSYM] = 68; //F2
  gamekeymap[KEY_SHOWPOS] = 69; //F3
  gamekeymap[KEY_STATS] = 70; //F4
  gamekeymap[KEY_SPEED] = 71; //F5
  gamekeymap[KEY_RESSURECT] = 72; //F6
  gamekeymap[KEY_SPAWN] = 73; //F7

  for (int i = 0; i < 256; i++){
    keyup[i] = 1;
    keydown[i] = 0;
  }

  /* prepare joysticks */
  SDL_InitSubSystem(SDL_INIT_JOYSTICK);
  joysticks = SDL_NumJoysticks();
  settings.joysticks = joysticks;
  if (joysticks > 0){
    joystick = new SDL_Joystick*[joysticks];
    joykeydown = new Sint32*[joysticks];
    joykeyup = new Sint32*[joysticks];
  }
  else{
    joystick = NULL;
    joykeydown = NULL;
    joykeyup = NULL;
  }

  if (joysticks == 0)
    printf("No joysticks found...\n");
  else
    printf("%d joysticks found\n", joysticks);

  for(int i = 0; i < joysticks; i++){
    printf("Joystick #%d: %s\n", i, SDL_JoystickName(i));
    joystick[i] = SDL_JoystickOpen(i);
    joykeyup[i] = new Sint32[256];
    joykeydown[i] = new Sint32[256];
    for (int u = 0; u < 256; u++){
      joykeyup[i][u] = 1;
      joykeydown[i][u] = 0;
    }
  }

  /* check if there's any keymap that uses an invalid joystick */
  Controllerkey *key;
  for (int i = 0; i < TOT_CONTROLLERS; i++){
    for (int a = 0; a < BUTTON_ALTERNATIVES; a++){
      for (int k = 0; k < BUTTON_TOTAL; k++){
        key = &settings.conkeymap[i].key[a][k];
        if (key->inputdevice == JOYSTICK &&
            (key->joyid < 0 ||
             key->joyid >= joysticks)){
          printf("Disabled input from joystick %d since it wasn't found"
                 " in the system\n", key->joyid);
          key->inputdevice = NOTUSED;
          key->keycode = -1;
          key->joyid = -1;
        }
      }
    }
  }
}

void Game::wait_until_next_frame(){
  while(SDL_GetTicks() < nextframe)
    SDL_Delay(1);
  nextextraframe = nextextraframe + frametimefracup;
  if (nextextraframe >= frametimefracdown){
    nextframe = nextframe + frametime + 1;
    nextextraframe = nextextraframe % frametimefracdown;
  }
  else{
    nextframe = nextframe + frametime;
  }
}

double Game::get_fps(){
  return fps;
}

void Game::next_frame(){
  double s;
  framenum ++;
  currframe = (currframe + 1) % 20;
  framelog[currframe] = SDL_GetTicks();
  s = (framelog[currframe] - framelog[(currframe + 1) % 20]);
  if (s)
    fps = 19000 / s;
  else
    fps = 0;
}

Sint32 Game::get_time(){
  return framenum;
}

void Game::handle_input(){
  if (keydown[gamekeymap[KEY_EXIT]] == get_time())
    quit = true;

  if (keydown[gamekeymap[KEY_PAUSE]] == get_time())
    paused = not paused;

  if (keydown[gamekeymap[KEY_COLLISIONMODE]] == get_time()){
    settings.collision_mode = not settings.collision_mode;
    settings.show_locations = settings.collision_mode;
    printf("collision mode: ");
    printf((settings.collision_mode?"true\n":"false\n"));
  }

  if (keydown[gamekeymap[KEY_SHOWSYM]] == get_time()) {
    settings.show_keysyms = not settings.show_keysyms;
    printf("showing keys: ");
    printf((settings.show_keysyms?"true\n":"false\n"));
  }

  if (keydown[gamekeymap[KEY_SHOWPOS]] == get_time()){
    if(world == NULL)
      printf("There exists no world yet\n");
    else
      printf("pos (x,y)=(%d,%d) w=%d h=%d\n", world->main_player->pos.x,
             world->main_player->pos.y, world->main_player->pos.w,
             world->main_player->pos.h);
  }

  if (keydown[gamekeymap[KEY_STATS]] == get_time())
    settings.showhp = not settings.showhp;

  if (keydown[gamekeymap[KEY_RESSURECT]] == get_time()) {
    printf("Everyone was ressureccted!\n");
    for(int i = 0; i <= world->get_creature_max(); i++){
      if (world->creature[i] != NULL){
        world->creature[i]->dead = false;
        world->creature[i]->hp = world->creature[i]->hpmax;
      }
    }
  }

  if (keydown[gamekeymap[KEY_SPEED]] == get_time()) {
    if (frametime == 0)
      frametime = 16;
    else
      frametime = 0;
    printf("frametime:%d+%d/%d\n", frametime, frametimefracup,
           frametimefracdown);
  }
}

void Game::get_input(){
  SDL_Event event;
  while( SDL_PollEvent( &event ) ){
    switch( event.type ){
    case SDL_KEYDOWN:
      keydown[event.key.keysym.scancode] = framenum;
      if (settings.show_keysyms){
        printf("frame: %d keydown: %d\n", get_time(),
               event.key.keysym.scancode);
        printf("up was: %d\n", keyup[event.key.keysym.scancode]);
        }
      break;
    case SDL_JOYBUTTONDOWN:
      joykeydown[event.jbutton.which][event.jbutton.button] = framenum;
      if (settings.show_keysyms)
        printf("joy %d: button down: %d\n", event.jbutton.which ,
               event.jbutton.button);
      break;

    case SDL_JOYBUTTONUP:
      joykeyup[event.jbutton.which][event.jbutton.button] = framenum;
      break;
    case SDL_KEYUP:
      keyup[event.key.keysym.scancode] = framenum;
      break;
      /*
    case SDL_JOYAXISMOTION:
      if ( ( event.jaxis.value < -3200 ) || (event.jaxis.value > 3200 ) )
        {
          if( event.jaxis.axis == 0)
            {
              // Left-right movement code goes here
              printf("Left-right %d\n", event.jaxis.value);
            }

          if( event.jaxis.axis == 1)
            {
              // Up-Down movement code goes here
              printf("Up-down %d\n", event.jaxis.value);

            }
        }
      else{
          printf("Center %d\n", 0);
      }
      break;
      */
    case SDL_QUIT:
      quit = true;
      break;

    default:
      //printf("unhandeled event\n");
      break;
    }
  }
  handle_input();
}
