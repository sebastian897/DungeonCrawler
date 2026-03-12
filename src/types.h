#pragma once
#include <raylib.h>
#include <stdint.h>

#define ARRAY_LENGTH(x) ((int)(sizeof(x) / sizeof((x)[0])))
#define ROUND3DP(x) (round((x) * 1000.0) / 1000.0)

enum {
  map_max_width = 64,
  map_max_height = 64,
  tile_size = 64,
  structure_max_width = 16,
  structure_max_height = 16,
  hallway_size = 2,
  num_chars = 1
};

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

typedef struct V2 {
  int x;
  int y;
} V2;

typedef enum texture_type {
  tex_empty,
  tex_floor,

  tex_wall_bottom,
  tex_wall_left,
  tex_wall_top,
  tex_wall_right,

  tex_outer_corner_topleft,
  tex_outer_corner_topright,
  tex_outer_corner_bottomright,
  tex_outer_corner_bottomleft,

  tex_inner_corner_topleft,
  tex_inner_corner_topright,
  tex_inner_corner_bottomright,
  tex_inner_corner_bottomleft,

  tex_count,
} texture_type;

typedef enum tile_type {
  tt_empty,
  tt_floor,

  tt_wall_bottom,
  tt_wall_left,
  tt_wall_top,
  tt_wall_right,

  tt_wall_outside_corner_topleft,
  tt_wall_outside_corner_topright,
  tt_wall_outside_corner_bottomright,
  tt_wall_outside_corner_bottomleft,

  tt_wall_inner_corner_topleft,
  tt_wall_inner_corner_topright,
  tt_wall_inner_corner_bottomright,
  tt_wall_inner_corner_bottomleft,

  tt_count,
} tile_type;

typedef struct MapTile {
  texture_type texture;
  bool can_col;
} MapTile;


typedef uint8_t tile_idx;

typedef struct Map {
  tile_idx arr[map_max_height][map_max_width];
} Map;

typedef struct MapPos {
  int x;
  int y;
} MapPos;

typedef struct Condition {
  uint8_t cond_halves[2];
} Condition;

typedef struct ScreenPos {
  float x;
  float y;
} ScreenPos;

typedef struct Rec {
  int x;
  int y;
  int width;
  int height;
} Rec;

typedef struct Structure {
  tile_idx arr[map_max_height][map_max_width];
  V2 pos;
} Structure;

V2 GetPosOfRec(Rec rec);
V2 AddV2(V2 v1, V2 v2);
V2 SubV2(V2 v1, V2 v2);
Vector2 V2ToVector2(V2 v);
V2 Vector2ToV2(Vector2 v);
ScreenPos V2ToScreenPos(V2 v);
