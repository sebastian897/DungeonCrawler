#pragma once
#include <raylib.h>

#include "types.h"

typedef enum texture_type {
  tex_empty,
  tex_floor,

  tex_wall_top,
  tex_wall_right,
  tex_wall_bottom,
  tex_wall_left,

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

  tt_wall_top,
  tt_wall_right,
  tt_wall_bottom,
  tt_wall_left,

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

typedef uint8_t Tile_idx;

typedef struct Map {
  Tile_idx arr[map_max_height][map_max_width];
} Map;

// typedef struct Structure {
//   tile_idx arr[map_max_height][map_max_width];
//   V2 pos;
// } Structure;

extern MapTile tiles[tt_count];
extern Texture2D textures[tex_count];

void CreateRoom(Map* map, Rec rec);
void PrettyTiles(Map* map);
void BreakWalls(Map* map);
// void AddContentsToStruct(Structure* dest, Structure* source);
// void ShiftStructure(Structure* dest, Structure* source, V2 v);
