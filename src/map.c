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
static const uint8_t breakwall_masks[] = {231, 189};

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
      V2I pos = V2IAdd(Vector2ToV2I(rec.pos), (V2I){x, y});
      map->arr[pos.y][pos.x] = tile;
    }
  }
}

void CreateRoom(Map* map, Rec rec) {
  PasteRecOnMap(map, rec, tt_wall_top);
  Vector2 pos_offset = {1, 1};
  Size size_offset = {-2, -2};
  PasteRecOnMap(map, TransformRec(rec, pos_offset, size_offset), tt_floor);
}

static Tile_idx GetGroupFromTileType(uint8_t tile) {
  return tile_type_to_tile_group[tile];
}

static bool IsInBounds(Vector2 pos) {
  return pos.x >= 0 && pos.y >= 0 && pos.x < map_max_width && pos.y < map_max_height;
}

static uint8_t GetSignature(Map* map, V2I pos) {
  uint8_t signature = 0;
  for (int dy = 1; dy >= -1; dy--) {
    for (int dx = 1; dx >= -1; dx--) {
      if (!dy && !dx) continue;
      V2I dir = {dx, dy};
      V2I new_pos = V2IAdd(pos, dir);
      uint8_t new_bit =
          !IsInBounds(V2IToVector2(new_pos)) || tiles[map->arr[new_pos.y][new_pos.x]].can_col;
      signature <<= 1;
      signature |= new_bit;
    }
  }
  return signature;
}

int ApplyPattern(uint8_t sig, const uint8_t* patterns, const uint8_t* masks, uint8_t size) {
  for (int idx = 0; idx < size; idx++) {
    if ((sig | masks[idx]) == (patterns[idx] | masks[idx])) {
      return idx;
    }
  }
  return -1;
}

void PrettyTiles(Map* map) {
  for (int my = 0; my < map_max_height; my++) {
    for (int mx = 0; mx < map_max_width; mx++) {
      V2I t_pos = {mx, my};
      Tile_idx* tile = &map->arr[t_pos.y][t_pos.x];
      if (GetGroupFromTileType(*tile) != tt_wall_top) continue;
      uint8_t sig = GetSignature(map, t_pos);
      for (int tg = 0; tg < ARRAY_LENGTH(tile_groups); tg++) {
        int idx_offset = ApplyPattern(sig, pretty_patterns[tg], pretty_masks[tg],
                                      ARRAY_LENGTH(pretty_masks[tg]));
        if (idx_offset >= 0) *tile = tile_groups[tg] + idx_offset;
      }
    }
  }
}

void BreakWalls(Map* map) {
  for (int my = 0; my < map_max_height; my++) {
    for (int mx = 0; mx < map_max_width; mx++) {
      V2I t_pos = {mx, my};
      Tile_idx* tile = &map->arr[t_pos.y][t_pos.x];
      if (GetGroupFromTileType(*tile) != tt_wall_top) continue;
      uint8_t sig = GetSignature(map, t_pos);
      int idx_offset =
          ApplyPattern(sig, breakwall_patterns, breakwall_masks, ARRAY_LENGTH(breakwall_patterns));
      if (idx_offset >= 0) *tile = tt_floor;
    }
  }
}
