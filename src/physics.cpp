#include "physics.h"
#include "map.h"
#include "grid.h"
#include "settings.h"
#include "platform.h"
#include "game.h"
#include "aitools.h"

#define MAXCLOSEPF 50 // maximal number of pf to check for collision

void gravity(Map &world){

  MovingCreature *incr;
  if (world.motion != NULL){
    printf("gravity bug: motion != NULL\n");
    exit(1);
  }
  Creature *c;
  for(int i = 0; i <= world.get_creature_max(); i++){
    c = world.creature[i];
    if (c != NULL){
      if (not c->immune_to_gravity){
        c->aspeed_y += settings.gravity_y;
        if (c->aspeed_x > settings.air_break)
          c->aspeed_x -= settings.air_break;
        else if (c->aspeed_x < -settings.air_break)
          c->aspeed_x += settings.air_break;
        else
          c->aspeed_x = 0;
      }

      incr = world.new_moving_creature();
      if (incr == NULL){
        printf("Too many moving creatures! Increase MAX_CREATURES?\n");
        exit(1);
      }
      incr->next = world.motion;
      incr->creature = world.creature[i];

      incr->x_speed =  (int)(c->aspeed_x + c->rspeed_x);
      c->rspeed_x = (c->aspeed_x + c->rspeed_x) - incr->x_speed;
      incr->y_speed = (int)(c->aspeed_y + c->rspeed_y);
      c->rspeed_y = (c->aspeed_y + c->rspeed_y) - incr->y_speed;

      world.motion = incr;
    }
  }
  return;
}


bool intersect_rect(const SDL_Rect &a, const SDL_Rect &b, SDL_Rect &intersect){

  int ax2 = a.x + a.w;
  int ay2 = a.y + a.h;
  int bx2 = b.x + b.w;
  int by2 = b.y + b.h;

  if (a.x <= b.x && a.x + a.w > b.x){
    // b.x is between a.x and a.x2
    intersect.w = (ax2 < bx2 ? ax2 : bx2) - b.x;
    intersect.x = b.x;
  }
  else if  (b.x <= a.x && b.x + b.w > a.x){
    // a.x is between b.x and b.x2
    intersect.w = (ax2 < bx2 ? ax2 : bx2) - a.x;
    intersect.x = a.x;
  }
  else
    return 0;

  if ( a.y <= b.y && a.y + a.h > b.y){
    //b.y is between a.y and a.y2
    intersect.h = (ay2 < by2 ? ay2 : by2) - b.y;
    intersect.y = b.y;
    return 1;
  }
  else if (b.y <= a.y && a.y < b.y + b.h){
    //a.y is between b.y and b.y2
    intersect.h = (ay2 < by2 ? ay2 : by2) - a.y;
    intersect.y = a.y;
    return 1;
  }
  return 0;
}

/* does pos collide with any platform on the given squares? */
/* returns the platform pos collides with, NULL if it doesn't */
Platform * collide(Map &world, SDL_Rect pos, int x1, int x2, int y1, int y2,
                   SDL_Rect &is){
  int x,y;
  Square *s;
  Node<Platform> *pf;
  if (x1 == -1 || x2 == -1 || y1 == -1 || y2 == -1){
    printf("errorous square in collide!\n");
    exit(1);
  }
  for (x = x1; x <= x2; x++)
    for (y = y1; y <= y2; y++){
      s = &world.grid->box[x][y];
      if (s != NULL){
        pf = s->platforms;
        while(pf != NULL){
          if (intersect_rect(pos, pf->link->pos, is))
            return pf->link;
          else
            pf = pf->next;
        }
      }
    }
  return NULL;
}

void physics(Map &world){
  MovingCreature *c;
  Platform *cw; //collide with
  int x1,x2,y1,y2,xdir,ydir;
  SDL_Rect is;
  // Move everyone in movelist
  while(world.motion != NULL){
    c = world.motion;
    world.unregister_creature(c->creature);
    xdir = (c->x_speed > 0 ? 1 : (c->x_speed < 0 ? -1 : 0));
    ydir = (c->y_speed > 0 ? 1 : (c->y_speed < 0 ? -1 : 0));
    while (c->x_speed != 0 || c->y_speed != 0){
     // move creature 1 px in x-dir
      c->creature->move_x(xdir);
      world.grid->px_to_coord(c->creature->pos.x + 1 * xdir,
                             c->creature->pos.y,
                             x1, y1);
      world.grid->px_to_coord(c->creature->pos.x + c->creature->pos.w + xdir -1,
                             c->creature->pos.y + c->creature->pos.h,
                             x2, y2);
      if (xdir > 0){
        //going right
        cw = collide(world, c->creature->pos, x2,x2,y1,y2, is);
        if (cw != NULL){
          c->creature->move_x(-1);
          c->creature->rightof = cw;
          c->creature->aspeed_x = 0;
          c->creature->rspeed_x = 0;
        }
      }
      else if (xdir < 0){
        //going left
        cw = collide(world, c->creature->pos, x1,x1,y1,y2, is);
        if (cw != NULL){
          c->creature->move_x(1);
          c->creature->leftof = cw;
          c->creature->aspeed_x = 0;
          c->creature->rspeed_x = 0;
        }
      }
      c->x_speed -= xdir;
      if(c->x_speed == 0){
        xdir = 0;
      }

      //now, move creature 1 px in y-dir
      c->creature->move_y(ydir);
      world.grid->px_to_coord(c->creature->pos.x,
                             c->creature->pos.y + 1 * ydir,
                             x1, y1);
      world.grid->px_to_coord(c->creature->pos.x + c->creature->pos.w,
                             c->creature->pos.y + c->creature->pos.h + ydir -1,
                             x2, y2);
      if (ydir < 0){
        //going up
        cw = collide(world, c->creature->pos, x1,x2,y1,y1, is);
        if (cw != NULL){
          c->creature->move_y(1);
          c->creature->above = cw;
          c->creature->aspeed_y = 0;
          c->creature->rspeed_y = 0;
        }
      }
      else if (ydir > 0){
        //going down
        cw = collide(world, c->creature->pos, x1,x2,y2,y2, is);
        if (cw != NULL){
          c->creature->move_y(-1);
          c->creature->below = cw;
          c->creature->aspeed_y = 0;
          c->creature->rspeed_y = 0;
        }
      }
      c->y_speed -= ydir;
      if(c->y_speed == 0){
        ydir = 0;
      }
    }

    //control if the creature still has below below of him
    if (c->creature->below != NULL) {
      if(c->creature->below->pos.y !=
         c->creature->pos.y + c->creature->pos.h){
        //leaped
        c->creature->below = NULL;
      }
      else if(c->creature->pos.x >=
              c->creature->below->pos.x + c->creature->below->pos.w ||
              c->creature->pos.x + c->creature->pos.w <=
              c->creature->below->pos.x ){
        c->creature->below = NULL;
      }
    }

    if (c->creature->above != NULL) {
      if(c->creature->above->pos.y + c->creature->above->pos.h !=
         c->creature->pos.y){
        // lost grip of roof
        c->creature->above = NULL;
      }
      else if(c->creature->pos.x >=
              c->creature->above->pos.x + c->creature->above->pos.w ||
              c->creature->pos.x + c->creature->pos.w <=
              c->creature->above->pos.x ){
        c->creature->above = NULL;
      }
    }

    if (c->creature->rightof != NULL) {
      if(c->creature->rightof->pos.x !=
         c->creature->pos.x + c->creature->pos.w){
        //left right wall
        c->creature->rightof = NULL;
      }
      else if(c->creature->pos.y >=
              c->creature->rightof->pos.y + c->creature->rightof->pos.h ||
              c->creature->pos.y + c->creature->pos.h <=
              c->creature->rightof->pos.y ){
        c->creature->rightof = NULL;
        //climed above/ fell below left wall
      }
    }

    if (c->creature->leftof != NULL) {
      if(c->creature->leftof->pos.x + c->creature->leftof->pos.w !=
         c->creature->pos.x){
        // left left wall
        c->creature->leftof = NULL;
      }
      else if(c->creature->pos.y >=
              c->creature->leftof->pos.y + c->creature->leftof->pos.h ||
              c->creature->pos.y + c->creature->pos.h <=
              c->creature->leftof->pos.y ){
        c->creature->leftof = NULL;
      }
    }

    world.register_creature(c->creature);
    world.motion = world.motion->next;
  }
  world.reset_motion();
  return;
}

void checkfxfields( Map &world ){
  int x, y;
  int fid; // fxfield id
  SDL_Rect is; //intersection
  Square *s;
  Node<Creature> *c;
  for ( int i = 0; i <= world.get_fxfield_max(); i ++ )
    if (world.fxfield[i] != NULL){
      for(fid = 0; fid < world.fxfield[i]->col_rects; fid ++){
        for( x = world.fxfield[i]->x1; x <= world.fxfield[i]->x2; x++ )
          for( y = world.fxfield[i]->y1; y <= world.fxfield[i]->y2; y++ ){
            s = &world.grid->box[x][y];
            if (s == NULL){
              printf("checkfxfields: errorous square\n");
              exit(1);
            }
            c = s->creatures;
            while(c != NULL){
              if (c->link->safe == 0 &&
                  world.fxfield[i]->team != c->link->team &&
                intersect_rect(world.fxfield[i]->col_rect[fid],
                                   c->link->pos ,is)){
                world.fxfield[i]->hit(c->link);
                if (not c->link->dead)
                  c->link->hurt( world.fxfield[i] );

              }
              c = c->next;
            }
          }
      }
    }
}


void checklocations( Aitools *aitools, Map &world ){
  SDL_Rect is; //intersection
  Scanresult res;
  int cid;
  Creature *c;
  for (int i = 0; i <= world.get_location_max(); i ++ )
    if (world.location[i] != NULL){
      Location *loc = world.location[i];
      if (loc->act_trigptr){
        res.reset();
        aitools->box_scan(CREATURES, res, loc->pos);
        cid = res.takeout_creature();
        while(cid != -1){
          c = aitools->identify_creature(cid);
          if (loc->act_team == c->team || loc->act_team == -1){
            if (loc->totalcol){
              if (loc->pos.x <= c->pos.x &&
                  loc->pos.x+loc->pos.w >= c->pos.x + c->pos.w &&
                  loc->pos.y <= c->pos.y &&
                  loc->pos.y+loc->pos.h >= c->pos.y + c->pos.h){
                loc->act_trigptr->activate(aitools, world);
              }
            }
            else{
              if (intersect_rect(loc->pos, c->pos ,is))
                loc->act_trigptr->activate(aitools, world);
            }
          }
          cid = res.takeout_creature();
        }
      }
    }
}
