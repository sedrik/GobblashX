#include "grassplatform.h"
#include "graphics.h"
#include "settings.h"

SDL_Surface *Grassplatform::tilesurf = NULL;
int Grassplatform::grassplatform_count = 0;

bool _px_in_rect(const int x, const int y, const SDL_Rect &r){
  return (x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h);
}

void Grassplatform::generate_graphics(){
  if(pos.w % 20 != 0 || pos.h % 20 != 0){
    printf("Error: Wrong size on platform of type grass - "
           "w and h must be divisable by 20\n");
    exit(1);
  }
  int px, py;

  if (paintpics != NULL){
    for(py = 0; py < paintpics_rows; py ++)
      delete paintpics[py];
    delete paintpics;
  }

  paintpics_cols = pos.w / 20;
  paintpics_rows = pos.h / 20;
  paintpics = new Specdata*[paintpics_rows];
  for(px = 0; px < paintpics_rows; px ++){
    paintpics[px] = new Specdata[paintpics_cols];
    for(py = 0; py < paintpics_cols; py ++){
      paintpics[px][py].specpic = 1; //set a random picture!! :-)
      paintpics[px][py].upleftspec = SPECNONE;
      paintpics[px][py].uprightspec = SPECNONE;
      paintpics[px][py].downleftspec = SPECNONE;
      paintpics[px][py].downrightspec = SPECNONE;
    }
  }

  int x, y;
  int pc, pr; //col and row
  Square *sqr;

  //generate specpics up...
  pr = 0;
  y = pos.y -1;
  for (pc = 0; pc < paintpics_cols; pc ++){
    x = pos.x + 20 * pc + 10;
    //is there something on x, y??
    sqr = world->grid->examine(x, y);

    paintpics[pr][pc].upleftspec = SPEC_U_P;
    paintpics[pr][pc].uprightspec = SPEC_U_P;

    if (sqr){
      for(Node<Platform> *pfl = sqr->platforms; pfl != NULL; pfl = pfl->next)
        if (dynamic_cast<Grassplatform*>(pfl->link) &&
            pfl->link->pos.y + pfl->link->pos.h - 1 == y &&
            pfl->link->pos.x <= x &&
            pfl->link->pos.w + pfl->link->pos.x > x){
          paintpics[pr][pc].upleftspec = SPECNONE;
          paintpics[pr][pc].uprightspec = SPECNONE;
        }
    }
  }

  //generate specpics down...
  pr = paintpics_rows - 1;
  y = pos.y + pos.h;
  for (pc = 0; pc < paintpics_cols; pc ++){
    x = pos.x + 20 * pc + 10;
    //is there something on x, y??
    sqr = world->grid->examine(x, y);

    paintpics[pr][pc].downleftspec = SPEC_D_P;
    paintpics[pr][pc].downrightspec = SPEC_D_P;

    if (sqr){
      for(Node<Platform> *pfl = sqr->platforms; pfl != NULL; pfl = pfl->next)
        if (dynamic_cast<Grassplatform*>(pfl->link) &&
            pfl->link->pos.y == y &&
            pfl->link->pos.x <= x &&
            pfl->link->pos.w + pfl->link->pos.x > x){
          paintpics[pr][pc].downleftspec = SPECNONE;
          paintpics[pr][pc].downrightspec = SPECNONE;
        }
    }
  }

  //generate specpics left...
  pc = 0;
  x = pos.x -1;
  int dlrestore, ulrestore; //upleftrestore and downleftrestore, instead of
                   //settings it to SPECNONE if there's something to the right
  for (pr = 0; pr < paintpics_rows; pr ++){
    y = pos.y + 20 * pr + 10;
    //is there something on x, y??
    sqr = world->grid->examine(x, y);

    dlrestore = paintpics[pr][pc].downleftspec;
    ulrestore = paintpics[pr][pc].upleftspec;
    if (paintpics[pr][pc].upleftspec == SPEC_U_P)
      paintpics[pr][pc].upleftspec = SPEC_UL_E;
    else
      paintpics[pr][pc].upleftspec = SPEC_L_W;

    if (paintpics[pr][pc].downleftspec == SPEC_D_P)
      paintpics[pr][pc].downleftspec = SPEC_DL_E;
    else
      paintpics[pr][pc].downleftspec = SPEC_L_W;

    if (sqr){
      for(Node<Platform> *pfl = sqr->platforms; pfl != NULL; pfl = pfl->next)
        if (dynamic_cast<Grassplatform*>(pfl->link) &&
            pfl->link->pos.x + pfl->link->pos.w - 1 == x &&
            pfl->link->pos.y <= y &&
            pfl->link->pos.h + pfl->link->pos.y > y){
          if (ulrestore == SPEC_U_P){
            paintpics[pr][pc].upleftspec = SPEC_U_P;
            Square *ts = world->grid->examine(pos.x - 1, pos.y - 1);
            if (ts){
              for(Node<Platform> *pft = ts->platforms; pft != NULL;
                  pft = pft->next)
                if (_px_in_rect( pos.x + pc * 20 - 1, pos.y -1,
                                 pft->link->pos ))
                    paintpics[pr][pc].upleftspec = SPEC_UL_F;
            }
          }
          else
            paintpics[pr][pc].upleftspec = ulrestore;

          if (dlrestore == SPEC_D_P){
            paintpics[pr][pc].downleftspec = SPEC_D_P;
            Square *ts = world->grid->examine(pos.x - 1, pos.y + pos.h);
            if (ts){
              for(Node<Platform> *pft = ts->platforms; pft != NULL;
                  pft = pft->next)
                if (_px_in_rect( pos.x - 1, pos.y + pos.h, pft->link->pos ))
                    paintpics[pr][pc].downleftspec = SPEC_DL_R;
            }
          }
          else
            paintpics[pr][pc].downleftspec = dlrestore;
        }
    }
  }

  //generate specpics right...
  pc = paintpics_cols - 1;
  x = pos.x + pos.w;
  int drrestore, urrestore; //upleftrestore and downleftrestore, instead of
                   //settings it to SPECNONE if there's something to the right
  for (pr = 0; pr < paintpics_rows; pr ++){
    y = pos.y + 20 * pr + 10;

    sqr = world->grid->examine(x, y);

    drrestore = paintpics[pr][pc].downrightspec;
    urrestore = paintpics[pr][pc].uprightspec;
    if (paintpics[pr][pc].uprightspec == SPEC_U_P)
      paintpics[pr][pc].uprightspec = SPEC_UR_E;
    else
      paintpics[pr][pc].uprightspec = SPEC_R_W;

    if (paintpics[pr][pc].downrightspec == SPEC_D_P)
      paintpics[pr][pc].downrightspec = SPEC_DR_E;
    else
      paintpics[pr][pc].downrightspec = SPEC_R_W;

    if (sqr){
      for(Node<Platform> *pfl = sqr->platforms; pfl != NULL; pfl = pfl->next)
        if (dynamic_cast<Grassplatform*>(pfl->link) &&
            pfl->link->pos.x == x &&
            pfl->link->pos.y <= y &&
            pfl->link->pos.h + pfl->link->pos.y > y){
          if (urrestore == SPEC_U_P){
            paintpics[pr][pc].uprightspec = SPEC_U_P;
            Square *ts = world->grid->examine(pos.x + pos.w, pos.y - 1);
            if (ts){
              for(Node<Platform> *pft = ts->platforms; pft != NULL;
                  pft = pft->next)
                if (_px_in_rect( pos.x + pos.w, pos.y - 1, pft->link->pos ))
                    paintpics[pr][pc].uprightspec = SPEC_UR_F;
            }
          }
          else
            paintpics[pr][pc].uprightspec = urrestore;
          if (drrestore == SPEC_D_P){
            paintpics[pr][pc].downrightspec = SPEC_D_P;
            Square *ts = world->grid->examine(pos.x + pos.w, pos.y + pos.h);
            if (ts){
              for(Node<Platform> *pft = ts->platforms; pft != NULL;
                  pft = pft->next)
                if (_px_in_rect( pos.x + pos.w, pos.y + pos.h,
                                 pft->link->pos ))
                    paintpics[pr][pc].downrightspec = SPEC_DR_R;
            }
          }
          else
            paintpics[pr][pc].downrightspec = drrestore;
        }
    }
  }

  //loop thru upside and see if something is touching it
  pr = 0;
  for (pc = 1; pc < paintpics_cols; pc ++){
    if (paintpics[pr][pc - 1].uprightspec == SPECNONE &&
        paintpics[pr][pc].upleftspec == SPEC_U_P)
      paintpics[pr][pc].upleftspec = SPEC_UL_F;
  }
  for (pc = 0; pc < paintpics_cols -1; pc ++){
    if (paintpics[pr][pc + 1].upleftspec == SPECNONE &&
        paintpics[pr][pc].uprightspec == SPEC_U_P)
      paintpics[pr][pc].uprightspec = SPEC_UR_F;
  }

  pr = paintpics_rows - 1;
  for (pc = 1; pc < paintpics_cols; pc ++){
    if (paintpics[pr][pc - 1].downrightspec == SPECNONE &&
        paintpics[pr][pc].downleftspec == SPEC_D_P)
      paintpics[pr][pc].downleftspec = SPEC_DL_R;
  }
  for (pc = 0; pc < paintpics_cols -1; pc ++){
    if (paintpics[pr][pc + 1].downleftspec == SPECNONE &&
        paintpics[pr][pc].downrightspec == SPEC_D_P)
      paintpics[pr][pc].downrightspec = SPEC_DR_R;
  }

  //check every corners... upleft
  if (paintpics[0][0].upleftspec == SPEC_UL_E){
    Square *s = world->grid->examine(pos.x - 1, pos.y - 1);
    if (s)
      for(Node<Platform> *pf = s->platforms; pf != NULL; pf = pf->next){
        if (dynamic_cast<Grassplatform *>(pf->link)){
          if (_px_in_rect(pos.x - 1, pos.y - 1, pf->link->pos)){
            paintpics[0][0].upleftspec = SPEC_UL_F;
          }
        }
      }
  }

  //upright
  if (paintpics[0][paintpics_cols - 1].uprightspec == SPEC_UR_E){
    Square *s = world->grid->examine(pos.x + pos.w, pos.y - 1);
    if (s)
      for(Node<Platform> *pf = s->platforms; pf != NULL; pf = pf->next){
        if (dynamic_cast<Grassplatform *>(pf->link)){
          if (_px_in_rect(pos.x + pos.w, pos.y - 1, pf->link->pos)){
            paintpics[0][paintpics_cols - 1].uprightspec = SPEC_UR_F;
          }
        }
      }
  }

  //downright
  if (paintpics[paintpics_rows - 1][paintpics_cols - 1].downrightspec ==
      SPEC_DR_E){
    Square *s = world->grid->examine(pos.x + pos.w, pos.y + pos.h);
    if (s)
      for(Node<Platform> *pf = s->platforms; pf != NULL; pf = pf->next){
        if (dynamic_cast<Grassplatform *>(pf->link)){
          if (_px_in_rect(pos.x + pos.w, pos.y + pos.h, pf->link->pos)){
            paintpics[paintpics_rows - 1][paintpics_cols - 1].downrightspec =
              SPEC_DR_R;
          }
        }
      }
  }

  //downleft
  if (paintpics[paintpics_rows - 1][0].downleftspec ==
      SPEC_DL_E){
    Square *s = world->grid->examine(pos.x - 1, pos.y + pos.h);
    if (s)
      for(Node<Platform> *pf = s->platforms; pf != NULL; pf = pf->next){
        if (dynamic_cast<Grassplatform *>(pf->link)){
          if (_px_in_rect(pos.x - 1, pos.y + pos.h, pf->link->pos)){
            paintpics[paintpics_rows - 1][0].downleftspec = SPEC_DL_R;
          }
        }
      }
  }

}


Grassplatform::Grassplatform(int x, int y, int w, int h, SDL_Surface **screen,
                             Map *iworld): Platform(x,y,w,h){
  if (grassplatform_count == 0)
    tilesurf = load_optimized_pic("graphics/grass.bmp", screen, 44, 255,
                                  153);
  world = iworld;
  paintpics_cols = 0;
  paintpics_rows = 0;
  paintpics = NULL; //this will be allocated when generate_graphics is called
  generate_graphics();
  grassplatform_count ++;
  set_pos(x,y,w,h);
}

void Grassplatform::set_pos(int ix, int iy, int iw, int ih){
  set_pos_x(ix);
  set_pos_y(iy);
  set_pos_w(iw);
  set_pos_h(ih);
}
void Grassplatform::set_pos_x(int ix){
  pos.x = ((int)((ix + 10) / 20)) * 20;
}

void Grassplatform::set_pos_y(int iy){
  pos.y = ((int)((iy + 10) / 20)) * 20;
}
void Grassplatform::set_pos_w(int iw){
  if (iw < 20)
    iw = 20;
  pos.w = ((int)((iw + 10) / 20)) * 20;
}
void Grassplatform::set_pos_h(int ih){
  if (ih < 20)
    ih = 20;
  pos.h = ((int)((ih + 10) / 20)) * 20;
}

void Grassplatform::setdirtpic(SDL_Rect &src, int dirtpic){
  if (dirtpic >= 1 && dirtpic <= 9){
    src.x = 53 + (dirtpic - 1) * 20;
    src.y = 70;
    src.w = 20;
    src.h = 20;
  }
  else{
    printf("Grassplatform: Errorous dirtpic \"%d\"\n", dirtpic);
    exit(1);
  }
}

Grassplatform::~Grassplatform(){
  grassplatform_count --;

  if (grassplatform_count == 0)
    SDL_FreeSurface(tilesurf);

  if (paintpics != NULL){
    for(int i = 0; i < paintpics_rows; i++)
      delete paintpics[i];
    delete paintpics;
  }
}

void Specdata::setspecdata(char ul, char ur, char dl, char dr){

  if (ul == '0')
    upleftspec = SPECNONE;
  else if (ul == 'f')
    upleftspec = SPEC_UL_F;
  else if (ul == 'e')
    upleftspec = SPEC_UL_E;
  else if (ul == 'p')
    upleftspec = SPEC_U_P;
  else if (ul == 'w')
    upleftspec = SPEC_L_W;
  else{
    printf("Grassplatform: Unknown type for specialpicture up-left \"%c\"\n",
           ul);
    exit(1);
  }

  if (ur == '0')
    uprightspec = SPECNONE;
  else if (ur == 'f')
    uprightspec = SPEC_UR_F;
  else if (ur == 'e')
    uprightspec = SPEC_UR_E;
  else if (ur == 'p')
    uprightspec = SPEC_U_P;
  else if (ur == 'w')
    uprightspec = SPEC_R_W;
  else{
    printf("Grassplatform: Unknown type for specialpicture up-right \"%c\"\n",
           ur);
    exit(1);
  }

  if (dl == '0')
    downleftspec = SPECNONE;
  else if (dl == 'r')
    downleftspec = SPEC_DL_R;
  else if (dl == 'e')
    downleftspec = SPEC_DL_E;
  else if (dl == 'p')
    downleftspec = SPEC_D_P;
  else if (dl == 'w')
    downleftspec = SPEC_L_W;
  else{
    printf("Grassplatform: Unknown type for specialpicture down-left \"%c\"\n",
           ur);
    exit(1);
  }

  if (dr == '0')
    downrightspec = SPECNONE;
  else if (dr == 'r')
    downrightspec = SPEC_DR_R;
  else if (dr == 'e')
    downrightspec = SPEC_DR_E;
  else if (dr == 'p')
    downrightspec = SPEC_D_P;
  else if (dr == 'w')
    downrightspec = SPEC_R_W;
  else{
    printf("Grassplatform: Unknown type for specialpicture down-right \"%c\""
           "\n", dr);
    exit(1);
  }
}

void Grassplatform::setspecpic( SDL_Rect &src, SDL_Rect &subtarget,
                                int x, int y, int specpic ){
  switch (specpic){
  case SPEC_UL_F:
    subtarget.x = x - 1;
    subtarget.y = y - 9;
    src.x = 63;
    src.y = 14;
    src.w = 11;
    src.h = 17;
    break;
  case SPEC_UL_E:
    subtarget.x = x - 7;
    subtarget.y = y - 4;
    src.x = 82;
    src.y = 18;
    src.w = 17;
    src.h = 16;
    break;
  case SPEC_U_P:
    subtarget.x = x;
    subtarget.y = y - 6;
    src.x = 168;
    src.y = 17;
    src.w = 10;
    src.h = 16;
    break;
  case SPEC_L_W:
    subtarget.x = x - 3;
    subtarget.y = y;
    src.x = 8;
    src.y = 7;
    src.w = 3;
    src.h = 10;
    break;
  case SPEC_UR_F:
    subtarget.x = x;
    subtarget.y = y - 10;
    src.x = 207;
    src.y = 12;
    src.w = 11;
    src.h = 17;
    break;
  case SPEC_UR_E:
    subtarget.x = x;
    subtarget.y = y - 4;
    src.x = 181;
    src.y = 18;
    src.w = 17;
    src.h = 16;
    break;
  case SPEC_R_W:
    subtarget.x = x + 10;
    subtarget.y = y;
    src.x = 42;
    src.y = 7;
    src.w = 3;
    src.h = 10;
    break;
  case SPEC_DL_E:
    subtarget.x = x - 3;
    subtarget.y = y;
    src.x = 8;
    src.y = 47;
    src.w = 13;
    src.h = 15;
    break;
  case SPEC_DL_R:
    subtarget.x = x;
    subtarget.y = y+10;
    src.x = 10;
    src.y = 72;
    src.w = 10;
    src.h = 10;
    break;
  case SPEC_D_P:
    subtarget.x = x;
    subtarget.y = y+10;
    src.x = 12;
    src.y = 98;
    src.w = 10;
    src.h = 3;
    break;
  case SPEC_DR_E:
    subtarget.x = x;
    subtarget.y = y;
    src.x = 32;
    src.y = 47;
    src.w = 13;
    src.h = 15;
    break;
  case SPEC_DR_R:
    subtarget.x = x;
    subtarget.y = y + 10;
    src.x = 33;
    src.y = 72;
    src.w = 10;
    src.h = 10;
    break;
  default:
    printf("Grassplatform: unknown specpic %d\n", specpic);
    exit(1);
  }
  subtarget.w = src.w;
  subtarget.h = src.h;
}

void Grassplatform::paint1(SDL_Surface **screen){
  SDL_Rect target, subtarget, src;

  prepare_to_show(target, pos);

  if (settings.collision_mode){
    SDL_FillRect(*screen, &target,
                 SDL_MapRGB((*screen)->format, 100, 150,0));
  }
  else{
    subtarget.w = 20;
    subtarget.h = 20;
    for(int r = 0; r < paintpics_rows; r++){
      for(int c = 0; c < paintpics_cols; c++){
        if ( paintpics[r][c].specpic >= 1 && paintpics[r][c].specpic <= 9){
          subtarget.x = target.x + c * 20;
          subtarget.y = target.y + r * 20;
          setdirtpic(src, paintpics[r][c].specpic);
        }
        else{
          printf("Grassplatform:paint() Unknown specpic \"%d\"\n",
                 paintpics[r][c].specpic );
          exit(1);
        }
        if(SDL_BlitSurface(tilesurf, &src, *screen,
                           &subtarget)){
          printf("Grassplatform:paint() Error while blitting:%s",
                 SDL_GetError());
          exit(1);
        }
      }
    }
  }
}

void Grassplatform::paint2(SDL_Surface **screen){
  if (settings.collision_mode)
    return;
  SDL_Rect src, target, subtarget;
  prepare_to_show(target, pos);
  for(int r = 0; r < paintpics_rows; r++){
    for(int c = 0; c < paintpics_cols; c++){
      if (paintpics[r][c].upleftspec != SPECNONE){
        setspecpic(src, subtarget, target.x + c * 20, target.y + r * 20,
                   paintpics[r][c].upleftspec);
        if(SDL_BlitSurface(tilesurf, &src, *screen,
                           &subtarget)){
          printf("Grassplatform:paint() Error while blitting:%s",
                 SDL_GetError());
          exit(1);
        }
      }
      if (paintpics[r][c].uprightspec != SPECNONE){
        setspecpic(src, subtarget, target.x + c * 20 + 10, target.y + r * 20,
                   paintpics[r][c].uprightspec);
        if(SDL_BlitSurface(tilesurf, &src, *screen,
                           &subtarget)){
          printf("Grassplatform:paint() Error while blitting:%s",
                 SDL_GetError());
          exit(1);
        }
      }
      if (paintpics[r][c].downleftspec != SPECNONE){
        setspecpic(src, subtarget, target.x + c * 20, target.y + r * 20 + 10,
                   paintpics[r][c].downleftspec);
        if(SDL_BlitSurface(tilesurf, &src, *screen,
                           &subtarget)){
          printf("Grassplatform:paint() Error while blitting:%s",
                 SDL_GetError());
          exit(1);
        }
      }
      if (paintpics[r][c].downrightspec != SPECNONE){
        setspecpic(src, subtarget, target.x + c * 20 + 10,
                   target.y + r * 20 + 10, paintpics[r][c].downrightspec);
        if(SDL_BlitSurface(tilesurf, &src, *screen,
                           &subtarget)){
          printf("Grassplatform:paint() Error while blitting:%s",
                 SDL_GetError());
          exit(1);
        }
      }

    } //for
  }
}
