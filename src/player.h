#pragma once
#include <raylib.h>

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
  int curr_sprite;
  int anim_frame_counter;
  float rot;
  anim_state anim_state;
  anim_action anim_action;
  AnimationSetup anim_setups[aa_count];
} Animation;

typedef struct Weapon {
  Animation walking_anim;
  Animation attacking_anim;
  Animation effect_anim;
  AnimationSetup hand_attacking_anim;
  bool can_hold;
} Weapon;

typedef struct Character {
  char* name;
  Animation body_anim;
  Animation hand_anim;
} Character;

typedef struct Player {
  Vector2 pos;
  int width;
  int height;
  float speed;
  Character character;
  Weapon weapon;
  Camera2D cam;
} Player;

typedef struct Map Map;

void PlayerMove(Player* p, Camera2D* c, Map* map);
void PlayerAttack(Player* player);
void AnimatePlayer(Player* player);
