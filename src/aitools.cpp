#include "aitools.h"
#include "physics.h" //intersect_rect
#include "game.h" //ta bort!!
Aitools::Aitools(Map *iworld){
  world = iworld;
}

Creature *Aitools::identify_creature(int id){
  Creature *c;
  int ret = world->creature_log->get(id, c);
  if (ret == 0)
    return c;
  else
    return NULL;
}

Platform *Aitools::identify_platform(int id){
  Platform *p;
  int ret = world->platform_log->get(id, p);
  if (ret == 0)
    return p;
  else
    return NULL;
}

int Aitools::box_scan(Scanfor scan_for, Scanresult &res, const SDL_Rect &box){
  SDL_Rect is;
  int x1,x2,y1,y2;
  Node<Creature> *cr;
  Node<Platform> *pf;
  Square *s;
  world->grid->px_to_coord(box.x, box.y, x1, y1);
  world->grid->px_to_coord(box.x + box.w - 1, box.y + box.h - 1, x2, y2);
  if (x1 == -1 || x2 == -2 || y1 == -1 || y2 == -1){
    /* failed to scan. rectangle is outside bourders */
    return 1;
  }

  for (int r = x1; r <= x2; r++){
    for (int c = y1; c <= y2; c++){
      s = &world->grid->box[r][c];
      if (scan_for & CREATURES){
        cr = s->creatures;
        while (cr != NULL){
          if (intersect_rect(box, cr->link->pos, is)){
            res.add_creature(cr->link->getid());
          }
          cr = cr->next;
        }
      }
      if (scan_for & PLATFORMS){
        pf = s->platforms;
        while (pf != NULL){
          if (intersect_rect(box, pf->link->pos, is)){
            res.add_platform(pf->link->getid());
          }
          pf = pf->next;
        }
      }
    }
  }

  return 0;
}

bool Aitools::px_pf_scan(int x, int y){
  SDL_Rect r;
  Scanresult res;
  r.x = x;
  r.y = y;
  r.w = 1;
  r.h = 1;
  box_scan(PLATFORMS, res, r);
  return res.count_platforms() != 0;
}


/* scan functions here!!*/
void Aitools::scan_down(int xstart, int ystart, Scanresult &res) {
  int x, y; //grid coordinates
  world->grid->px_to_coord(xstart, ystart, x, y);
  Platform *tempp = NULL;
  Creature *tempc = NULL;
  Node<Creature> *clist = NULL;
  Node<Platform> *plist = NULL;
  Platform *closest = NULL; //the platform closest to (x,y) on the line down

  while(!closest) {
    plist = world->grid->box[x][y].platforms;
    while (plist) {
      tempp = plist->link;

      if (tempp->pos.x <= xstart && //x is between the platforms x...
	  tempp->pos.x + tempp->pos.w >= xstart && //...and it's width
	  tempp->pos.y > ystart) //and the platform is not above the point
	if (closest == NULL ||
	    tempp->pos.y < closest->pos.y)
	  closest = tempp;
	
      plist = plist->next;
    }

    clist = world->grid->box[x][y].creatures;
    while (clist) {
      tempc = clist->link;

      if (tempc->pos.x <= xstart &&
	  tempc->pos.x + tempc->pos.w >= xstart &&
	  tempc->pos.y > ystart &&
	  (!closest ||
	   tempc->pos.y <= closest->pos.y)) {
	res.add_creature(tempc->getid());
      }
      clist = clist->next;
    }
    y += 1;
  }
  res.add_platform(closest->getid());
}

void Aitools::scan_up(int xstart, int ystart, Scanresult &res) {
  int x, y; //grid coordinates

  world->grid->px_to_coord(xstart, ystart, x, y); 
  Platform *tempp = NULL; 
  Creature *tempc = NULL;
  Node<Creature> *clist = NULL;
  Node<Platform> *plist = NULL;
  Platform *closest = NULL; //the platform closest to (x,y) on the line down

  while(!closest) {
    plist = world->grid->box[x][y].platforms;
    while (plist) {
      tempp = plist->link;

      if (tempp->pos.x <= xstart && //x is between the platforms x...
	  tempp->pos.x + tempp->pos.w >= xstart && //...and it's width
	  tempp->pos.y < ystart) //and the platform is not above the point
	if (closest == NULL ||
	    tempp->pos.y > closest->pos.y)
	  closest = tempp;
	
      plist = plist->next;
    }

    clist = world->grid->box[x][y].creatures;
    while (clist) {
      tempc = clist->link;
      
      if (tempc->pos.x <= xstart &&
	  tempc->pos.x + tempc->pos.w >= xstart &&
	  tempc->pos.y < ystart &&
	  (!closest ||
	   tempc->pos.y >= closest->pos.y)) {
	res.add_creature(tempc->getid());
      }
      clist = clist->next;
    }
    y -= 1;
  }
  fprintf(stderr, "Platform ID: %d\nPlatform pointer: %p\n", closest->getid(), closest);
  fprintf(stderr, "Identify_platform(closest->getid()): %p\n", identify_platform(closest->getid()));
  res.add_platform(closest->getid());
}

/* end of scan functions */
