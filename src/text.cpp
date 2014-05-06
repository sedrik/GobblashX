#include "text.h"
#include "graphics.h"

SDL_Surface *Text::tilesurf = NULL;
int Text::count = 0;

Text::Text(SDL_Surface **screen, int ix, int iy, const char *def, int imenu){
  str = def;
  x = ix;
  y = iy;
  visible = true;
  menu = imenu;
  if (count == 0 && tilesurf == NULL){
    tilesurf = load_optimized_pic( "graphics/font.bmp", screen,
                              255, 255, 255 );
  }
  count ++;
}

Text::~Text(){
  count --;
  if (count == 0)
    SDL_FreeSurface( tilesurf );
}

void Text::paint( SDL_Surface **screen ){
  SDL_Rect src, target;
  target.x = x;
  target.y = y;
  for (unsigned int i = 0; i < str.size(); i++){
    if (str[i] == '\n'){
      target.x = x;
      target.y = target.y + 13;
    }
    else if (str[i] == ' ')
      target.x = target.x + 11;
    else{
      target.w = 10;
      target.h = 10;
      src.x = getpos(str[i]);
      src.h = 10;
      src.w = 10;
      src.y = 0;
      if (SDL_BlitSurface(tilesurf, &src, *screen, &target)){
        printf("Failed to blit letter\n");
        exit(1);
      }
      target.x = target.x + 11;
    }
  }
}

void Text::calc_size(Uint16 &w, Uint16 &h){
  int i = 0;
  int lines = 1;
  int maxchars = 0, chars = 0; //chars in a line
  while (str[i] != '\0'){
    if (str[i] == '\n'){
      chars = 0;
      lines ++;
    }
    else{
      chars ++;
      if (chars > maxchars)
        maxchars = chars;
    }
    i++;
  }

  h = 13 * lines - 3;
  w = 11 * maxchars - 1;
}

int Text::getpos(char letter){
  if (letter >= 97 && letter <= 122)
    letter -= 'a'-'A';
  if (letter >= 33 && letter <= 96)
    return (letter - 33) * 10;
  else if (letter >= 123 && letter <= 125)
    return 640 + (letter - 123)*10;
  else
    return 300;
}
