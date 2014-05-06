#include "settings.h"
#include <fstream>
#include <string>

#include "filefunctions.h"

Settings settings;

Settings::Settings(){
  air_break = 0.1;
  gravity_y = 0.2;
  collision_mode = false;
  follow_hero = true;
  show_keysyms = false;
  show_locations = false;
  debug = false;
  showhp = true;
}

void Settings::load_settings( const char filename[] ){
  std::string row, word;
  int rownum = 0;
  std::ifstream configfile(filename);
  if (not configfile){
    printf("Failed to load settings from file \"%s\"\n", filename);
    exit(1);
  }
  settings.screen_flags = 0;

  while(not configfile.eof()){
    word = skipcomments(configfile, row, rownum);
    if (word == "screenres"){
      word = first_word(row);
      if (safe_strtol(word, settings.screen_w))
        printf("load_settings: file %s:%d - Expected number and got %s\n",
               filename, rownum, word.c_str());

      word = first_word(row);
      if (safe_strtol(word, settings.screen_h))
        printf("load_settings: file %s:%d - Expected number and got %s\n",
               filename, rownum, word.c_str());

      word = first_word(row);
      if (safe_strtol(word, settings.screen_bpp))
        printf("load_settings: file %s:%d - Expected number and got %s\n",
               filename, rownum, word.c_str());
    }
    else if (word == "fullscreen"){
      word = first_word(row);
      if (word == "yes" || word == "on" || word == "1")
        settings.screen_flags |= SDL_FULLSCREEN;
      else
        settings.screen_flags &= ~SDL_FULLSCREEN;
    }
    else if (word == "doublebuf"){
      word = first_word(row);
      if (word == "yes" || word == "on" || word == "1")
        settings.screen_flags |= SDL_DOUBLEBUF;
      else
        settings.screen_flags &= ~SDL_DOUBLEBUF;
    }
    else if (word == "asyncblit"){
      word = first_word(row);
      if (word == "yes" || word == "on" || word == "1")
        settings.screen_flags |= SDL_ASYNCBLIT;
      else
        settings.screen_flags &= ~SDL_ASYNCBLIT;
    }
    else if (word == "hardwaremem"){
      word = first_word(row);
      if (word == "yes" || word == "on" || word == "1"){
        settings.screen_flags |= SDL_HWSURFACE;
        settings.screen_flags &= ~SDL_SWSURFACE;
      }
      else{
        settings.screen_flags |= SDL_SWSURFACE;
        settings.screen_flags &= ~SDL_HWSURFACE;
      }
    }
    else if (word == "controller"){
      int conid;
      word = first_word(row);
      if (safe_strtol(word, conid)){
        printf("load_settings: file %s:%d - Expected number and got %s\n",
               filename, rownum, word.c_str());
        exit(1);
      }
      else if (conid < 0 || conid >= TOT_CONTROLLERS){
        printf("load_settings: file %s:%d - Number was %s, but must be"
               " >= than 0 and < %d",
               filename, rownum, word.c_str(), TOT_CONTROLLERS);
        exit(1);
      }
      conkeymap[conid].load_keymap(configfile, filename, rownum);
    }
  }
  configfile.close();
}
