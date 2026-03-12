#include <raylib.h>
#include <stdbool.h>
#include <raymath.h>
#include <stdint.h>

#include "map.h"

MapTile tiles[tt_count] = {
    [tt_empty] = {0, true},
    [tt_floor] = {tex_floor, false},

    [tt_wall_bottom] = {tex_wall_bottom, true},
    [tt_wall_left] = {tex_wall_left, true},
    [tt_wall_top] = {tex_wall_top, true},
    [tt_wall_right] = {tex_wall_right, true},

    [tt_wall_outside_corner_topleft] = {tex_outer_corner_topleft, true},
    [tt_wall_outside_corner_topright] = {tex_outer_corner_topright, true},
    [tt_wall_outside_corner_bottomright] = {tex_outer_corner_bottomright, true},
    [tt_wall_outside_corner_bottomleft] = {tex_outer_corner_bottomleft, true},

    [tt_wall_inner_corner_topleft] = {tex_inner_corner_topleft, true},
    [tt_wall_inner_corner_topright] = {tex_inner_corner_topright, true},
    [tt_wall_inner_corner_bottomright] = {tex_inner_corner_bottomright, true},
    [tt_wall_inner_corner_bottomleft] = {tex_inner_corner_bottomleft, true},
};

Texture2D textures[tex_count] = {0};

static const uint8_t patterns_breakwall[] = {0, 0};
static const uint8_t masks_breakwall[] = {231, 189};

static const uint8_t patterns_pretty_outside_corners[] = {80, 72, 10, 18};
static const uint8_t masks_pretty_outside_corners[] = {47, 151, 244, 233};

static const uint8_t patterns_pretty_inner_corners[] = {80, 72, 10, 18};
static const uint8_t masks_pretty_inner_corners[] = {128, 32, 1, 4};

static const tile_type tile_groups[] = {tt_wall_bottom, tt_wall_outside_corner_topleft,
                                        tt_wall_inner_corner_topleft};
static const tile_type tile_type_to_tile_group[tt_count] = {
    [tt_wall_bottom] = tt_wall_bottom,
    [tt_wall_left] = tt_wall_bottom,
    [tt_wall_top] = tt_wall_bottom,
    [tt_wall_right] = tt_wall_bottom,

    [tt_wall_outside_corner_topleft] = tt_wall_outside_corner_topleft,
    [tt_wall_outside_corner_topright] = tt_wall_outside_corner_topleft,
    [tt_wall_outside_corner_bottomright] = tt_wall_outside_corner_topleft,
    [tt_wall_outside_corner_bottomleft] = tt_wall_outside_corner_topleft,

    [tt_wall_inner_corner_topleft] = tt_wall_inner_corner_topleft,
    [tt_wall_inner_corner_topright] = tt_wall_inner_corner_topleft,
    [tt_wall_inner_corner_bottomright] = tt_wall_inner_corner_topleft,
    [tt_wall_inner_corner_bottomleft] = tt_wall_inner_corner_topleft,
};


static void RecTiles(Structure* structure, Rec rec, tile_type tile) {
  for (int y = 0; y < rec.height; y++) {
    for (int x = 0; x < rec.width; x++) {
      structure->arr[rec.y + y][rec.x + x] = tile;
    }
  }
}

int texture[4] = {2, 0, 1, 3};
int rx[4] = {0, 1, 1, 0};
int ry[4] = {0, 0, 1, 1};
int wrx[4] = {1, 0, 1, 0};
int wry[4] = {0, 1, 0, 1};

static void AddStructureToEnvObjs(Map* map, Structure* structure) {
  for (int ty = 0; ty < structure_max_height; ty++) {
    for (int tx = 0; tx < structure_max_width; tx++) {
      if (structure->arr[ty][tx] == tt_empty) continue;
      V2 newpos = AddV2(structure->pos, (V2){tx, ty});
      map->arr[newpos.y][newpos.x] = structure->arr[ty][tx];
    }
  }
}

int sxw[4] = {0, 0, 1, 0};
int syw[4] = {0, 0, 0, 1};
int sxh[4] = {0, 1, 0, 0};
int syh[4] = {0, 0, 1, 0};
static void RotateEnvObjs(Structure* dest, Structure* source, Rec envobjs, int rot) {
  V2 shift = {sxw[rot] * envobjs.width + sxh[rot] * envobjs.height,
              syw[rot] * envobjs.width +
                  syh[rot] * envobjs.height};  // add to the shape coods so is not negative
  for (int ty = envobjs.y; ty < envobjs.height + envobjs.y; ty++) {
    for (int tx = envobjs.x; tx < envobjs.width + envobjs.x;
         tx++) {  // only loop through the exact coods of the envobjs
      if (source->arr[ty][tx] == tt_empty) continue;
      V2 old_pos = (V2){tx, ty};
      V2 new_pos =
          AddV2(Vector2ToV2(Vector2Rotate(V2ToVector2(SubV2(old_pos, GetPosOfRec(envobjs))),
                                          rot * 90 * DEG2RAD)),
                GetPosOfRec(envobjs));  // just rotate the coods of the tiles
      V2 rot_factor = SubV2(
          new_pos, (V2){(rx[rot] * 1), (ry[rot] * 1)});  // shift the top left corner of each tile
      V2 new_pos2 = AddV2(rot_factor, shift);
      dest->arr[new_pos2.y][new_pos2.x] = source->arr[ty][tx];
      for (int tg = 0; tg < ARRAY_LENGTH(tile_groups); tg++) {
        int group_pos = source->arr[ty][tx] - tile_groups[tg];
        if (group_pos < 4 && group_pos >= 0) {
          dest->arr[new_pos2.y][new_pos2.x] = tile_groups[tg] + (rot + group_pos) % 4;
          break;
        }
      }
    }
  }
}

static void CreateWall(Structure* wallobjs, Rec rec) {
  RecTiles(wallobjs, (Rec){rec.x, rec.y, rec.width, rec.height}, tt_wall_top);
}

void CreateRoom(Map* map, Rec rec) {
  Structure room = {0};
  room.pos = (V2){rec.x, rec.y};
  for (int d = 0; d < 4; d++) {
    Structure new_wall_objs = {0};
    Rec new_wall = {0, 0, (wrx[d] * rec.width + wry[d] * rec.height) - 1, 1};
    CreateWall(&new_wall_objs, new_wall);

    Structure rot_new_wall_objs = {0};
    RotateEnvObjs(&rot_new_wall_objs, &new_wall_objs, new_wall, d);

    Structure shift_new_wall_objs = {0};
    V2 shift = {sxh[d] * (rec.width - 1) + sxw[d], syh[d] * (rec.height - 1) + syw[d]};
    ShiftStructure(&shift_new_wall_objs, &rot_new_wall_objs, shift);

    AddContentsToStruct(&room, &shift_new_wall_objs);
  }
  Structure new_floor_objs = {0};
  RecTiles(&new_floor_objs, (Rec){1, 1, (rec.width - 2), (rec.height - 2)}, tt_floor);
  AddContentsToStruct(&room, &new_floor_objs);
  AddStructureToEnvObjs(map, &room);
}

static uint8_t GetGroupFromTileType(uint8_t tile) {
  return tile_type_to_tile_group[tile];
}

static bool IsInBounds(V2 pos) {
  return pos.x >= 0 && pos.y >= 0 && pos.x < map_max_width && pos.y < map_max_height;
}

static uint8_t GetSignature(Map* map, V2 pos) {
  uint8_t signature = 0;
  for (int dy = 1; dy >= -1; dy--) {
    for (int dx = 1; dx >= -1; dx--) {
      if (!dy && !dx) continue;
      V2 dir = {dx, dy};
      V2 new_pos = AddV2(pos, dir);
      uint8_t new_bit = !IsInBounds(new_pos) || tiles[map->arr[new_pos.y][new_pos.x]].can_col;
      signature <<= 1;
      signature |= new_bit;
    }
  }
  return signature;
}

void PrettyTiles(Map* map) {
  for (int my = 0; my < map_max_height; my++) {
    for (int mx = 0; mx < map_max_width; mx++) {
      V2 t_pos = {mx, my};
      uint8_t* tile = &map->arr[t_pos.y][t_pos.x];
      if (GetGroupFromTileType(*tile) != tt_wall_bottom) continue;
      uint8_t sig = GetSignature(map, t_pos);
      for (int p = 0; p < ARRAY_LENGTH(patterns_pretty_outside_corners); p++) {
        if ((sig | masks_pretty_outside_corners[p]) ==
            (patterns_pretty_outside_corners[p] | masks_pretty_outside_corners[p])) {
          *tile = tt_wall_outside_corner_topleft + p;
          break;
        }
      }
      for (int p = 0; p < ARRAY_LENGTH(patterns_pretty_inner_corners); p++) {
        if ((sig | masks_pretty_inner_corners[p]) ==
            (patterns_pretty_inner_corners[p] | masks_pretty_inner_corners[p])) {
          *tile = tt_wall_inner_corner_topleft + p;
          break;
        }
      }
    }
  }
}

void BreakWalls(Map* map) {
  for (int my = 0; my < map_max_height; my++) {
    for (int mx = 0; mx < map_max_width; mx++) {
      V2 t_pos = {mx, my};
      uint8_t* tile = &map->arr[t_pos.y][t_pos.x];
      if (GetGroupFromTileType(*tile) != tt_wall_bottom) continue;
      uint8_t sig = GetSignature(map, t_pos);
      for (int p = 0; p < ARRAY_LENGTH(patterns_breakwall); p++) {
        if ((sig | masks_breakwall[p]) == (patterns_breakwall[p] | masks_breakwall[p])) {
          *tile = tt_floor;
          break;
        }
      }
    }
  }
}
void AddContentsToStruct(Structure* dest, Structure* source) {
  for (int ty = 0; ty < structure_max_height; ty++) {
    for (int tx = 0; tx < structure_max_width; tx++) {
      if (source->arr[ty][tx] == tt_empty) continue;
      dest->arr[ty][tx] = source->arr[ty][tx];
    }
  }
}

void ShiftStructure(Structure* dest, Structure* source, V2 v) {
  for (int ty = 0; ty < structure_max_height; ty++) {
    for (int tx = 0; tx < structure_max_width; tx++) {
      if (source->arr[ty][tx] == tt_empty) continue;
      V2 newpos = AddV2((V2){tx, ty}, v);
      dest->arr[newpos.y][newpos.x] = source->arr[ty][tx];
    }
  }
}
