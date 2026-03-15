#pragma once
#include <raylib.h>

#include "types.h"
typedef enum anim_state {
  as_idle,
  as_inactive,
  as_active,
} anim_state;

typedef enum anim_action {
  aa_walking,
  aa_attacking,
  aa_count,
} anim_action;

typedef enum anim_type {
  at_body,
  at_hands,
  at_effect,
  at_count,
} anim_type;

extern anim_type anim_draw_priority[at_count];

extern anim_type anim_types[at_count];

typedef struct AnimationSetup {
  Texture2D sprite_sheet;
  int initial_sprite;
  int first_active_sprite;
  int last_active_sprite;
  int first_inactive_sprite;
  int anim_speed;
  bool valid;
} AnimationSetup;

typedef struct Animation {
  // int draw_priority;
  int curr_sprite;
  int anim_frame_counter;
  float rot;
  anim_state anim_state;
  anim_action anim_action;
  AnimationSetup anim_setups[aa_count];
} Animation;

typedef struct Weapon {
  // Animation walking_anim;
  // Animation attacking_anim;
  // Animation effect_anim;
  AnimationSetup effect_attacking_anim;
  AnimationSetup hand_attacking_anim;
  bool can_hold;
} Weapon;

// extern Weapon weapons[];

typedef struct Character {
  char* name;
  Animation anims[at_count];
  // Animation body_anim;
  // Animation hand_anim;
  // Animation effect_anim;
} Character;

// typedef struct Hitbox {
//   rec;
// } Hitbox;

typedef struct Player {
  Rec rec;
  // int width;
  // int height;
  // Rec hitbox;
  float speed;
  float rot;
  Character character;
  Weapon weapons[num_weapons];
  int weapon;
  Camera2D cam;
} Player;

typedef struct Map Map;

void PlayerMove(Player* p, Map* map);
void PlayerAttack(Player* player);
void AnimatePlayer(Player* player);
void SetCameraPos(Camera2D* cam, Vector2 p_pos);