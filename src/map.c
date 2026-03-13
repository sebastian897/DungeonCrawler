#include "map.h"

#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdint.h>

#include "types.h"

MapTile tiles[tt_count] = {
    [tt_empty] = {0, true},
    [tt_floor] = {tex_floor, false},

    [tt_wall_top] = {tex_wall_top, true},
    [tt_wall_right] = {tex_wall_right, true},
    [tt_wall_bottom] = {tex_wall_bottom, true},
    [tt_wall_left] = {tex_wall_left, true},

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

static const uint8_t breakwall_patterns[] = {0, 0};
static const uint8_t masks_breakwall[] = {231, 189};

static const tile_type tile_groups[] = {tt_wall_top, tt_wall_outside_corner_topleft,
                                        tt_wall_inner_corner_topleft};

static const uint8_t pretty_patterns[][4] = {{24, 66, 24, 66}, {80, 72, 10, 18}, {80, 72, 10, 18}};
static const uint8_t pretty_masks[][4] = {
    {167, 181, 229, 173}, {47, 151, 244, 233}, {128, 32, 1, 4}};

static const tile_type tile_type_to_tile_group[] = {
    [tt_wall_top] = tt_wall_top,
    [tt_wall_right] = tt_wall_top,
    [tt_wall_bottom] = tt_wall_top,
    [tt_wall_left] = tt_wall_top,

    [tt_wall_outside_corner_topleft] = tt_wall_outside_corner_topleft,
    [tt_wall_outside_corner_topright] = tt_wall_outside_corner_topleft,
    [tt_wall_outside_corner_bottomright] = tt_wall_outside_corner_topleft,
    [tt_wall_outside_corner_bottomleft] = tt_wall_outside_corner_topleft,

    [tt_wall_inner_corner_topleft] = tt_wall_inner_corner_topleft,
    [tt_wall_inner_corner_topright] = tt_wall_inner_corner_topleft,
    [tt_wall_inner_corner_bottomright] = tt_wall_inner_corner_topleft,
    [tt_wall_inner_corner_bottomleft] = tt_wall_inner_corner_topleft,
};

static void PasteRecOnMap(Map* map, Rec rec, tile_type tile) {
  for (int y = 0; y < rec.size.height; y++) {
    for (int x = 0; x < rec.size.width; x++) {
      map->arr[rec.pos.y + y][rec.pos.x + x] = tile;
    }
  }
}

void CreateRoom(Map* map, Rec rec) {
  PasteRecOnMap(map, rec, tt_wall_top);
  PasteRecOnMap(map, (Rec){AddV2(rec.pos, (V2){1, 1}), SubSize(rec.size, (Size){2, 2})}, tt_floor);
}

static Tile_idx GetGroupFromTileType(uint8_t tile) {
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

void ApplyPattern(Tile_idx* tile, uint8_t sig, const uint8_t* patterns, const uint8_t* masks, tile_type tt) {
  for (int rot = 0; rot < 4; rot++) {
    if ((sig | masks[rot]) == (patterns[rot] | masks[rot])) {
      *tile = tt + rot;
      break;
    }
  }
}

void PrettyTiles(Map* map) {
  for (int my = 0; my < map_max_height; my++) {
    for (int mx = 0; mx < map_max_width; mx++) {
      V2 t_pos = {mx, my};
      Tile_idx* tile = &map->arr[t_pos.y][t_pos.x];
      if (GetGroupFromTileType(*tile) != tt_wall_top) continue;
      uint8_t sig = GetSignature(map, t_pos);
      for (int tt = 0; tt < ARRAY_LENGTH(tile_groups); tt++) {
        ApplyPattern(tile, sig, pretty_patterns[tt], pretty_masks[tt], tile_groups[tt]);
      }
    }
  }
}

void BreakWalls(Map* map) {
  for (int my = 0; my < map_max_height; my++) {
    for (int mx = 0; mx < map_max_width; mx++) {
      V2 t_pos = {mx, my};
      Tile_idx* tile = &map->arr[t_pos.y][t_pos.x];
      if (GetGroupFromTileType(*tile) != tt_wall_top) continue;
      uint8_t sig = GetSignature(map, t_pos);
      ApplyPattern(tile, sig, breakwall_patterns, masks_breakwall, tt_floor);
    }
  }
}
