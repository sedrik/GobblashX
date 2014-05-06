#include "graphics.h"
#include "map.h"
#include "controller.h"
#include "SDL/SDL.h"
#include "physics.h"
#include "game.h"
#include "settings.h"

#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif


Map world;
SDL_Surface *screen = NULL;

int mainloop();
void iter();

int main(int argc, char *argv[])
{

  settings.load_settings("./gobblashx.conf");

  game = new Game;

  SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK );
  prepare_graphics( &screen );

  game->set_aitools(&world);
  if(argc > 1){ /* Load map argument */
    if (world.load_mapfile(argv[1], &screen, game->aitools)){
      printf("Failed to load map!\n"
             "Was it on the form maps/map.map?\n");
      exit(1);
    }
  }else{ /* If no map argument try to load standard map */
    if (world.load_mapfile("maps/first.map", &screen, game->aitools)){
      printf("Failed to load map!\n"
             "Have you made a symlink called maps/first.map to the first map?\n"
             );
      exit(1);
    }
  }
  game->set_world(&world);
  mainloop();
  end_graphics( &screen );
  world.free_map();
  delete game;
  SDL_Quit();

  return 0;
}

int mainloop()
{
  //  settings.show_locations = true;

#ifdef EMSCRIPTEN
  // void emscripten_set_main_loop(void (*func)(), int fps, int simulate_infinite_loop);
  emscripten_set_main_loop(iter, 60, 1);
#else
  while(not game->quit && game->playing())
  {
    game->wait_until_next_frame();
    iter();
  }

  printf("fps:%f\n", game->get_fps());

  if (game->get_state() == GAME_COMPLETED){
    printf("Yaaay! game was completed!\n");
    printf("Time taken: %d frame(s) (%d minutes and %d seconds)\n",
           game->get_time(),
           game->get_time() / 3600,
           game->get_time() / 60);
  }
  else if(game->get_state() == GAME_PLAYING)
    printf("Player ended game\n");
  else if(game->get_state() == GAME_OVER)
    printf("Game over...\n");
#endif
    return 0;
}

void iter(){

  game->get_input();

  if (not game->paused){
    gravity(world);
    physics(world);

    int i;
    for(i = 0; i < world.get_fxfield_max(); i++){
      if (world.fxfield[i] != NULL){
        world.unregister_fxfield( world.fxfield[i] );
        world.fxfield[i]->lifetime --;
        if (world.fxfield[i]->lifetime < 0){
          delete world.fxfield[i];
          world.fxfield[i] = NULL;
        }
        else{
          //think - can fxfields think?
          world.register_fxfield( world.fxfield[i] );
        }
      }
    }

    for(i = 0; i <= world.get_creature_max(); i++){
      if (world.creature[i] != NULL){
        if (not world.creature[i]->dead){
          if (world.creature[i]->safe > 0)
            world.creature[i]->safe --;
          world.creature[i]->think();
        }
      }
    }
  } // fi not game.paused

  checkfxfields(world);

  checklocations(game->aitools, world);

  paint_world( world, &screen );

  SDL_Flip(screen);

  game->next_frame();
}
