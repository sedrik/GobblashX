#include "editor.h"
#include "SDL/SDL.h"
#include "game.h"
#include "settings.h"
#include "map.h"
#include "graphics.h"
#include "text.h"
#include <sstream>
#include "physics.h" //to get intersect_rect

Editor *editor;

int Button::buttoncount = 0;
SDL_Surface *Button::tilesurf = NULL;

void Button::paint(SDL_Surface **screen){
  SDL_Rect source, target;
  if (down)
    source = srcdown;
  else
    source = src;
  target = pos;

  if(SDL_BlitSurface(tilesurf, &source, *screen,
                     &target)){
    printf("Pomgob:paint() Error while blitting button:%s",SDL_GetError());
    exit(1);
  }
}

Button::Button(SDL_Surface **screen, int srcx, int srcy, int w, int h, int posx, int posy, int imenu, int ievent){
  if (buttoncount == 0){
    tilesurf = load_optimized_pic( "graphics/buttons.bmp", screen,
                                             255, 255, 255);
  }
  buttoncount ++;
  src.x = srcx;
  src.y = srcy;
  src.w = w;
  src.h = h;

  pos.x = posx;
  pos.y = posy;
  pos.w = w;
  pos.h = h;

  srcdown.x = src.x + src.w;
  srcdown.y = src.y;
  srcdown.w = w;
  srcdown.h = h;

  visible = true;
  down = false;
  menu = imenu;
  event = ievent;
}

Button::~Button(){
  buttoncount --;
  if (buttoncount == 0){
    SDL_FreeSurface( tilesurf );
  }
}

void Editor::unregister_everything(){
    for (int i = 0; i < CREATURE_MAX; i++){
      if (world->creature[i])
        world->unregister_creature(world->creature[i]);
    }
    for (int i = 0; i < CREATURE_MAX; i++){
      if (world->platform[i])
        world->unregister_platform(world->platform[i]);
    }
}

void Editor::register_everything(){
    for (int i = 0; i < CREATURE_MAX; i++){
      if (world->creature[i])
        world->register_creature(world->creature[i]);
    }
    for (int i = 0; i < CREATURE_MAX; i++){
      if (world->platform[i])
        world->register_platform(world->platform[i]);
    }
}

Editor::~Editor(){
  delete message;
}

Editor::Editor(SDL_Surface **screen, Map &iworld, string mapfolder,
               string mapfile){
  message = new Text(screen, 30, 550, "", 0);
  messagetime = 0;

  world = &iworld;

  creatures_think = false;
  gravity = false;
  gamecontrollers = false;
  quit = false;
  scrollspeed = 10;

  menuwidth = 200;
  menuvisible = false;
  currmenu = MENU_MAIN;
  buttondownindex = -1;

  mousex = 0;
  mousey = 0;
  mousebuttons = 0;
  mouselastdown = 0;
  marked_creature = NULL;
  selected_creature = NULL;
  marked_platform = NULL;
  selected_platform = NULL;
  marked_location = NULL;
  selected_location = NULL;

  selected_trigger = -1;

  keymap[EKEY_EXIT] = 9;  //escape
  keymap[EKEY_COLLISIONMODE] = 67; //F1
  keymap[EKEY_GOTOHERO] = 43; //h
  keymap[EKEY_GRAVITY] = 42; //g
  keymap[EKEY_CLEARMARK] = 65; //space
  keymap[EKEY_RESIZEPLATFORM] = 50; //tab
  keymap[EKEY_SHIFT] = 23; //left shift

  keymap[EKEY_LEFT] = 100;
  keymap[EKEY_RIGHT] = 102;
  keymap[EKEY_DOWN] = 104;
  keymap[EKEY_UP] = 98;

  // menu options
  defplateam = 0;
  defplalr = true;
  defplacon = 0;
  defplaskin = 0;

  defpomteam = 1;
  defpomlr = false;
  defpomjump = false;

  defsbbteam = 1;
  defsbblr = false;

  defzomteam = 1;
  defzomlr = false;

  defblulr = false;
  defblugoup = false;
  defbluteam = 1;

  mo_mapwidth = 2000;
  mo_mapheight = 1000;
  mo_file = mapfile;
  mo_folder = mapfolder;
  mo_bgpic[0] = "graphics/mountains.bmp";
  mo_bgpic[1] = "graphics/forest.bmp";
  mo_currbgpic=0;

  for (int i = 0; i < 256; i++){
    keyup[i] = 1;
    keydown[i] = 0;
  }

  for (int i = 0; i < MENUTEXTTOT; i++)
    text[i] = NULL;

  //x y defaulttext, menu
  text[0] = new Text(screen, 630, 80, "New Map", MENU_EDITORMAIN);
  text[1] = new Text(screen, 603, 125, "1 Mapwidth:??", MENU_EDITORMAIN);
  text[2] = new Text(screen, 603, 175, "2 Mapheight:??", MENU_EDITORMAIN);
  text[3] = new Text(screen, 603, 290, "3 mapfile ey?", MENU_EDITORMAIN);
  text[4] = new Text(screen, 603, 225, "4 bgpic ey?", MENU_EDITORMAIN);
  // init buttons...

  // arguments:
  // srcx srcy w h posx posy imenu, ievent
  // all downed buttons must be strictly rightof the nondowned buttons
  for (int i = 0; i < MENUBUTTONSTOT; i++)
    button[i] = NULL;

  /********************** Main menu ************************/
  button[0] = new Button(screen, 100, 10, 40, 25, 610, 10, MENU_MAIN, 1);
  button[1] = new Button(screen, 100, 37, 40, 25, 655, 10, MENU_MAIN, 2);
  button[2] = new Button(screen, 100, 63, 40, 25, 700, 10, MENU_MAIN, 3);
  button[3] = new Button(screen, 100, 89, 40, 25, 745, 10, MENU_MAIN, 4);
  button[4] = new Button(screen, 284, 14, 40, 25, 610, 40, MENU_MAIN, 41);
  button[5] = new Button(screen, 284, 40, 40, 25, 655, 40, MENU_MAIN, 42);

  /*********************** Editor -menu *********************/
  //back...
  button[6] = new Button(screen, 14, 89, 32, 23, 610, 30,
                             MENU_EDITORMAIN, 5);
  //+ and - buttons
  button[7] = new Button(screen, 11, 117, 32, 22, 625, 100,
                             MENU_EDITORMAIN, 6);
  button[8] = new Button(screen, 11, 141, 32, 22, 675, 100,
                             MENU_EDITORMAIN, 7);
  button[9] = new Button(screen, 11, 117, 32, 22, 625, 150,
                             MENU_EDITORMAIN, 8);
  button[10] = new Button(screen, 11, 141, 32, 22, 675, 150,
                             MENU_EDITORMAIN, 9);

  //create
  button[11] = new Button(screen, 188, 119, 40, 22, 740, 360,
                             MENU_EDITORMAIN, 10);
  //save
  button[12] = new Button(screen, 96, 121, 40, 22, 620, 360,
                             MENU_EDITORMAIN, 11);
  //load
  button[13] = new Button(screen, 95, 144, 40, 22, 680, 360,
                             MENU_EDITORMAIN, 12);
  //change bg
  button[14] = new Button(screen, 95, 170, 40, 22, 660, 200,
                             MENU_EDITORMAIN, 13);


  /********************* Platform -menu *********************/
  //back
  button[15] = new Button(screen, 14, 89, 32, 23, 610, 30,
                             MENU_PLATFORM, 5);

  //black
  button[16] = new Button(screen, 116, 251, 51, 46, 610, 60,
                             MENU_PLATFORM, 14);
  //grass
  button[17] = new Button(screen, 116, 204, 51, 46, 662, 60,
                             MENU_PLATFORM, 15);

  //delete
  button[18] = new Button(screen, 187, 167, 40, 22, 680, 185,
                             MENU_PLATFORM, 38);

  text[24] = new Text(screen, 602, 170, "24 platform selected",
                         MENU_PLATFORM);
  text[25] = new Text(screen, 602, 230, "25 stats",
                         MENU_PLATFORM);

  /******************* Creature -menu **********************/
  //back
  button[19] = new Button(screen, 14, 89, 32, 23, 610, 30,
                             MENU_CREATUREMAIN, 5);
  //player
  button[20] = new Button(screen, 7, 252, 51, 46, 610, 60,
                             MENU_CREATUREMAIN, 16);
  //pomgob
  button[21] = new Button(screen, 107, 301, 51, 46, 662, 60,
                             MENU_CREATUREMAIN, 17);
  //sbbflyer
  button[22] = new Button(screen, 2, 302, 51, 46, 714, 60,
                             MENU_CREATUREMAIN, 18);
  //Zombo
  button[77] = new Button(screen, 223, 251, 51, 46, 610, 120,
                          MENU_CREATUREMAIN, 55);
  //Blubber
  button[99] = new Button(screen, 211, 299, 51, 46, 662, 120,
                          MENU_CREATUREMAIN, 69);
  /********************   Player -menu   ********************/
  //back
  button[23] = new Button(screen, 14, 89, 32, 23, 610, 30,
                             MENU_CREATUREPLAYER, 5);
  text[5] = new Text(screen, 602, 60, "5 player selected",
                         MENU_CREATUREPLAYER);
  text[7] = new Text(screen, 603, 165, "7 look_right ???",
                         MENU_CREATUREPLAYER);
  text[8] = new Text(screen, 603, 245, "8 team ???", MENU_CREATUREPLAYER);
  text[6] = new Text(screen, 603, 325, "6 coords (x,y)", MENU_CREATUREPLAYER);
  text[27] = new Text(screen, 603, 405, "27 input ???", MENU_CREATUREPLAYER);
  text[42] = new Text(screen, 603, 485, "42 skin ???", MENU_CREATUREPLAYER);

  //plus
  button[24] = new Button(screen, 11, 117, 32, 22, 625, 260,
                             MENU_CREATUREPLAYER, 19);
  //minus
  button[25] = new Button(screen, 11, 141, 32, 22, 675, 260,
                             MENU_CREATUREPLAYER, 20);
  //delete
  button[26] = new Button(screen, 187, 167, 40, 22, 680, 75,
                             MENU_CREATUREPLAYER, 21);
  //playercreature
  button[27] = new Button(screen, 7, 252, 51, 46, 610, 75,
                             MENU_CREATUREPLAYER, 22);
  //toggle
  button[28] = new Button(screen, 95, 170, 40, 22, 660, 180,
                             MENU_CREATUREPLAYER, 23);

  //find
  button[29] = new Button(screen, 228, 198, 40, 22, 660, 340,
                             MENU_CREATUREPLAYER, 24);

  //plus
  button[30] = new Button(screen, 11, 117, 32, 22, 625, 420,
                             MENU_CREATUREPLAYER, 39);
  //minus
  button[31] = new Button(screen, 11, 141, 32, 22, 675, 420,
                             MENU_CREATUREPLAYER, 40);

  //plus
  button[79] = new Button(screen, 11, 117, 32, 22, 625, 500,
                             MENU_CREATUREPLAYER, 57);
  //minus
  button[80] = new Button(screen, 11, 141, 32, 22, 675, 500,
                             MENU_CREATUREPLAYER, 58);

  /********************   Pomgob -menu   ********************/
  //back
  button[32] = new Button(screen, 14, 89, 32, 23, 610, 30,
                             MENU_CREATUREPOMGOB, 5);

  text[9] = new Text(screen, 602, 60, "9 pomgob selected",
                         MENU_CREATUREPOMGOB);
  text[10] = new Text(screen, 603, 325, "10 coords (x,y)",MENU_CREATUREPOMGOB);
  text[11] = new Text(screen, 603, 165, "11 look_right ???",
                         MENU_CREATUREPOMGOB);
  text[12] = new Text(screen, 603, 245, "12 team ???", MENU_CREATUREPOMGOB);
  text[17] = new Text(screen, 603, 405, "17 mode:", MENU_CREATUREPOMGOB);
  //create pomgob button
  button[33] = new Button(screen, 107, 301, 51, 46, 610, 75,
                             MENU_CREATUREPOMGOB, 25);
  //plus
  button[34] = new Button(screen, 11, 117, 32, 22, 625, 260,
                             MENU_CREATUREPOMGOB, 19);
  //minus
  button[35] = new Button(screen, 11, 141, 32, 22, 675, 260,
                             MENU_CREATUREPOMGOB, 20);
  //delete
  button[36] = new Button(screen, 187, 167, 40, 22, 680, 75,
                             MENU_CREATUREPOMGOB, 21);
  //toggle-direction
  button[37] = new Button(screen, 95, 170, 40, 22, 660, 180,
                             MENU_CREATUREPOMGOB, 23);
  //find
  button[38] = new Button(screen, 228, 198, 40, 22, 660, 340,
                             MENU_CREATUREPOMGOB, 24);
  //toggle-mode
  button[39] = new Button(screen, 95, 170, 40, 22, 660, 420,
                             MENU_CREATUREPOMGOB, 27);

  /********************   Sbblyer -menu   ********************/
  //back
  button[40] = new Button(screen, 14, 89, 32, 23, 610, 30,
                             MENU_CREATURESBBFLYER, 5);
  //create sbbflyer
  button[41] = new Button(screen, 2, 302, 51, 46, 610, 75,
                             MENU_CREATURESBBFLYER, 26);
  text[13] = new Text(screen, 602, 60, "13 sbbflyer selected",
                         MENU_CREATURESBBFLYER);
  text[14] = new Text(screen, 603, 325, "14 coords (x,y)",
                      MENU_CREATURESBBFLYER);
  text[15] = new Text(screen, 603, 165, "15 look_right ???",
                         MENU_CREATURESBBFLYER);
  text[16] = new Text(screen, 603, 245, "16 team ???", MENU_CREATURESBBFLYER);
  //plus
  button[42] = new Button(screen, 11, 117, 32, 22, 625, 260,
                             MENU_CREATURESBBFLYER, 19);
  //minus
  button[43] = new Button(screen, 11, 141, 32, 22, 675, 260,
                             MENU_CREATURESBBFLYER, 20);
  //delete
  button[44] = new Button(screen, 187, 167, 40, 22, 680, 75,
                             MENU_CREATURESBBFLYER, 21);
  //toggle
  button[45] = new Button(screen, 95, 170, 40, 22, 660, 180,
                             MENU_CREATURESBBFLYER, 23);
  //find
  button[46] = new Button(screen, 228, 198, 40, 22, 660, 340,
                             MENU_CREATURESBBFLYER, 24);


  /********************   Blubber -menu   ********************/
  //back
  button[90] = new Button(screen, 14, 89, 32, 23, 610, 30,
                             MENU_CREATUREBLUBBER, 5);

  text[49] = new Text(screen, 602, 60, "49 blubber selected",
                         MENU_CREATUREBLUBBER);
  text[50] = new Text(screen, 603, 325, "50 coords (x,y)",
                      MENU_CREATUREBLUBBER);
  text[51] = new Text(screen, 603, 165, "51 look_right ???",
                         MENU_CREATUREBLUBBER);
  text[52] = new Text(screen, 603, 245, "52 team ???", MENU_CREATUREBLUBBER);
  text[53] = new Text(screen, 603, 405, "53 mode:", MENU_CREATUREBLUBBER);
  //create blubber button
  button[91] = new Button(screen, 211, 299, 51, 46, 610, 75,
                             MENU_CREATUREBLUBBER, 70);
  //plus
  button[95] = new Button(screen, 11, 117, 32, 22, 625, 260,
                             MENU_CREATUREBLUBBER, 19);
  //minus
  button[94] = new Button(screen, 11, 141, 32, 22, 675, 260,
                             MENU_CREATUREBLUBBER, 20);
  //delete
  button[92] = new Button(screen, 187, 167, 40, 22, 680, 75,
                             MENU_CREATUREBLUBBER, 21);
  //toggle-direction
  button[93] = new Button(screen, 95, 170, 40, 22, 660, 180,
                             MENU_CREATUREBLUBBER, 23);
  //find
  button[96] = new Button(screen, 228, 198, 40, 22, 660, 340,
                             MENU_CREATUREBLUBBER, 24);
  //toggle-mode
  button[97] = new Button(screen, 95, 170, 40, 22, 660, 420,
                             MENU_CREATUREBLUBBER, 71);
  /********************   Zombo -menu   ********************/
  //back
  button[70] = new Button(screen, 14, 89, 32, 23, 610, 30,
                             MENU_CREATUREZOMBO, 5);
  //create zombo
  button[71] = new Button(screen, 223, 251, 51, 46, 610, 75,
                             MENU_CREATUREZOMBO, 54);
  text[37] = new Text(screen, 602, 60, "13 zombo selected",
                         MENU_CREATUREZOMBO);
  text[38] = new Text(screen, 603, 325, "14 coords (x,y)", MENU_CREATUREZOMBO);
  text[39] = new Text(screen, 603, 165, "15 look_right ???",
                         MENU_CREATUREZOMBO);
  text[40] = new Text(screen, 603, 245, "16 team ???", MENU_CREATUREZOMBO);
  //plus
  button[72] = new Button(screen, 11, 117, 32, 22, 625, 260,
                             MENU_CREATUREZOMBO, 19);
  //minus
  button[73] = new Button(screen, 11, 141, 32, 22, 675, 260,
                             MENU_CREATUREZOMBO, 20);
  //delete
  button[74] = new Button(screen, 187, 167, 40, 22, 680, 75,
                             MENU_CREATUREZOMBO, 21);
  //toggle
  button[75] = new Button(screen, 95, 170, 40, 22, 660, 180,
                             MENU_CREATUREZOMBO, 23);
  //find
  button[76] = new Button(screen, 228, 198, 40, 22, 660, 340,
                             MENU_CREATUREZOMBO, 24);

  /********************** Map -menu *************************/
  //back
  button[47] = new Button(screen, 14, 89, 32, 23, 610, 30, MENU_MAP, 5);

  text[26] = new Text(screen, 603, 65, "26 width:12345\nheight:12345",
                      MENU_MAP);
  text[18] = new Text(screen, 643, 141, "Resize map", MENU_MAP);
  text[19] = new Text(screen, 602, 225, "19 platforms ???\ntot\na\nb",
                      MENU_MAP);
  text[20] = new Text(screen, 602, 278, "delete all\nplatforms?", MENU_MAP);
  text[21] = new Text(screen, 602, 325, "21 creatures ???\ntot\na\nb\nc\nd",
                      MENU_MAP);
  text[22] = new Text(screen, 602, 405, "delete all\ncreatures?", MENU_MAP);

  //plus up
  button[48] = new Button(screen, 11, 117, 32, 22, 666, 107, MENU_MAP, 28);
  //minus up
  button[49] = new Button(screen, 11, 141, 32, 22, 702, 107, MENU_MAP, 29);

  //plus left
  button[50] = new Button(screen, 11, 117, 32, 22, 604, 122, MENU_MAP, 30);
  //minus left
  button[51] = new Button(screen, 11, 141, 32, 22, 604, 146, MENU_MAP, 31);

  //plus right
  button[52] = new Button(screen, 11, 117, 32, 22, 764, 122, MENU_MAP, 32);
  //minus right
  button[53] = new Button(screen, 11, 141, 32, 22, 764, 146, MENU_MAP, 33);

  //plus down
  button[54] = new Button(screen, 11, 117, 32, 22, 666, 161, MENU_MAP, 34);
  //minus down
  button[55] = new Button(screen, 11, 141, 32, 22, 702, 161, MENU_MAP, 35);


  //delete all platforms
  button[56] = new Button(screen, 187, 167, 40, 22, 730, 282,
                             MENU_MAP, 36);
  //delete all creatures
  button[57] = new Button(screen, 187, 167, 40, 22, 730, 405,
                             MENU_MAP, 37);


  /********************** Location -menu *************************/

  //on-off button
  button[78] = new Button(screen, 7, 2, 40, 25, 745, 55,
                             MENU_LOCATION, 56);
  text[41] = new Text(screen, 633, 55, "showing\nlocations",
                      MENU_LOCATION);

  button[58] = new Button(screen, 14, 89, 32, 23, 610, 30,
                          MENU_LOCATION, 5);
  text[28] = new Text(screen, 603, 95, "28 selected location",
                      MENU_LOCATION);
  //create
  button[59] = new Button(screen, 188, 119, 40, 22, 630, 115,
                          MENU_LOCATION, 43);
  //delete
  button[60] = new Button(screen, 187, 167, 40, 22, 710, 115,
                          MENU_LOCATION, 44);

  text[29] = new Text(screen, 603, 230, "UL:12345x12345\n"
                                        "DR::12345x12345\n",
                      MENU_LOCATION);

  text[56] = new Text(screen, 603, 165, "56 Tag",
                      MENU_LOCATION);
  //tag plus
  button[102] = new Button(screen, 11, 117, 32, 22, 666, 186, MENU_LOCATION,
                          73);
  //tag minus
  button[101] = new Button(screen, 11, 141, 32, 22, 702, 186, MENU_LOCATION,
                          74);

  //find
  button[61] = new Button(screen, 228, 198, 40, 22, 660, 260,
                             MENU_LOCATION, 24);

  text[30] = new Text(screen, 603, 290, "Collision\nreqiures\nfull contact",
                      MENU_LOCATION);

  //on-off button
  button[62] = new Button(screen, 7, 2, 40, 25, 745, 295,
                             MENU_LOCATION, 46);

  text[31] = new Text(screen, 603, 350, "Activate trigger:\ndisabled",
                      MENU_LOCATION);
  //activate for team plus
  button[63] = new Button(screen, 11, 117, 32, 22, 666, 376, MENU_LOCATION,
                          47);
  //activate for team minus
  button[64] = new Button(screen, 11, 141, 32, 22, 702, 376, MENU_LOCATION,
                          48);

  text[32] = new Text(screen, 603, 410, "Activate for\nany team",
                      MENU_LOCATION);
  //activate for team plus
  button[65] = new Button(screen, 11, 117, 32, 22, 666, 436, MENU_LOCATION,
                          49);
  //activate for team minus
  button[66] = new Button(screen, 11, 141, 32, 22, 702, 436, MENU_LOCATION,
                          50);

  text[33] = new Text(screen, 603, 470, "Activate for",
                      MENU_LOCATION);
  text[34] = new Text(screen, 603, 487, "Player",
                      MENU_LOCATION);
  button[67] = new Button(screen, 9, 59, 41, 15, 702, 485, MENU_LOCATION, 51);

  text[35] = new Text(screen, 603, 505, "Pomgob",
                      MENU_LOCATION);
  button[68] = new Button(screen, 9, 59, 41, 15, 702, 503, MENU_LOCATION, 52);

  text[36] = new Text(screen, 603, 523, "Sbbflyer",
                      MENU_LOCATION);
  button[69] = new Button(screen, 9, 59, 41, 15, 702, 521, MENU_LOCATION, 53);

  text[43] = new Text(screen, 603, 541, "Zombo",
                      MENU_LOCATION);
  button[81] = new Button(screen, 9, 59, 41, 15, 702, 539, MENU_LOCATION, 59);

  /*************************** Trigger-menu **************************/

  text[44] = new Text(screen, 603, 60, "44 TRIGGER(x/y)", MENU_TRIGGER);
  text[45] = new Text(screen, 643, 180, "Condition", MENU_TRIGGER);
  text[46] = new Text(screen, 603, 200, "46 eg. enter location", MENU_TRIGGER);
  text[47] = new Text(screen, 658, 395, "Action", MENU_TRIGGER);
  text[48] = new Text(screen, 603, 415, "48 eg. victory", MENU_TRIGGER);

  //back
  button[105] = new Button(screen, 14, 89, 32, 23, 610, 30, MENU_TRIGGER, 5);
  //create
  button[84] = new Button(screen, 188, 119, 40, 22, 642, 100, MENU_TRIGGER,62);
  //delete
  button[85] = new Button(screen, 187, 167, 40, 22, 700, 100, MENU_TRIGGER,63);

  //prev - previus trigger
  button[82] = new Button(screen, 282, 70, 32, 22, 650, 75, MENU_TRIGGER,60);
  //next - next trigger
  button[83] = new Button(screen, 282, 94, 32, 22, 700, 75, MENU_TRIGGER,61);


  text[57] = new Text(screen, 603, 130, "57 Tag",
                      MENU_TRIGGER);
  //tag plus
  button[103] = new Button(screen, 11, 117, 32, 22, 666, 145, MENU_TRIGGER,
                          75);
  //tag minus
  button[104] = new Button(screen, 11, 141, 32, 22, 702, 145, MENU_TRIGGER,
                          76);

  //prev - prev condition
  button[86] = new Button(screen, 282, 70, 32, 22, 650, 220, MENU_TRIGGER,64);
  //next - next condition
  button[87] = new Button(screen, 282, 94, 32, 22, 700, 220, MENU_TRIGGER,65);

  //prev - prev action
  button[88] = new Button(screen, 282, 70, 32, 22, 650, 435, MENU_TRIGGER,66);
  //next - next action
  button[89] = new Button(screen, 282, 94, 32, 22, 700, 435, MENU_TRIGGER,67);

  /**** Trigger Condition Enters_Location *****/
  text[58] = new Text(screen, 603, 260, "58 enters location", MENU_TRIGGER);

  /**** Trigger Action Game_Completed *****/
  text[54] = new Text(screen, 603, 475, "Game completed", MENU_TRIGGER);

  /**** Trigger Action Intern_Teleport *****/

  text[60] = new Text(screen, 603, 475, "60 from loc", MENU_TRIGGER);

  //from loc +
  button[100] = new Button(screen, 11, 117, 32, 22, 666, 490, MENU_TRIGGER,
                          72);

  //from loc -
  button[106] = new Button(screen, 11, 141, 32, 22, 702, 490, MENU_TRIGGER,77);


  text[61] = new Text(screen, 603, 520, "61 to loc", MENU_TRIGGER);
  //to loc +
  button[107] = new Button(screen, 11, 117, 32, 22, 666, 535, MENU_TRIGGER,
                          78);
  //to loc -
  button[108] = new Button(screen, 11, 141, 32, 22, 702, 535, MENU_TRIGGER,
                          79);

  text[62] = new Text(screen, 603, 565, "62 team", MENU_TRIGGER);
  //activate for team plus
  button[109] = new Button(screen, 11, 117, 32, 22, 666, 580, MENU_TRIGGER,
                          80);
  //activate for team minus
  button[110] = new Button(screen, 11, 141, 32, 22, 702, 580, MENU_TRIGGER,
                          81);

  text[63] = new Text(screen, 603, 610, "Teleport creatures", MENU_TRIGGER);

  text[64] = new Text(screen, 603, 628, "Player", MENU_TRIGGER);
  button[111] = new Button(screen, 9, 59, 41, 15, 702, 628, MENU_TRIGGER, 82);

  text[65] = new Text(screen, 603, 646, "Pomgob", MENU_TRIGGER);
  button[112] = new Button(screen, 9, 59, 41, 15, 702, 646, MENU_TRIGGER, 83);

  text[66] = new Text(screen, 603, 664, "Sbbflyer",  MENU_TRIGGER);
  button[113] = new Button(screen, 9, 59, 41, 15, 702, 664, MENU_TRIGGER, 84);

  text[67] = new Text(screen, 603, 682, "Zombo", MENU_TRIGGER);
  button[114] = new Button(screen, 9, 59, 41, 15, 702, 680, MENU_TRIGGER, 85);

  text[68] = new Text(screen, 603, 700, "Blubber",  MENU_TRIGGER);
  button[115] = new Button(screen, 9, 59, 41, 15, 702, 698, MENU_TRIGGER, 86);


  //next free button id:  116
  //next free buttonevent: 87
  //next free text id:     69

}

void Editor::toggle_menu(){
  if (not menuvisible)
    cliprectworld.w = settings.screen_w - menuwidth;
  else
    cliprectworld.w = settings.screen_w;

  menuvisible = not menuvisible;

  cliprectmenu.x = cliprectworld.x + cliprectworld.w;
  cliprectmenu.w = settings.screen_w - cliprectmenu.x;
  cliprectmenu.y = cliprectworld.y;
  cliprectmenu.h = cliprectworld.h;
  look.w = cliprectworld.w;
}

template <class TYP>
void Editor::add_to_string(string &s, TYP t){
  std::string temp;
  std::stringstream ss;
  ss << t;
  ss >> temp;
  s = s + temp;
}

bool Editor::control_shrink(SDL_Rect sr){
  int i;
  SDL_Rect is;
  for (i = 0; i < CREATURE_MAX; i++){
    if (world->creature[i] && intersect_rect(world->creature[i]->pos,sr,is)){
      look.x = world->creature[i]->pos.x + world->creature[i]->pos.w / 2
        - look.w / 2;
      look.y = world->creature[i]->pos.y + world->creature[i]->pos.h / 2
        - look.h / 2;
      show_message( "Free space is needed in order to shrink map\n"
        "Please move this creature out of the way!", 180);
      return false;
    }
  }

  for (i = 0; i < PLATFORM_MAX; i++){
    if (world->platform[i] && intersect_rect(world->platform[i]->pos,sr,is)){
      look.x = world->platform[i]->pos.x + world->platform[i]->pos.w / 2
        - look.w / 2;
      look.y = world->platform[i]->pos.y + world->platform[i]->pos.h / 2
        - look.h / 2;
      show_message("Free space is needed in order to shrink map\n"
        "Please move this platform out of the way!", 180);
      return false;
    }
  }

  return true;
}

void Editor::button_event(int eventnum){
  int i;
  string o;
  bool failed;
  int nextt;
  int next_loctag = LOCATIONTAGMAX +1;
  int locid = -1;
  SDL_Rect rr;//removed rect. (when resizing map)
  Player *p;
  Pomgob *pom;
  Sbbflyer *sbb;
  Zombo *zom;
  Blubber *blu;
  Trigger *trig;

  switch(eventnum){
  case 1: //editor
    submenu_show(MENU_EDITORMAIN);
    break;
  case 2://map
    submenu_show(MENU_MAP);
    break;
  case 3://creature
    submenu_show(MENU_CREATUREMAIN);
    break;
  case 4://platform
    submenu_show(MENU_PLATFORM);
    break;
  case 5: //back-arrow
    selected_creature = NULL;
    selected_platform = NULL;
    selected_location = NULL;
    submenu_show(MENU_MAIN);
    break;
  case 6: //width +
    mo_mapwidth += MAPWIDTHSTEP;
    if (mo_mapwidth > MAPWIDTHMAX)
      mo_mapwidth = MAPWIDTHMAX;
    submenu_show(MENU_EDITORMAIN);
    break;
  case 7: //width -
    mo_mapwidth -= MAPWIDTHSTEP;
    if (mo_mapwidth < MAPWIDTHMIN)
      mo_mapwidth = MAPWIDTHMIN;
    submenu_show(MENU_EDITORMAIN);
    break;
  case 8: //height +
    mo_mapheight += MAPHEIGHTSTEP;
    if (mo_mapheight > MAPHEIGHTMAX)
      mo_mapheight = MAPHEIGHTMAX;
    submenu_show(MENU_EDITORMAIN);
    break;
  case 9: //height -
    mo_mapheight -= MAPHEIGHTSTEP;
    if (mo_mapheight < MAPHEIGHTMIN)
      mo_mapheight = MAPHEIGHTMIN;
    submenu_show(MENU_EDITORMAIN);
    break;
  case 10: //new map
    world->free_map();
    world->new_map(screen, mo_mapwidth, mo_mapheight, mo_bgpic[mo_currbgpic]);
    marked_creature = NULL;
    selected_creature = NULL;
    marked_platform = NULL;
    selected_platform = NULL;
    break;
  case 11: //save map
    world->set_bg( mo_bgpic[mo_currbgpic].c_str(), screen );
    if (world->save_map( (mo_folder + mo_file).c_str() ) == 0)
      show_message("Map saved", 90);
    else
      show_message("Error while saving map!", 230);
    break;
  case 13: //toggle bgpic
    mo_currbgpic = (mo_currbgpic + 1) % BGPICSTOT;
    submenu_show(MENU_EDITORMAIN);
    break;
  case 14: //platform nothing
    failed = true;
    for(i = 0; i <= PLATFORM_MAX; i++){
      if (world->platform[i] == NULL){
        world->platform[i] = new Platform( look.x + settings.screen_w / 2 - 20,
                                    look.y + settings.screen_h / 2 - 20,
                                    40, 40 );
        world->put_in_boundary(world->platform[i]);
        world->register_platform(world->platform[i]);
        selected_platform = world->platform[i];
        failed = false;;
        i = PLATFORM_MAX + 1; //breaks
      }
    }
    if (failed)
      show_message("No free space to create platform...", 130);
    break;
  case 15: //platform grass
    failed = true;
    for(i = 0; i <= PLATFORM_MAX; i++){
      if (world->platform[i] == NULL){
        world->platform[i] = new Grassplatform( look.x + settings.screen_w / 2
                                                - 20,
                            look.y + settings.screen_h / 2 - 20, 40, 40, screen,
                            world);
        world->put_in_boundary(world->platform[i]);
        world->register_platform(world->platform[i]);
        selected_platform = world->platform[i];
        game->world->generate_graphics_surroundings(world->platform[i]->pos);
        failed = false;
        i = PLATFORM_MAX + 1; //breaks
      }
    }
    if (failed)
      show_message("No free space to create grass...", 130);
    break;
  case 16: //creature player
    submenu_show(MENU_CREATUREPLAYER);
    break;
  case 17: //creature pomgob
    submenu_show(MENU_CREATUREPOMGOB);
    break;
  case 18: //creature sbb-flyer
    submenu_show(MENU_CREATURESBBFLYER);
    break;
  case 23: //creature toggle look_right
    p = dynamic_cast<Player*>(selected_creature);
    pom = dynamic_cast<Pomgob*>(selected_creature);
    sbb = dynamic_cast<Sbbflyer*>(selected_creature);
    zom = dynamic_cast<Zombo*>(selected_creature);
    blu = dynamic_cast<Blubber*>(selected_creature);
    if (p){
      p->set_look_right( not p->looking_right() );
      defplalr = p->looking_right();;
    }
    else if (pom){
      pom->set_look_right( not pom->looking_right() );
      defpomlr = pom->looking_right();;
    }
    else if (sbb){
      sbb->set_look_right( not sbb->looking_right() );
      defsbblr = sbb->looking_right();;
    }
    else if (zom){
      zom->set_look_right( not zom->looking_right() );
      defzomlr = zom->looking_right();;
    }
    else if (blu){
      blu->toggle_look_right();
      defblulr = blu->looking_right();;
    }
    else
      show_message("No creature is selected!", 90);

    submenu_show(currmenu);
    break;
  case 19: //creature team +
    if (selected_creature)
        if (selected_creature->team < 50)
          selected_creature->team = selected_creature->team + 1;
        else
          show_message("Limit reached!", 90);
    else
      show_message("No creature is selected!", 90);
    p = dynamic_cast<Player*>(selected_creature);
    pom = dynamic_cast<Pomgob*>(selected_creature);
    sbb = dynamic_cast<Sbbflyer*>(selected_creature);
    zom = dynamic_cast<Zombo*>(selected_creature);
    if (p)
      defplateam = selected_creature->team;
    else if (pom)
      defpomteam = selected_creature->team;
    else if (sbb)
      defsbbteam = selected_creature->team;
    else if (zom)
      defzomteam = selected_creature->team;
    submenu_show(currmenu);
    break;
  case 20: //creature team -
    if (selected_creature)
      if (selected_creature->team > 0)
        selected_creature->team = selected_creature->team - 1;
      else
        show_message("Limit reached!", 90);
    else
      show_message("No creature is selected!", 90);
    p = dynamic_cast<Player*>(selected_creature);
    pom = dynamic_cast<Pomgob*>(selected_creature);
    sbb = dynamic_cast<Sbbflyer*>(selected_creature);
    zom = dynamic_cast<Zombo*>(selected_creature);
    if (p)
      defplateam = selected_creature->team;
    else if (pom)
      defpomteam = selected_creature->team;
    else if (sbb)
      defsbbteam = selected_creature->team;
    else if (zom)
      defzomteam = selected_creature->team;
    submenu_show(currmenu);
    break;
  case 24: //find
    if (selected_creature){
      look.x = selected_creature->pos.x + selected_creature->pos.w / 2 -
        look.w / 2;
      look.y = selected_creature->pos.y + selected_creature->pos.h / 2 -
        look.h / 2;
    }
    else if (selected_location){
      look.x = selected_location->pos.x + selected_location->pos.w / 2 -
        look.w / 2;
      look.y = selected_location->pos.y + selected_location->pos.h / 2 -
        look.h / 2;
    }
    else
      show_message("Nothing is selected!", 90);
    break;
  case 21: //creature delete
    if (selected_creature){
      for (i = 0; i < CREATURE_MAX; i++){
        if (world->creature[i] == selected_creature){
          world->unregister_creature(world->creature[i]);
          delete world->creature[i];
          world->creature[i] = NULL;
          selected_creature = NULL;
          marked_creature = NULL;
          submenu_show(currmenu);
          i = CREATURE_MAX; //break
        }
      }
    }
    else
      show_message("No creature is selected!", 90);
    break;
  case 22: //creature player create
    failed = true;
    for(i = 0; i < world->get_creature_max(); i++){
      if (world->creature[i] == NULL){
        world->creature[i] = new Player(screen, NULL, look.x + look.w / 2,
                                 look.y + look.h / 2, world, defplalr,
                                 defplateam, defplacon, defplaskin);
        cib(world->creature[i]);
        world->register_creature(world->creature[i]);
        selected_creature = world->creature[i];
        submenu_show(currmenu);
        failed = false;
        i = world->get_creature_max();
      }
    }
    if (failed)
      show_message("No free space to create creature...", 130);
    break;
  case 25: //creature pomgob create
    failed = true;
    for(i = 0; i < world->get_creature_max(); i++){
      if (world->creature[i] == NULL){
        world->creature[i] = new Pomgob(screen, look.x + look.w / 2,
                                 look.y + look.h / 2, defpomlr,
                                 not defpomjump, defpomteam,
                                 world);
        cib(world->creature[i]);
        world->register_creature(world->creature[i]);
        selected_creature = world->creature[i];
        submenu_show(currmenu);
        failed = false;
        i = world->get_creature_max();
      }
    }
    if (failed)
      show_message("No free space to create creature...", 130);
    break;
  case 26: //creature sbbflyer create
    failed = true;
    for(i = 0; i < world->get_creature_max(); i++){
      if (world->creature[i] == NULL){
        world->creature[i] = new Sbbflyer(screen, look.x + look.w / 2,
                                 look.y + look.h / 2, defsbblr, defsbbteam,
                                 world);
        cib(world->creature[i]);
        world->register_creature(world->creature[i]);
        selected_creature = world->creature[i];
        submenu_show(currmenu);
        failed = false;
        i = world->get_creature_max();
      }
    }
    if (failed)
      show_message("No free space to create creature...", 130);
    break;
  case 54: //creature zombo create
    failed = true;
    for(i = 0; i < world->get_creature_max(); i++){
      if (world->creature[i] == NULL){
        world->creature[i] = new Zombo(screen, look.x + look.w / 2,
                                 look.y + look.h / 2, defzomlr,
                                 defzomteam, world, NULL);
        cib(world->creature[i]);
        world->register_creature(world->creature[i]);
        selected_creature = world->creature[i];
        submenu_show(currmenu);
        failed = false;
        i = world->get_creature_max();
      }
    }
    if (failed)
      show_message("No free space to create creature...", 130);
    break;
  case 27: //pomgob change mode
    pom = dynamic_cast<Pomgob*>(selected_creature);
    if (pom){
      pom->set_stand_still(not pom->get_stand_still());
      defpomjump = not pom->get_stand_still();
      submenu_show(currmenu);
    }
    else
      show_message("No creature is selected!", 90);
    break;
  case 28: //resizemap up +
    unregister_everything();
    world->boundary.h += 100;
    world->boundary.y -= 100;
    if (world->boundary.h > MAPWIDTHMAX){
      world->boundary.h -= 100;
      world->boundary.y += 100;
    }
    world->grid->set_size(world->boundary);
    register_everything();
    submenu_show(currmenu);
    break;
  case 30: //resize left +
    unregister_everything();
    world->boundary.w += 100;
    world->boundary.x -= 100;
    if (world->boundary.w > MAPWIDTHMAX){
      world->boundary.w -= 100;
      world->boundary.x += 100;
    }
    world->grid->set_size(world->boundary);
    register_everything();
    submenu_show(currmenu);
    break;
  case 34: //resize down +
    unregister_everything();
    world->boundary.h += 100;
    if (world->boundary.h > MAPHEIGHTMAX)
      world->boundary.h = MAPHEIGHTMAX;
    world->grid->set_size(world->boundary);
    register_everything();
    submenu_show(currmenu);
    break;
  case 32: //resizemap width +
    unregister_everything();
    world->boundary.w += 100;
    if (world->boundary.w > MAPWIDTHMAX)
      world->boundary.w = MAPWIDTHMAX;
    world->grid->set_size(world->boundary);
    register_everything();
    submenu_show(currmenu);
    break;
  case 29: //shrink above
    rr.x = world->boundary.x;
    rr.w = world->boundary.w;
    rr.h = 100;
    rr.y = world->boundary.y;

    if (not control_shrink(rr))
      break;

    unregister_everything();
    world->boundary.h -= 100;
    world->boundary.y += 100;
    if (world->boundary.h < MAPHEIGHTMIN){
      show_message("Map cannot shrink any more", 120);
      world->boundary.h += 100;
      world->boundary.y -= 100;
    }
    world->grid->set_size(world->boundary);
    register_everything();
    submenu_show(currmenu);

    break;
  case 35: //shrink below
    rr.y = world->boundary.y + world->boundary.h - 100;
    rr.h = 100;
    rr.w = world->boundary.w;
    rr.x = world->boundary.x;

    if (not control_shrink(rr))
      break;

    unregister_everything();
    world->boundary.h -= 100;
    if (world->boundary.h < MAPHEIGHTMIN){
      message->str = "Map cannot shrink any more";
      messagetime = 120;
      world->boundary.h += 100;
    }
    world->grid->set_size(world->boundary);
    register_everything();
    submenu_show(currmenu);
    break;
  case 31: //shrink left
    rr.x = world->boundary.x;
    rr.w = 100;
    rr.h = world->boundary.h;
    rr.y = world->boundary.y;

    if (not control_shrink(rr))
      break;

    unregister_everything();
    world->boundary.w -= 100;
    world->boundary.x += 100;
    if (world->boundary.w < MAPWIDTHMIN){
      message->str = "Map cannot shrink any more";
      messagetime = 120;
      world->boundary.w += 100;
      world->boundary.x -= 100;
    }
    world->grid->set_size(world->boundary);
    register_everything();
    submenu_show(currmenu);

    break;
  case 33: //shrink right
    rr.x = world->boundary.x + world->boundary.w - 100;
    rr.w = 100;
    rr.h = world->boundary.h;
    rr.y = world->boundary.y;

    if (not control_shrink(rr))
      break;

    unregister_everything();
    world->boundary.w -= 100;
    if (world->boundary.w < MAPWIDTHMIN){
      message->str = "Map cannot shrink any more";
      messagetime = 120;
      world->boundary.w += 100;
    }
    world->grid->set_size(world->boundary);
    register_everything();
    submenu_show(currmenu);

    break;
  case 36: //delete all platforms
    show_message("deleting all platforms", 120);
    i = 0;
    while (i < world->get_platform_max()){
      if (world->platform[i]){
        world->unregister_platform(world->platform[i]);
        delete world->platform[i];
        world->platform[i] = NULL;
      }
      i ++;
    }
    submenu_show(currmenu);
    break;
  case 37: //delete all creatures
    show_message("deleting all creatures", 120);
    i = 0;
    while (i < world->get_creature_max()){
      if (world->creature[i]){
        world->unregister_creature(world->creature[i]);
        delete world->creature[i];
        world->creature[i] = NULL;
      }
      i ++;
    }
    submenu_show(currmenu);
    break;
  case 38: //delete platform
    if (selected_platform){
      for (i = 0; i < PLATFORM_MAX; i++){
        if (world->platform[i] == selected_platform){
          world->unregister_platform(world->platform[i]);
          delete world->platform[i];
          world->platform[i] = NULL;
          selected_platform = NULL;
          marked_platform = NULL;
          submenu_show(currmenu);
          i = PLATFORM_MAX;
        }
      }
    }
    else
      show_message("No platform is selected", 90);
    break;
  case 39: //player controller id +
    p = dynamic_cast<Player*>(selected_creature);
    if (p){
      p->set_controllerid( p->get_controllerid() + 1 );
    }
    submenu_show(currmenu);
    break;
  case 40: //player controller id -
    p = dynamic_cast<Player*>(selected_creature);
    if (p){
      p->set_controllerid( p->get_controllerid() - 1 );
    }
    submenu_show(currmenu);
    break;
  case 41: //menumain button trigger
    submenu_show(MENU_TRIGGER);
    settings.show_locations = true;
    break;
  case 42: //menumain button location
    settings.show_locations = true;
    submenu_show(MENU_LOCATION);
    break;
  case 55: //zombo button
    submenu_show(MENU_CREATUREZOMBO);
    break;
  case 56: // show locations on-off
    settings.show_locations = not settings.show_locations;
    submenu_show(currmenu);
    break;
  case 46: // location require full contact on-off
    if (selected_location){
      selected_location->totalcol = (selected_location->totalcol? 0 : 1);
      submenu_show(currmenu);
    }
    else
      show_message("No location is selected", 90);
    break;
  case 44: // location delete
    if (selected_location){
      for (i = 0; i < LOCATION_MAX; i++){
        if (world->location[i] == selected_location){
          world->unregister_location(world->location[i]);
          delete world->location[i];
          world->location[i] = NULL;
          selected_location = NULL;
          marked_location = NULL;
          submenu_show(currmenu);
          i = LOCATION_MAX;
        }
      }
    }
    else
      show_message("No location is selected", 90);
    break;
  case 43: // location new
    settings.show_locations = true;
    failed = true;
    for(i = 0; i < world->get_location_max() && failed; i++){
      if (world->location[i] == NULL){
        failed = false;
        world->location[i] = new Location;
        world->location[i]->pos.x = look.x + settings.screen_w / 2;
        world->location[i]->pos.y = look.y + settings.screen_h / 2;
        world->location[i]->pos.w = 20;
        world->location[i]->pos.h = 20;
        world->location[i]->tag = get_next_ltag(0, true);
        world->put_in_boundary(world->location[i]);
        world->register_location(world->location[i]);
        selected_location = world->location[i];
      }
    }
    if (failed)
      show_message("No free space to create location...", 130);
    break;
  case 47: // location act_trigger +
    if (selected_location){
      if (selected_location->tag > 0){
        int next_trigtag = TRIGGERTAGMAX +1;
        int trigid = -1;
        for (int i = 0; i < world->get_trigger_max(); i++){
          if (world->trigger[i] &&
              world->trigger[i]->cond == TC_ENTER_LOCATION &&
              world->trigger[i]->tag > selected_location->act_trigger &&
              world->trigger[i]->tag < next_trigtag){
            next_trigtag = world->trigger[i]->tag;
            trigid = i;
          }
        }
        if (next_trigtag <= TRIGGERTAGMAX){
          selected_location->act_trigger = next_trigtag;
          //world->trigger[trigid]->condo.entloc.ltag = selected_location->tag;
        }
        else
          show_message("No trigger with the condition \"enters location\""
                       "\nhas a higher tag", 200);
      }
      else
        show_message("Location need a tag in order to activate a trigger",
                     200);
    }
    else
      show_message("No location selected", 130);
    break;
  case 48: // location act_trigger -
    if (selected_location){
      if (selected_location->tag > 0){
        int next_trigtag = 0;
        int trigid = -1;
        if (selected_location->act_trigger==0){
          show_message("No trigger with the condition \"enters location\""
                       "\nhas a lower tag", 200);
        }
        else{
          for (int i = 0; i < world->get_trigger_max(); i++){
            if (world->trigger[i] &&
                world->trigger[i]->cond == TC_ENTER_LOCATION &&
                world->trigger[i]->tag < selected_location->act_trigger &&
                world->trigger[i]->tag > next_trigtag){
              next_trigtag = world->trigger[i]->tag;
              trigid = i;
            }
          }
          if (next_trigtag > 0){
            selected_location->act_trigger = next_trigtag;
          world->trigger[trigid]->condo.entloc.ltag = selected_location->tag;
          }
          else
            selected_location->act_trigger = 0;
        }
      }
      else
        show_message("Location need a tag in order to activate a trigger",
                     200);
    }
    else
      show_message("No location selected", 130);
    break;
  case 49: // locationactivation team +
    if (selected_location){
      selected_location->act_team += 1;
      submenu_show(currmenu);
    }
    else
      show_message("No location is selected", 90);
    break;
  case 50: // locationactivation team -
    if (selected_location){
      if (selected_location->act_team >= 0){
        selected_location->act_team -= 1;
        submenu_show(currmenu);
      }
    }
    else
      show_message("No location is selected", 90);
    break;
  case 51: // location activate for player
    if (selected_location)
      selected_location->act_player = not selected_location->act_player;
    else
      show_message("No location is selected", 90);
    submenu_show(currmenu);
    break;
  case 52: // location activate for pomgob
    if (selected_location)
      selected_location->act_pomgob = not selected_location->act_pomgob;
    else
      show_message("No location is selected", 90);
    submenu_show(currmenu);
    break;
  case 53: // location activate for sbbflyer
    if (selected_location)
      selected_location->act_sbbflyer = not selected_location->act_sbbflyer;
    else
      show_message("No location is selected", 90);
    submenu_show(currmenu);
    break;
  case 59: // location activate for zombo
    if (selected_location)
      selected_location->act_zombo = not selected_location->act_zombo;
    else
      show_message("No location is selected", 90);
    submenu_show(currmenu);
    break;
  case 73: //location tag +
    if (selected_location){
      selected_location->tag = get_next_ltag(selected_location->tag, true);
      if (selected_location->tag == 0)
        show_message("Limit reached", 90);
    }
    else
      show_message("No location is selected", 90);
    submenu_show(currmenu);
    break;
  case 74: //location tag -
    if (selected_location){
      if (selected_location->tag == 0)
        show_message("Limit reached", 90);
      else{
        selected_location->tag = get_next_ltag(selected_location->tag, false);
      }
    }
    else
      show_message("No location is selected", 90);
    submenu_show(currmenu);
    break;

  case 57: //set playerskin +
    p = dynamic_cast<Player*>(selected_creature);
    if (p && p->get_skin() < 1)
      p->set_skin( p->get_skin() + 1 );
    defplaskin = p->get_skin();
    submenu_show(currmenu);
    break;
  case 58: //set playerskin -
    p = dynamic_cast<Player*>(selected_creature);
    if (p && p->get_skin() > 0)
      p->set_skin( p->get_skin() - 1 );
    defplaskin = p->get_skin();
    submenu_show(currmenu);
    break;
  case 60: //prev trigger
    if (selected_trigger >= 0){
      nextt = get_next_trigger(world->trigger[selected_trigger]->tag, false);
      selected_trigger = nextt;
    }
    else
      show_message("No previous trigger", 130);

    break;
  case 61: //next trigger
    if (selected_trigger >= 0)
      nextt = get_next_trigger(world->trigger[selected_trigger]->tag, true);
    else
      nextt = get_next_trigger(0, true);

    if (nextt == -1)
      show_message("No further trigger", 130);
    else
      selected_trigger = nextt;
    break;
  case 62: // new trigger
    failed = true;
    for(i = 0; i < world->get_trigger_max(); i++){
      if (world->trigger[i] == NULL){
        world->trigger[i] = new Trigger(TC_NONE, TA_NONE);
        selected_trigger = i;
        world->trigger[i]->tag = get_next_ttag(0,true);
        submenu_show(currmenu);
        failed = false;
        i = world->get_trigger_max();
      }
    }
    if (failed)
      show_message("No free space to create trigger...", 130);
    break;
  case 63: //delete trigger
    if (selected_trigger != -1){
      printf("deleting trigger %d\n", selected_trigger);
      delete world->trigger[selected_trigger];
      world->trigger[selected_trigger] = NULL;
      while(selected_trigger > -1 && world->trigger[selected_trigger] == NULL)
            selected_trigger -= 1;
      submenu_show(currmenu);
    }
    else
      show_message("No trigger is selected", 130);
    break;

  case 75: //next trigger tag, ttag +
    if (selected_trigger != -1){
      world->trigger[selected_trigger]->tag =
        get_next_ttag(world->trigger[selected_trigger]->tag, true);
      if (world->trigger[selected_trigger]->tag == 0){
        show_message("reached limit", 130);
      }
    }
    break;
  case 76: //prev trigger tag, ttag -
    if (selected_trigger != -1){
      if (world->trigger[selected_trigger]->tag == 0){
        show_message("reached limit", 130);
      }
      else
        world->trigger[selected_trigger]->tag =
          get_next_ttag(world->trigger[selected_trigger]->tag, false);
    }
    break;
  case 65: //trigger next cond
    if (selected_trigger != -1){
      trig = world->trigger[selected_trigger];
      switch(trig->cond){
      case TC_NONE:
        trig->set_cond(TC_ENTER_LOCATION);
        break;
      case TC_ENTER_LOCATION:
        trig->set_cond(TC_ALL_ENEMIES_DEAD);
        break;
      case TC_ALL_ENEMIES_DEAD:
        show_message("There are no other options", 120);
        break;
      }
    }
    break;
  case 64: //trigger prev cond
    if (selected_trigger != -1){
      trig = world->trigger[selected_trigger];
      switch(trig->cond){
      case TC_NONE:
        show_message("There are no other options", 120);
        break;
      case TC_ENTER_LOCATION:
        trig->set_cond(TC_NONE);
        break;
      case TC_ALL_ENEMIES_DEAD:
        trig->set_cond(TC_ENTER_LOCATION);
        break;
      }
    }
    break;
  case 66: //trigger prev-action
    if (selected_trigger != -1){
      trig = world->trigger[selected_trigger];
      switch(trig->act){
      case TA_NONE:
        show_message("There are no other options", 120);
        break;
      case TA_VICTORY:
        trig->set_act(TA_NONE);
        break;
      case TA_DEFEAT:
        trig->set_act(TA_VICTORY);
        break;
      case TA_INTERN_TELEPORT:
        trig->set_act(TA_DEFEAT);
        break;
      }
    }
    break;
  case 67: //trigger next-action
    if (selected_trigger != -1){
      trig = world->trigger[selected_trigger];
      switch(world->trigger[selected_trigger]->act){
      case TA_NONE:
        trig->set_act(TA_VICTORY);
        break;
      case TA_VICTORY:
        trig->set_act(TA_DEFEAT);
        break;
      case TA_DEFEAT:
        trig->set_act(TA_INTERN_TELEPORT);
        break;
      case TA_INTERN_TELEPORT:
        show_message("There are no other options", 120);
        break;
      }
    }
    break;
  case 69: //creautre blubber
    submenu_show(MENU_CREATUREBLUBBER);
    break;
  case 70: // create blubber
    failed = true;
    for(i = 0; i < world->get_creature_max(); i++){
      if (world->creature[i] == NULL){
        world->creature[i] = new Blubber(look.x + look.w / 2,
                                         look.y + look.h / 2, defbluteam,
                                         defblulr, defblugoup,
                                         screen, NULL, world);
        /* also needs to select type! */
        cib(world->creature[i]);
        world->register_creature(world->creature[i]);
        selected_creature = world->creature[i];
        submenu_show(currmenu);
        failed = false;
        i = world->get_creature_max();
      }
    }
    if (failed)
      show_message("No free space to create creature...", 130);
    break;
  case 71: // toggle-blubber fall dir
    blu = dynamic_cast<Blubber*>(selected_creature);
    if (blu){
      blu->set_falling(not blu->get_falling());
      defblugoup = not blu->get_falling();
    }
    else
      show_message("Not selected", 70);
    break;
  case 82: //trig act intel player
    trig = world->trigger[selected_trigger];
    if (trig)
      trig->acto.intel.cpla = not trig->acto.intel.cpla;
    else
      show_message("Not selected", 70);
    break;
  case 83: //trig act intel pomgob
    trig = world->trigger[selected_trigger];
    if (trig)
      trig->acto.intel.cpom = not trig->acto.intel.cpom;
    else
      show_message("Not selected", 70);
    break;
  case 84: //trig act intel sbbflyer
    trig = world->trigger[selected_trigger];
    if (trig)
      trig->acto.intel.csbb = not trig->acto.intel.csbb;
    else
      show_message("Not selected", 70);
    break;
  case 85: //trig act intel zombo
    trig = world->trigger[selected_trigger];
    if (trig)
      trig->acto.intel.czom = not trig->acto.intel.czom;
    else
      show_message("Not selected", 70);
    break;
  case 86: //trig act intel blubber
    trig = world->trigger[selected_trigger];
    if (trig)
      trig->acto.intel.cblu = not trig->acto.intel.cblu;
    else
      show_message("Not selected", 70);
    break;
  case 80: //team +
    trig = world->trigger[selected_trigger];
    if (trig)
      trig->acto.intel.ctm ++;
    else
      show_message("Not selected", 70);
    break;
  case 81:
    trig = world->trigger[selected_trigger];
    if (trig)
      if(trig->acto.intel.ctm >= 0)
        trig->acto.intel.ctm --;
      else
        show_message("Limit reached", 70);
    else
      show_message("Not selected", 70);
    break;

  case 72: //trgiger tele from +
    trig = world->trigger[selected_trigger];
    next_loctag = LOCATIONTAGMAX +1;
    locid = -1;
    for (int i = 0; i < world->get_location_max(); i++){
      if (world->location[i] && world->location[i]->tag < next_loctag &&
          world->location[i]->tag > trig->acto.intel.frltag ){
            next_loctag = world->location[i]->tag;
            locid = i;
      }
    }
    if (next_loctag <= LOCATIONTAGMAX){
      trig->acto.intel.frltag = next_loctag;
    }
    else
      show_message("No location has a higher tag", 150);
    break;

  case 77: //trigger tele from -
    trig = world->trigger[selected_trigger];
    if (trig->acto.intel.frltag > -1){
      next_loctag = -1;
      locid = -1;
      for (int i = 0; i < world->get_location_max(); i++){
        if (world->location[i] && world->location[i]->tag > next_loctag &&
            world->location[i]->tag < trig->acto.intel.frltag ){
          next_loctag = world->location[i]->tag;
          locid = i;
        }
      }
      if (next_loctag >= -1)
        trig->acto.intel.frltag = next_loctag;
    }
    else
        show_message("No location has no lower tag", 150);
    break;

  case 78: //trgiger tele to +
    trig = world->trigger[selected_trigger];
    next_loctag = LOCATIONTAGMAX +1;
    locid = -1;
    for (int i = 0; i < world->get_location_max(); i++){
      if (world->location[i] && world->location[i]->tag < next_loctag &&
          world->location[i]->tag > trig->acto.intel.toltag ){
            next_loctag = world->location[i]->tag;
            locid = i;
      }
    }
    if (next_loctag <= LOCATIONTAGMAX){
      trig->acto.intel.toltag = next_loctag;
    }
    else
      show_message("No location has a higher tag", 150);
    break;

  case 79:
    trig = world->trigger[selected_trigger];
    if (trig->acto.intel.toltag > -1){
      next_loctag = -1;
      locid = -1;
      for (int i = 0; i < world->get_location_max(); i++){
        if (world->location[i] && world->location[i]->tag > next_loctag &&
            world->location[i]->tag < trig->acto.intel.toltag ){
          next_loctag = world->location[i]->tag;
          locid = i;
        }
      }
      if (next_loctag >= -1)
        trig->acto.intel.toltag = next_loctag;
    }
    else
        show_message("No location has no lower tag", 150);
    break;

  default:
    printf("This button does nothing %d!\n", eventnum);
  }
}

void Editor::show_message(string msg, int time){
  message->str = msg;
  messagetime = time;
}

void Editor::submenu_show(int menu){
  currmenu = menu;
  Trigger *trig;

  switch(menu){
  case MENU_MAIN:
    break;
  case MENU_EDITORMAIN:
    text[1]->str = "Mapwidth:";
    add_to_string<int>(text[1]->str, mo_mapwidth);
    text[2]->str = "Mapheight:";
    add_to_string<int>(text[2]->str, mo_mapheight);
    text[3]->str = "Mapfolder:\n" + mo_folder + "\n\nMapfile:\n" + mo_file;
    text[4]->str = "Background:\n" + mo_bgpic[mo_currbgpic];
    break;
  case MENU_CREATUREMAIN:
    break;
  case MENU_MAP:
    int c, i, pfg, pfb; //count, i, platformgrasscount, platformblackcount
    int cpla, cpom, csbb, coth; //creatures: player, pomgob, sbbflyer, other
    text[26]->str = "**mapsize**\nwidth: ";
    add_to_string<int>(text[26]->str, (int)world->boundary.w);
    text[26]->str += "\nheight: ";
    add_to_string<int>(text[26]->str, (int)world->boundary.h);

    c = 0;
    i = 0;
    pfg = 0;
    pfb = 0;
    while (i < PLATFORM_MAX){
      if (world->platform[i]){
        c++;
        if (dynamic_cast<Grassplatform*>(world->platform[i]))
          pfg ++;
        else
          pfb ++;
      }
      i++;
    }
    text[19]->str = "**platforms**\ntot:(";
    add_to_string<int>(text[19]->str, c);
    text[19]->str += "/";
    add_to_string<int>(text[19]->str, PLATFORM_MAX);
    text[19]->str += ")\ngrass:";
    add_to_string<int>(text[19]->str, pfg);
    text[19]->str += "\nother:";
    add_to_string<int>(text[19]->str, pfb);

    c = 0;
    i = 0;
    cpla = 0;
    cpom = 0;
    csbb = 0;
    coth = 0;
    while (i < CREATURE_MAX){
      if (world->creature[i]){
        c++;
        if (dynamic_cast<Player*>(world->creature[i]))
          cpla ++;
        else if (dynamic_cast<Pomgob*>(world->creature[i]))
          cpom ++;
        else if (dynamic_cast<Sbbflyer*>(world->creature[i]))
          csbb ++;
        else
          coth ++;
      }
      i ++;
    }
    text[21]->str = "**creatures**\ntot:(";
    add_to_string<int>(text[21]->str, c);
    text[21]->str += "/";
    add_to_string<int>(text[21]->str, CREATURE_MAX);
    text[21]->str += ")\nplayer:";
    add_to_string<int>(text[21]->str, cpla);
    text[21]->str += "\npomgob:";
    add_to_string<int>(text[21]->str, cpom);
    text[21]->str += "\nsbbflyer:";
    add_to_string<int>(text[21]->str, csbb);
    text[21]->str += "\nother:";
    add_to_string<int>(text[21]->str, coth);


    break;
  case MENU_PLATFORM:
    if (selected_platform){
      if (dynamic_cast<Grassplatform *>(selected_platform))
        text[24]->str = "Grass selected";
      else
        text[24]->str = "Platform selected";

      text[25]->str = "Cornercoordinates\nUL:(";
      add_to_string<int>(text[25]->str, (int)selected_platform->pos.x);
      text[25]->str = text[25]->str + ",";
      add_to_string<int>(text[25]->str, (int)selected_platform->pos.y);
      text[25]->str = text[25]->str + ")\nUR:(";
      add_to_string<int>(text[25]->str, (int)selected_platform->pos.x +
                         selected_platform->pos.w - 1);
      text[25]->str = text[25]->str + ",";
      add_to_string<int>(text[25]->str, (int)selected_platform->pos.y);
      text[25]->str = text[25]->str + ")\nDL:(";

      add_to_string<int>(text[25]->str, (int)selected_platform->pos.x);
      text[25]->str = text[25]->str + ",";
      add_to_string<int>(text[25]->str, (int)selected_platform->pos.y +
                         selected_platform->pos.h - 1);
      text[25]->str = text[25]->str + ")\nDR:(";
      add_to_string<int>(text[25]->str, (int)selected_platform->pos.x +
                         selected_platform->pos.w - 1);
      text[25]->str = text[25]->str + ",";
      add_to_string<int>(text[25]->str, (int)selected_platform->pos.y +
                         selected_platform->pos.h - 1);

      text[25]->str = text[25]->str + ")\nWidth:";
      add_to_string<int>(text[25]->str, (int)selected_platform->pos.w);
      text[25]->str = text[25]->str + "\nHeight:";
      add_to_string<int>(text[25]->str, (int)selected_platform->pos.h);
    }
    else{
      text[24]->str = "Not selected";
      text[25]->str = "Not selected";
    }
    break;
  case MENU_CREATUREPOMGOB:
    Pomgob *pom;
    pom = dynamic_cast<Pomgob *>(selected_creature);
    if (pom == NULL){
      text[9]->str = "Not selected";
      text[10]->str = "Not selected";
      text[11]->str = "Not selected";
      text[12]->str = "Not selected";
      text[17]->str = "Not selected";
    }
    else{
      text[9]->str = "Pomgob selected\n";

      text[10]->str = "Coord (";
      add_to_string<int>(text[10]->str, pom->pos.x);
      text[10]->str = text[10]->str + ",";
      add_to_string<int>(text[10]->str, pom->pos.y);
      text[10]->str = text[10]->str + ")";

      if (pom->looking_right())
        text[11]->str = "Looking right";
      else
        text[11]->str = "Looking left";

      text[12]->str = "Team: ";
      add_to_string<int>(text[12]->str, pom->team);

      text[17]->str = "Mode: ";
      if (pom->get_stand_still())
        text[17]->str += "Jumping";
      else
        text[17]->str += "Running";
    }
    break;
  case MENU_CREATUREPLAYER:
    Player *p;
    p = dynamic_cast<Player *>(selected_creature);
    if (p == NULL){
      text[5]->str = "Not selected\n";
      text[6]->str = "Not selected\n";
      text[7]->str = "Not selected\n";
      text[8]->str = "Not selected\n";
    }
    else{
      text[5]->str = "Player selected\n";

      text[6]->str = "Coord (";
      add_to_string<int>(text[6]->str, p->pos.x);
      text[6]->str = text[6]->str + ",";
      add_to_string<int>(text[6]->str, p->pos.y);
      text[6]->str = text[6]->str + ")";

      if (p->looking_right())
        text[7]->str = "Looking right";
      else
        text[7]->str = "Looking left";

      text[8]->str = "Team: ";
      add_to_string<int>(text[8]->str, p->team);

      text[27]->str = "Controller: ";
      add_to_string<int>(text[27]->str, p->get_controllerid());

      text[42]->str = "Skin: ";
      add_to_string<int>(text[42]->str, p->get_skin());
    }
    break;
  case MENU_CREATURESBBFLYER:
    Sbbflyer *sbb;
    sbb = dynamic_cast<Sbbflyer *>(selected_creature);
    if (sbb == NULL){
      text[13]->str = "Not selected\n";
      text[14]->str = "Not selected\n";
      text[15]->str = "Not selected\n";
      text[16]->str = "Not selected\n";
    }
    else{
      text[13]->str = "Sbbflyer selected\n";

      text[14]->str = "Coord (";
      add_to_string<int>(text[14]->str, sbb->pos.x);
      text[14]->str = text[14]->str + ",";
      add_to_string<int>(text[14]->str, sbb->pos.y);
      text[14]->str = text[14]->str + ")";

      if (sbb->looking_right())
        text[15]->str = "Looking right";
      else
        text[15]->str = "Looking left";

      text[16]->str = "Team: ";
      add_to_string<int>(text[16]->str, sbb->team);
    }
    break;
  case MENU_CREATUREZOMBO:
    Zombo *zom;
    zom = dynamic_cast<Zombo *>(selected_creature);
    if (zom == NULL){
      text[37]->str = "Not selected\n";
      text[38]->str = "Not selected\n";
      text[39]->str = "Not selected\n";
      text[40]->str = "Not selected\n";
    }
    else{
      text[37]->str = "Zombo selected\n";

      text[38]->str = "Coord (";
      add_to_string<int>(text[38]->str, zom->pos.x);
      text[38]->str = text[38]->str + ",";
      add_to_string<int>(text[38]->str, zom->pos.y);
      text[38]->str = text[38]->str + ")";

      if (zom->looking_right())
        text[39]->str = "Looking right";
      else
        text[39]->str = "Looking left";

      text[40]->str = "Team: ";
      add_to_string<int>(text[40]->str, zom->team);
    }
    break;
  case MENU_PLATFORM_NONE:
    break;
  case MENU_PLATFORM_GRASS:
    break;
  case MENU_TRIGGER:
    int triggers_tot;
    triggers_tot = 0;
    for(int i=0; i <= TRIGGER_MAX; i++)
      if (world->trigger[i])
        triggers_tot ++;

    text[54]->visible = false;
    //victory -->
    text[58]->visible = false;
    //intern teleport -->
    text[60]->visible = false;
    button[100]->visible = false;
    button[106]->visible = false;
    text[61]->visible = false;
    button[107]->visible = false;
    button[108]->visible = false;
    text[62]->visible = false;
    button[109]->visible = false;
    button[110]->visible = false;
    text[63]->visible = false;
    text[64]->visible = false;
    button[111]->visible = false;
    text[65]->visible = false;
    button[112]->visible = false;
    text[66]->visible = false;
    button[113]->visible = false;
    text[67]->visible = false;
    button[114]->visible = false;
    text[68]->visible = false;
    button[115]->visible = false;

    if (selected_trigger == -1){
      text[44]->str = "Not sel. Tot ";
      add_to_string<int>(text[44]->str, triggers_tot);
      text[46]->visible = false;
      text[48]->visible = false;
      text[45]->visible = false;
      text[47]->visible = false;
      text[57]->visible = false;

      button[85]->visible = false;
      button[86]->visible = false;
      button[87]->visible = false;
      button[88]->visible = false;
      button[89]->visible = false;
      button[103]->visible = false;
      button[104]->visible = false;
    }
    else{
      trig = world->trigger[selected_trigger];
      text[44]->str = "Trigger ";
      add_to_string<int>(text[44]->str, selected_trigger);
      text[44]->str += ", tot ";
      add_to_string<int>(text[44]->str, triggers_tot);

      button[85]->visible = true;
      text[45]->visible = true;
      text[46]->visible = true;
      text[47]->visible = true;
      text[48]->visible = true;
      text[57]->visible = true;

      button[86]->visible = true;
      button[87]->visible = true;
      button[88]->visible = true;
      button[89]->visible = true;
      button[103]->visible = true;
      button[104]->visible = true;

      switch (trig->cond){
      case TC_NONE: text[46]->str = "None" ;
        break;
      case TC_ENTER_LOCATION: text[46]->str = "Enters location" ;
        text[58]->visible = true;

        for (int i = 0; i < world->get_location_max(); i++){
          if (world->location[i] &&
              world->location[i]->act_trigger == trig->tag)
            trig->condo.entloc.ltag = world->location[i]->tag;
        }
        if (trig->condo.entloc.ltag > 0){
          text[58]->str = "Activated by\nlocation with\nloctag ";
          add_to_string<int>(text[58]->str, trig->condo.entloc.ltag);
        }
        else
          text[58]->str = "Activated by no\nlocation";
        break;
      case TC_ALL_ENEMIES_DEAD: text[46]->str = "Enemies dead" ;
        break;
      }

      text[54]->visible = false;
      text[60]->visible = false;
      button[100]->visible = false;
      button[106]->visible = false;
      switch (trig->act){
      case TA_NONE: text[48]->str = "None" ;
        break;
      case TA_VICTORY: text[48]->str = "Victory" ;
        text[54]->visible = true;
        break;
      case TA_DEFEAT: text[48]->str = "Defeat" ;
        break;
      case TA_INTERN_TELEPORT: text[48]->str = "Intern teleport";
        trig = world->trigger[selected_trigger];
        text[60]->visible = true;
        text[60]->visible = true;
        button[100]->visible = true;
        button[106]->visible = true;
        text[61]->visible = true;
        button[107]->visible = true;
        button[108]->visible = true;
        text[62]->visible = true;
        button[109]->visible = true;
        button[110]->visible = true;
        text[63]->visible = true;
        text[64]->visible = true;
        button[111]->visible = true;
        text[65]->visible = true;
        button[112]->visible = true;
        text[66]->visible = true;
        button[113]->visible = true;
        text[67]->visible = true;
        button[114]->visible = true;
        text[68]->visible = true;
        button[115]->visible = true;

        if (trig->acto.intel.frltag == -1)
          text[60]->str = "from anywhere";
        else{
          text[60]->str = "from loctag ";
          add_to_string<int>(text[60]->str, trig->acto.intel.frltag);
        }

        if (trig->acto.intel.toltag == -1)
          text[61]->str = "to nowhere(remove)";
        else{
          text[61]->str = "to loctag ";
          add_to_string<int>(text[61]->str, trig->acto.intel.toltag);
        }

        if (trig->acto.intel.ctm == -1)
          text[62]->str = "in any team";
        else{
          text[62]->str = "in team ";
          add_to_string<int>(text[62]->str, trig->acto.intel.ctm);
        }

      button[111]->src.y = (trig->acto.intel.cpla ? 59 : 74);
      button[111]->srcdown.y = (trig->acto.intel.cpla ? 59 : 74);

      button[112]->src.y = (trig->acto.intel.cpom ? 59 : 74);
      button[112]->srcdown.y = (trig->acto.intel.cpom ? 59 : 74);

      button[113]->src.y = (trig->acto.intel.csbb ? 59 : 74);
      button[113]->srcdown.y = (trig->acto.intel.csbb ? 59 : 74);

      button[114]->src.y = (trig->acto.intel.czom ? 59 : 74);
      button[114]->srcdown.y = (trig->acto.intel.czom ? 59 : 74);

      button[115]->src.y = (trig->acto.intel.cblu ? 59 : 74);
      button[115]->srcdown.y = (trig->acto.intel.cblu ? 59 : 74);
      break;
      }

      if (trig->tag == 0)
        text[57]->str = "Has no tag";
      else{
        text[57]->str = "TrigTag: ";
        add_to_string<int>(text[57]->str, trig->tag);
      }
    }
    break;
  case MENU_LOCATION:
    text[30]->str = "Collision\nreqiures\nfull contact";
    text[32]->str = "Activate for\n";

    button[60]->visible = selected_location;
    text[29]->visible = selected_location;
    text[56]->visible = selected_location;
    button[102]->visible = selected_location;
    button[101]->visible = selected_location;
    button[61]->visible = selected_location;
    text[30]->visible = selected_location;
    button[62]->visible = selected_location;
    text[31]->visible = selected_location;
    button[63]->visible = selected_location;
    button[64]->visible = selected_location;
    text[32]->visible = selected_location;
    button[65]->visible = selected_location;
    button[66]->visible = selected_location;
    text[33]->visible = selected_location;
    text[34]->visible = selected_location;
    button[67]->visible = selected_location;
    text[35]->visible = selected_location;
    button[68]->visible = selected_location;
    text[36]->visible = selected_location;
    button[69]->visible = selected_location;
    text[43]->visible = selected_location;
    button[81]->visible = selected_location;

    if (selected_location){
      Location *loc = selected_location;

      text[28]->str = "Location selected\n";
      text[29]->str = "UL:";
      add_to_string<int>(text[29]->str, loc->pos.x);
      text[29]->str += "x";
      add_to_string<int>(text[29]->str, loc->pos.y);
      text[29]->str += "\nDR:";
      add_to_string<int>(text[29]->str, loc->pos.x + loc->pos.w - 1);
      text[29]->str += "x";
      add_to_string<int>(text[29]->str, loc->pos.y + loc->pos.h - 1);
      if (loc->totalcol)
        button[62]->src.y = 2;
      else
        button[62]->src.y = 27;
      button[62]->srcdown.y = button[62]->src.y;

      if (loc->act_team == -1)
        text[32]->str += "any team";
      else{
        text[32]->str += "team ";
        add_to_string<int>(text[32]->str, loc->act_team);
      }

      if (selected_location->tag == 0)
        text[56]->str = "No tag used";
      else{
        text[56]->str = "LocTag ";
        add_to_string<int>(text[56]->str, selected_location->tag);
      }

      if (selected_location->act_trigger == 0)
        text[31]->str = "Activate trigger\ndisabled";
      else{
        text[31]->str = "Activate trigger\nwith TrigTag ";
        add_to_string(text[31]->str, selected_location->act_trigger);
      }
      button[67]->src.y = (selected_location->act_player   ? 59 : 74);
      button[67]->srcdown.y = (selected_location->act_player   ? 59 : 74);
      button[68]->src.y = (selected_location->act_pomgob   ? 59 : 74);
      button[68]->srcdown.y = (selected_location->act_pomgob   ? 59 : 74);
      button[69]->src.y = (selected_location->act_sbbflyer ? 59 : 74);
      button[69]->srcdown.y = (selected_location->act_sbbflyer ? 59 : 74);
      button[81]->src.y = (selected_location->act_zombo    ? 59 : 74);
      button[81]->srcdown.y = (selected_location->act_zombo    ? 59 : 74);
    }
    else{
      text[28]->str = "Nothing selected";
    }

    if (settings.show_locations)
      button[78]->src.y = 2;
    else
      button[78]->src.y = 27;
    button[78]->srcdown.y = button[78]->src.y;
    break;
  case MENU_CREATUREBLUBBER:
    Blubber *blu;
    blu = dynamic_cast<Blubber *>(selected_creature);
    text[50]->visible = blu != NULL;
    text[51]->visible = blu != NULL;
    text[52]->visible = blu != NULL;
    text[53]->visible = blu != NULL;
    button[92]->visible = blu != NULL;
    button[93]->visible = blu != NULL;
    button[94]->visible = blu != NULL;
    button[95]->visible = blu != NULL;
    button[96]->visible = blu != NULL;
    button[97]->visible = blu != NULL;

    if (blu == NULL){
      text[49]->str = "Not selected :(";
    }
    else{
      text[49]->str = "Blubber selected";

      if (blu->looking_right())
        text[51]->str = "looking right";
      else
        text[51]->str = "looking left";

      text[52]->str = "team: ";
      add_to_string<int>(text[52]->str, blu->team);
      text[50]->str = "coord: (";
      add_to_string<int>(text[50]->str, blu->pos.x);
      text[50]->str += ",";
      add_to_string<int>(text[50]->str, blu->pos.y);
      text[50]->str += ")";

      if (blu->get_falling())
        text[53]->str = "Falling";
      else
        text[53]->str = "Going to roof";

    }
    break;
  default:
    printf("unknown menu: %d\n", menu);
    exit(1);
  }
}

void Editor::handle_input(){
  Pomgob * pom;
  Player * pla;
  Sbbflyer * sbb;
  Zombo *zom;
  Blubber *blu;
  if (keydown[keymap[EKEY_SHIFT]] == game->get_time())
    toggle_menu();

  if (keydown[keymap[EKEY_CLEARMARK]] == game->get_time()){
    marked_creature = NULL;
    marked_platform = NULL;
  }

  if (keydown[keymap[EKEY_GRAVITY]] == game->get_time()){
    gravity = not gravity;
    printf("gravity: %d\n", gravity);
  }

  if (keydown[keymap[EKEY_EXIT]] == game->get_time())
    quit = true;

  if (keydown[keymap[EKEY_COLLISIONMODE]] == game->get_time()){
    settings.collision_mode = not settings.collision_mode;
    settings.show_locations = settings.collision_mode;
    printf("collision mode: ");
    printf((settings.collision_mode?"true\n":"false\n"));
  }

  if (keydown[keymap[EKEY_LEFT]] > keyup[keymap[EKEY_LEFT]]){
    if (look.x + look.w >= game->world->boundary.x + 100){
      look.x -= scrollspeed;
      mapmousex -= scrollspeed;
    }
  }
  if (keydown[keymap[EKEY_RIGHT]] > keyup[keymap[EKEY_RIGHT]]){
    if (look.x<=
        game->world->boundary.x + game->world->boundary.w - 100){
      look.x += scrollspeed;
      mapmousex += scrollspeed;
    }
  }
  if (keydown[keymap[EKEY_UP]] > keyup[keymap[EKEY_UP]]){
    if (look.y + look.h >= game->world->boundary.y + 100){
      look.y -= scrollspeed;
      mapmousey -= scrollspeed;
    }
  }
  if (keydown[keymap[EKEY_DOWN]] > keyup[keymap[EKEY_DOWN]]){
    if (look.y <= game->world->boundary.y +
        game->world->boundary.h - 100){
      look.y += scrollspeed;
      mapmousey += scrollspeed;
    }
  }

  if (mousebuttons & SDL_BUTTON_LEFT){
    //the button is down
    if (mouselastdown == game->get_time()){
      //button was downed just now (mouse down)
      bool old_selection = (selected_creature || selected_platform ||
                            selected_location);
      marked_creature = NULL;
      marked_platform = NULL;
      marked_location = NULL;
      if (worldmouse){ /* worldmouse is true if mouse was downed on
                          cliprectworld, defined in edtior.h */
        resizing = keydown[keymap[EKEY_RESIZEPLATFORM]] >
          keyup[keymap[EKEY_RESIZEPLATFORM]];
        Square *s;
        s = game->world->grid->examine(mapmousex, mapmousey);
        if (s != NULL){
          Node<Creature> *c;
          Node<Platform> *p;
          Node<Location> *l;

          if (settings.show_locations){
            l = s->locations;
            while(l != NULL){
              if (mapmousex >= l->link->pos.x &&
                  mapmousex <= l->link->pos.x + l->link->pos.w &&
                  mapmousey >= l->link->pos.y &&
                  mapmousey <= l->link->pos.y + l->link->pos.h){
                marked_location = l->link;
                downrect = l->link->pos;
              }
              l = l->next;
            }
          } else
            l = NULL;
          if (marked_location == NULL){
            c = s->creatures;
            while(c != NULL){
              if (mapmousex >= c->link->pos.x &&
                  mapmousex <= c->link->pos.x + c->link->pos.w &&
                  mapmousey >= c->link->pos.y &&
                  mapmousey <= c->link->pos.y + c->link->pos.h){
                marked_creature = c->link;
                gravity = false;
              }
              c = c->next;
            }
            if (c == NULL){
              p = s->platforms;
              while(p != NULL){
                if (mapmousex >= p->link->pos.x &&
                    mapmousex <= p->link->pos.x + p->link->pos.w &&
                    mapmousey >= p->link->pos.y &&
                    mapmousey <= p->link->pos.y + p->link->pos.h){
                  marked_platform = p->link;
                  downrect = p->link->pos;
                }
                p = p->next;
              }
            }
          }


          selected_platform = NULL;
          selected_creature = NULL;
          selected_location = NULL;
          if (marked_location)
            selected_location = marked_location;
          else if (marked_creature)
            selected_creature = marked_creature;
          else if (marked_platform)
            selected_platform = marked_platform;

          /* Opens the correct menu depending on what was downed on */
          pla = dynamic_cast<Player *>(marked_creature);
          pom = dynamic_cast<Pomgob *>(marked_creature);
          sbb = dynamic_cast<Sbbflyer *>(marked_creature);
          zom = dynamic_cast<Zombo *>(marked_creature);
          blu = dynamic_cast<Blubber *>(marked_creature);
          if (selected_location)
            submenu_show(MENU_LOCATION);
          else if (pla){
            defplalr = pla->looking_right();
            defplateam = pla->team;
            submenu_show(MENU_CREATUREPLAYER);
          }
          else if (pom){
            defpomlr = pom->looking_right();
            defpomteam = pom->team;
            defpomjump = not pom->get_stand_still();
            submenu_show(MENU_CREATUREPOMGOB);
          }
          else if (sbb){
            defsbblr = sbb->looking_right();
            defsbbteam = sbb->team;
            submenu_show(MENU_CREATURESBBFLYER);
          }
          else if (zom){
            defzomlr = zom->looking_right();
            defzomteam = zom->team;
            submenu_show(MENU_CREATUREZOMBO);
          }
          else if (blu){
            /* TODO - fix default value for blubber!!
               defblulr = blu->looking_right();
               defbluteam = blu->team;
               defblugoup = blu->get_goingup();
            */
            submenu_show(MENU_CREATUREBLUBBER);
          }
          else if (marked_platform)
            submenu_show(MENU_PLATFORM);
          else if (old_selection)
            submenu_show(MENU_MAIN);
        }
      }
      else{
        buttondownindex = -1;
        for (int i = 0; i < MENUBUTTONSTOT; i++)
          if (button[i] && button[i]->visible){
            if (button[i]->pos.x <= mousex &&
                button[i]->pos.x + button[i]->pos.w >= mousex &&
                button[i]->pos.y <= mousey &&
                button[i]->pos.y + button[i]->pos.h >= mousey){
              if (button[i]->menu == currmenu){
                buttondownindex = i;
                button[i]->down = true;
                i = MENUBUTTONSTOT; //break
              }
            }
          }
      }
    }
    else {
      //the button has been down longer than 1 frame
      if (marked_location){
        if (resizing){
          if (mapmousexdown <= downrect.w + mapmousex)
            marked_location->pos.w = downrect.w + mapmousex - mapmousexdown;
          else
            marked_location->pos.w = 3;

          if (mapmouseydown <= downrect.h + mapmousey)
            marked_location->pos.h = downrect.h + mapmousey - mapmouseydown;
          else
            marked_location->pos.h = 3;
        }
        else{
          marked_location->pos.x = downrect.x + mapmousex - mapmousexdown;
          marked_location->pos.y = downrect.y + mapmousey - mapmouseydown;
        }
      }
      else if (marked_creature){
        marked_creature->move_x(mapmousex - lastmapmousex);
        marked_creature->move_y(mapmousey - lastmapmousey);
      }
      else if (marked_platform){
        if (resizing){
          marked_platform->set_pos( downrect.x, downrect.y,
                           downrect.w + mapmousex - mapmousexdown,
                           downrect.h + mapmousey - mapmouseydown);
        }
        else
          marked_platform->set_pos( downrect.x + mapmousex - mapmousexdown,
                                    downrect.y + mapmousey - mapmouseydown,
                                    downrect.w, downrect.h);
      }
    }
  }//fi leftbutton down
  else if ((lastmousebuttons & SDL_BUTTON_LEFT) &&
           not (mousebuttons & SDL_BUTTON_LEFT)){
    //the button was just released
    if (buttondownindex != -1){
      Button *bd = button[buttondownindex];
      bd->down = false;
      buttondownindex = -1;
      if (bd->pos.x <= mousex &&
          bd->pos.x + bd->pos.w >= mousex &&
          bd->pos.y <= mousey &&
          bd->pos.y + bd->pos.h >= mousey){
        button_event(bd->event);
      }
    }
    if (marked_creature != NULL){
      game->world->unregister_creature(marked_creature);
      cib(marked_creature);
      game->world->register_creature(marked_creature);
      marked_creature = NULL;
    }
    if (marked_platform != NULL){
      game->world->unregister_platform(marked_platform);
      game->world->generate_graphics_surroundings(downrect);
      game->world->put_in_boundary(marked_platform);
      game->world->register_platform(marked_platform);
      game->world->generate_graphics_surroundings(marked_platform->pos);
      marked_platform = NULL;
    }
    if (marked_location != NULL){
      game->world->unregister_location(marked_location);
      game->world->put_in_boundary(marked_location);
      game->world->register_location(marked_location);
      marked_location = NULL;
    }
    submenu_show(currmenu);
  }
}

void Editor::cib(Creature *c){
  if (c->pos.x < game->world->boundary.x)
    c->move_x(game->world->boundary.x - c->pos.x);
  if (c->pos.y < game->world->boundary.y)
    c->move_y(game->world->boundary.y - c->pos.y);
  if (c->pos.y + c->pos.h > game->world->boundary.y + game->world->boundary.h)
    c->move_y(game->world->boundary.y + game->world->boundary.h - c->pos.y -
              c->pos.h - 2);
  if (c->pos.x + c->pos.w > game->world->boundary.x + game->world->boundary.w)
    c->move_x(game->world->boundary.x + game->world->boundary.w - c->pos.x -
              c->pos.w - 1);
}

void Editor::get_input(){
  SDL_Event event;
  lastmapmousex = mapmousex;
  lastmapmousey = mapmousey;
  lastmousebuttons = mousebuttons;
  while( SDL_PollEvent( &event ) ){
    switch( event.type ){
    case SDL_KEYDOWN:
      keydown[event.key.keysym.scancode] = (int)game->get_time();
      //        printf("frame: %d keydown: %d\n", (int)game->get_time(),
      //         event.key.keysym.scancode);
      break;

    case SDL_KEYUP:
      keyup[event.key.keysym.scancode] = (int)game->get_time();
      break;

    case SDL_QUIT:
      quit = true;
      break;

    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
      mousex = event.button.x;
      mousey = event.button.y;
      if (event.button.state == SDL_PRESSED){
        mouselastdown = game->get_time();
        if (mousex >= cliprectworld.x && mousey >= cliprectworld.y &&
            mousex <= cliprectworld.x + cliprectworld.w &&
            mousey <= cliprectworld.y + cliprectworld.h){
          //the button was pressed on the cliprectworld
          worldmouse = true;
        }
        else
          worldmouse = false;
      }
      mapmousexdown = look.x + mousex;
      mapmouseydown = look.y + mousey;
      mapmousex = mapmousexdown;
      mapmousey = mapmouseydown;
      mousebuttons = mousebuttons ^ event.button.button;
      break;
    case SDL_MOUSEMOTION:
      mousex = event.motion.x;
      mousey = event.motion.y;
      if (worldmouse && mousebuttons){
        if (mousex < cliprectworld.x)
          mousex = cliprectworld.x;
        if (mousey < cliprectworld.y)
          mousey = cliprectworld.y;
        if (mousex > cliprectworld.x + cliprectworld.w - 1)
          mousex = cliprectworld.x + cliprectworld.w - 1;
        if (mousey > cliprectworld.y + cliprectworld.h - 1)
          mousey = cliprectworld.y + cliprectworld.h - 1;
      }
      mapmousex = look.x + mousex;
      mapmousey = look.y + mousey;
      mousebuttons = event.motion.state;
      break;

    default:
      break;
    }
  }
  handle_input();
}

void Editor::paint_message(SDL_Surface **screen){
  static int col = 0;
  static int dir = 1;
  static int dir_speed = 3;
  if (col > 60){
    col = 60;
    dir = -1;
  }
  else if (col < 0){
    col = 0;
    dir = 1;
  }
  col = col + dir * dir_speed;

  if (messagetime == 0)
    return;
  else
    messagetime --;

  SDL_Rect target, border;
  Uint16 w, h;
  message->calc_size(w, h);
  target.w = w;
  target.h = h;
  target.x = message->x - 10;
  target.y = message->y - 5;
  target.w = target.w + 20;
  target.h = target.h + 10;

  border = target;
  border.x = border.x - 2;
  border.y = border.y - 2;
  border.w = border.w + 4;
  border.h = border.h + 4;

  SDL_FillRect(*screen, &border,
               SDL_MapRGB((*screen)->format,
                          0, 0, 0));

  SDL_FillRect(*screen, &target,
               SDL_MapRGB((*screen)->format,
                          0, 90 + col, 120 + col));

  message->paint(screen);
}

void Editor::paint_menu(SDL_Surface **screen){
  if (not menuvisible)
    return;

  SDL_Rect target;
  target.x = cliprectmenu.x;
  target.y = cliprectmenu.y;
  target.w = cliprectmenu.w;
  target.h = cliprectmenu.h;

  SDL_FillRect(*screen, &target,
               SDL_MapRGB((*screen)->format,
                          255, 255, 180));

  for(int i = 0; i < MENUBUTTONSTOT; i++){
    if (button[i] != NULL && currmenu == button[i]->menu
        && button[i]->visible) {
      button[i]->paint(screen);
    }
  }
  for(int i = 0; i < MENUTEXTTOT; i++){
    if (text[i] != NULL && currmenu == text[i]->menu && text[i]->visible) {
      text[i]->paint(screen);
    }
  }
}

int Editor::get_next_ltag(int starttag, bool next){
  bool freeslot[LOCATIONTAGMAX];
  int idiff = (next?1:-1);
  for (int i = 0; i < LOCATIONTAGMAX; i++)
    freeslot[i] = true;

  int tag;
  for(int i = 0; i < LOCATION_MAX; i++){
    if (world->location[i]){
      tag = world->location[i]->tag;
      if (tag > 0)
        freeslot[tag] = false;
    }
  }

  for(int i = starttag + idiff; i < LOCATIONTAGMAX && i > 0; i+=idiff)
    if (freeslot[i])
      return i;

  return 0;
}

int Editor::get_next_ttag(int starttag, bool next){
  bool freeslot[TRIGGERTAGMAX];
  int idiff = (next?1:-1);
  for (int i = 0; i < TRIGGERTAGMAX; i++)
    freeslot[i] = true;

  int tag;
  for(int i = 0; i < TRIGGER_MAX; i++){
    if (world->trigger[i]){
      tag = world->trigger[i]->tag;
      if (tag > 0)
        freeslot[tag] = false;
    }
  }

  for(int i = starttag + idiff; i < TRIGGERTAGMAX && i > 0; i+=idiff)
    if (freeslot[i])
      return i;

  return 0;
}

int Editor::get_next_trigger(int starttag, bool next){
  int closesttag = (next?TRIGGERTAGMAX:0);
  int id_of_tag = -1; //the id of the best tag sofar
  int tag;
  for(int i = 0; i < TRIGGER_MAX; i++){
    if (world->trigger[i]){
      tag = world->trigger[i]->tag;
      if ((next && tag > starttag && tag < closesttag)
          || (not next && tag < starttag && tag > closesttag)){
        closesttag = tag;
        id_of_tag = i;
      }

    }
  }

  if (closesttag > -1 || closesttag < TRIGGERTAGMAX)
    return id_of_tag;

  return -1;
}
