#include "graphics.h"
#include "map.h"
#include "controller.h"
#include "SDL/SDL.h"
#include "physics.h"
#include "game.h"
#include "editor.h"

#include "settings.h"

int mainloop( Map &world, SDL_Surface **screen );

int main(int argc, char **argw){
  string mapfolder, mapfile;
  SDL_Surface *screen = NULL;

  settings.load_settings("./gobblashx.conf");

  Map world;

  SDL_Init( SDL_INIT_VIDEO );
  prepare_graphics( &screen );

  cliprectworld.w = settings.screen_w;
  cliprectmenu.x = cliprectworld.x + cliprectworld.w;
  cliprectmenu.w = settings.screen_w - cliprectmenu.x;
  cliprectmenu.y = cliprectworld.y;
  cliprectmenu.h = cliprectworld.h;

  if (argc == 1){
    mapfolder = "maps/";
    mapfile = "first.map";
  }
  else if (argc == 2){
    int ls = -1; //lastslash
    int ts = 0; //testslash
    string arg = argw[1];
    while( arg[ts] != '\0'){
      ts ++;
      if (arg[ts] == '/')
        ls = ts;
    }
    if (ls == -1){
      mapfolder = "./";
      mapfile = arg;
    }
    else{
      mapfolder = arg.substr(0, ls + 1);
      mapfile = arg.substr(ls +1, arg.size() - ls);
    }

    printf("mapfolder:%s\n", mapfolder.c_str());
    printf("mapfile:%s\n", mapfile.c_str());
  }
  else {
    printf("Wrong number of arguments!\n");
    exit(1);
  }
  editor = new Editor(&screen, world, mapfolder, mapfile);
  game = new Game;
  game->set_aitools(&world);
  if (world.load_mapfile((mapfolder + mapfile).c_str(), &screen,
                         game->aitools)){
    printf("Failed to load map!\n");
    exit(1);
  }

  game->set_world(&world);

  settings.follow_hero = false;
  mainloop( world, &screen );

  end_graphics( &screen );
  world.free_map();
  SDL_Quit();

  return 0;
}

int mainloop( Map &world, SDL_Surface **screen )
{
  int i;

  while(not editor->quit)
    {
      game->wait_until_next_frame();
      if (editor->gamecontrollers == true){
        game->get_input();
      }
      else{
        editor->get_input();
      }
      if (not game->paused){
        for(i = 0; i <= world.get_fxfield_max(); i++){
          if (world.fxfield[i] != NULL){
            world.unregister_fxfield( world.fxfield[i] );
            world.fxfield[i]->lifetime --;
            if (world.fxfield[i]->lifetime < 0){
              delete world.fxfield[i];
              world.fxfield[i] = NULL;
            }
            else{
              world.register_fxfield( world.fxfield[i] );
            }
          }
        }
        if (editor->creatures_think){
          for(i = 0; i <= world.get_creature_max(); i++){
            if (not world.creature[i]->dead){
              if (world.creature[i]->safe > 0)
                world.creature[i]->safe --;
              world.creature[i]->think();
            }
          }
        }
        if (editor->gravity){
          gravity(world);
          physics(world);
        }
        checkfxfields(world);
      } // fi not game->paused

      SDL_SetClipRect(*screen, &cliprectmenu);

      editor->paint_menu( screen );
      paint_world( world, screen );
      editor->paint_message( screen );
      SDL_Flip( *screen );

      game->next_frame();
    }

  printf("fps:%f\n", game->get_fps());
  return 0;
}
