#ifndef __TRIGGER_H__
#define __TRIGGER_H__

#include <string>
#include "map.h"

class Location;
class Aitools;

enum TriggerCondition {
  TC_NONE, //there's no condition to trigger this trigger
  TC_ENTER_LOCATION, // A creatures is (fully) inside a location
  TC_ALL_ENEMIES_DEAD // The given team has no enemies left
};

enum TriggerAction {
  TA_NONE, //When this trigger triggers, nothing will happen
  TA_VICTORY, // The game was completed!
  TA_DEFEAT, // Game over...
  TA_INTERN_TELEPORT // Teleports creatures on one team at a given location
                     // to another location
  /*
  TA_DISPLAY_MESSAGE, // A Message is displayed
  TA_WAIT, // Wait for a while before continuing
  TA_PAUSE, // Pauses the game (not the same pause as ingame-pause
  TA_UNPAUSE, // Unpauses the game
  TA_EXTERN_TELEPORT, // Teleports creatures on one team at a given location
                      // to another location in another map
  */
};

enum TriggerCreature {
  TCR_ANY,
  TCR_PLAYER,
  TCR_POMGOB,
  TCR_SBBFLYER,
  TCR_ZOMBO
};

/****************** CONDITIONS setting-classes *******************/

class Tco_enter_location{
 public:
  // Condition: location_id detected a creature
  int ltag;
};

class Tco_all_enemies_dead{
 public:
  // Condition: team T has no enemes left alive
  int team;
};

/****************** ACTIOINS setting-classes **********************/

/*
class TA_Display_message{
  // A Message is displayed
  string message;
};

class TA_Wait{
  // Wait for a while before continuing
};
*/

class TA_Intern_teleport{
 public:
  /* Teleports creatures from location with ltag=frltag to
     location with tag toltag, note that cretures */
  int frltag; //from locationtag, -1 means from anywhere
  Location *lfrom; /* pointer to the location to teleport from
                      (used only in game) */
  int toltag; //to locationtag, -1 means to nowhere (ie. remove it)
  Location *lto; /* pointer to the location to teleport to
                      (used only in game) */
  int ctm; //creatureteam, -1 means any team
  bool csbb; //creature - small blue bird
  bool cpom; //creature - pomgob
  bool cpla; //creature - player
  bool cblu; //creature - blubber
  bool czom; //creature - zombo
};

/*
class TA_Extern_teleport{
  TA_EXTERN_TELEPORT, // Teleports creatures on one team at a given location
                      // to another location in another map
};
*/

union ConditionSettings{
  Tco_enter_location entloc; //enters location
  Tco_all_enemies_dead enedead; //enemies dead
};

union ActionSettings{
  //pause and unpause has no structure since, they have no options
  TA_Intern_teleport intel; //internal teleport
  /*
  TA_Display_message msg;
  TA_Wait wait;
  TA_Extern_teleport exttel;
  */
};

class Trigger{
 public:
  int tag; //a way to identify a trigger
  TriggerCondition cond; // what condition that activates the trigger
  ConditionSettings condo; //the options for the condition
  TriggerAction act; /* what action that will happen when the trigger is
                        activated */
  ActionSettings acto; // options for the action
  void set_cond(TriggerCondition newcond);
  void set_act(TriggerAction newact);
  void activate(Aitools *aitools, Map &world); // activates the trigger
  void think();
  std::string generate_save_string();
  Trigger(TriggerCondition itc, TriggerAction ita);
  Trigger(Map *world, std::string initline);
};

#endif // __TRIGGER_H__
