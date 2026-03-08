#include <math.h>
#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "raymath.h"
#include "resources.h"

#define ARRAY_LENGTH(x) ((int)(sizeof(x) / sizeof((x)[0])))
#define ROUND3DP(x) (round((x) * 1000.0) / 1000.0)
#define map_max_width 64
#define map_max_height 64
#define structure_max_width 16
#define structure_max_height 16
enum { tile_size = 64, hallway_size = 2, num_chars = 1 };
Texture2D wall_texture;
Texture2D wall_corner_texture;
Texture2D tile_texture;
Texture2D wall_background_texture;
typedef struct Animation {
  int curr_sprite;
  int anim_speed;
  int anim_frame_counter;
  float facing;
} Animation;

typedef struct Character {
  char* name;
  Texture2D sprite_sheet;
  int stopped_sprite;
  int first_walk_sprite;
  int last_walk_sprite;
  int first_stopping_sprite;
  Animation anim;
} Character;

typedef struct Player {
  Vector2 pos;
  int width;
  int height;
  float speed;
  Character character;
} Player;

typedef struct V2 {
  int x;
  int y;
} V2;

typedef enum texture_type {
  tex_empty,
  tex_floor,
  tex_wall,
  tex_outer_corner,
  tex_inner_corner,
  tex_count,
} texture_type;

Texture2D textures[tex_count] = {0};

typedef struct MapTile {
  texture_type texture;
  int rot;
  bool can_col;
} MapTile;

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

MapTile tiles[tt_count] = {
    [tt_empty] = {0, 0, true},
    [tt_floor] = {0, 0, false},
    [tt_wall_bottom] = {0, 0, true},
    [tt_wall_left] = {0, 1, true},
    [tt_wall_top] = {0, 2, true},
    [tt_wall_right] = {0, 3, true},
    [tt_wall_outside_corner_topleft] = {0, 0, true},
    [tt_wall_outside_corner_topright] = {0, 1, true},
    [tt_wall_outside_corner_bottomright] = {0, 2, true},
    [tt_wall_outside_corner_bottomleft] = {0, 3, true},
    [tt_wall_inner_corner_topleft] = {0, 0, true},
    [tt_wall_inner_corner_topright] = {0, 1, true},
    [tt_wall_inner_corner_bottomright] = {0, 2, true},
    [tt_wall_inner_corner_bottomleft] = {0, 3, true},
};

static const uint8_t patterns_breakwall[] = {0, 0};
static const uint8_t masks_breakwall[] = {231, 189};

static const uint8_t patterns_pretty_outside_corners[] = {80, 72, 10, 18};
static const uint8_t masks_pretty_outside_corners[] = {47, 151, 244, 233};

static const uint8_t patterns_pretty_inner_corners[] = {80, 72, 10, 18};
static const uint8_t masks_pretty_inner_corners[] = {128, 32, 1, 4};

static const tile_type tile_groups[] = {tt_wall_bottom, tt_wall_outside_corner_topleft};
static const tile_type tile_type_to_tile_group[] = {
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

typedef uint8_t tile_idx;

typedef struct Map {
  tile_idx arr[map_max_height][map_max_width];
} Map;

typedef struct MapPos {
  int x;
  int y;
} MapPos;

// typedef struct TileType {
//   uint8_t type;
// } TileType;

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

// MapTile DeepCopyMapTile(MapTile tile) {
//   return (MapTile){tile.texture, tile.rot, tile.can_col};
// }

V2 V2Empty() {
  return (V2){0, 0};
}

V2 ScaleV2(V2 v, int sf) {
  return (V2){v.x * sf, v.y * sf};
}

V2 DeepCopyV2(V2 v) {
  return (V2){v.x, v.y};
}

V2 GetPosOfRec(Rec rec) {
  return (V2){rec.x, rec.y};
}

V2 AddV2(V2 v1, V2 v2) {
  return (V2){v1.x + v2.x, v1.y + v2.y};
}

V2 SubV2(V2 v1, V2 v2) {
  return (V2){v1.x - v2.x, v1.y - v2.y};
}

bool V2Equal(V2 v1, V2 v2) {
  return v1.x == v2.x && v1.y == v2.y;
}

Vector2 V2ToVector2(V2 v) {
  return (Vector2){v.x, v.y};
}

V2 Vector2ToV2(Vector2 v) {
  return (V2){v.x, v.y};
}

ScreenPos V2ToScreenPos(V2 v) {
  return (ScreenPos){v.x * tile_size, v.y * tile_size};
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

bool PBetween2P(float p, float p2l, float p2h) {
  return p >= p2l && p <= p2h;
}

bool Collided(ScreenPos r1, ScreenPos r2) {
  return (PBetween2P(r1.x, r2.x, r2.x + tile_size - 1) ||
          PBetween2P(r1.x + tile_size - 1, r2.x, r2.x + tile_size - 1)) &&
         (PBetween2P(r1.y, r2.y, r2.y + tile_size - 1) ||
          PBetween2P(r1.y + tile_size - 1, r2.y, r2.y + tile_size - 1));
}

int rotation[4] = {0, 1, 2, 3};
void MovePlayer(Player* p, Camera2D* c, Map* map) {
  V2 dir_vec = {IsKeyDown(KEY_D) - IsKeyDown(KEY_A), IsKeyDown(KEY_S) - IsKeyDown(KEY_W)};
  // V2 dir_vec = {-1, 1};
  if (!(dir_vec.x == 0 && dir_vec.y == 0)) {
    float mag = p->speed;
    float rot = atan2(dir_vec.y, dir_vec.x);
    int newx = p->pos.x + mag * ROUND3DP(cos(rot));
    int newy = p->pos.y + mag * ROUND3DP(sin(rot));
    bool x_col_tile = false;
    bool y_col_tile = false;
    V2 col_tile = {-1, -1};
    for (int my = 0; my < map_max_height; my++) {
      for (int mx = 0; mx < map_max_width; mx++) {
        if (!tiles[map->arr[my][mx]].can_col) continue;
        V2 tile = {mx, my};
        bool xcol = Collided((ScreenPos){newx, p->pos.y}, V2ToScreenPos(tile));
        bool ycol = Collided((ScreenPos){p->pos.x, newy}, V2ToScreenPos(tile));
        if (xcol) {
          x_col_tile = true;
          col_tile.x = SubV2(tile, dir_vec).x;
        }
        if (ycol) {
          y_col_tile = true;
          col_tile.y = SubV2(tile, dir_vec).y;
        }
      }
    }
    if (!x_col_tile) {
      p->pos.x = newx;
    } else if (col_tile.x >= 0) {
      p->pos.x = V2ToScreenPos(col_tile).x;
    }
    if (!y_col_tile) {
      p->pos.y = newy;
    } else if (col_tile.y >= 0) {
      p->pos.y = V2ToScreenPos(col_tile).y;
    }

    p->character.anim.facing = rot;
    if (p->character.anim.anim_frame_counter >= p->character.anim.anim_speed) {
      p->character.anim.anim_frame_counter = 0;
      if (p->character.anim.curr_sprite == p->character.last_walk_sprite) {
        p->character.anim.curr_sprite = p->character.first_walk_sprite;
      } else {
        p->character.anim.curr_sprite += 1;
      }
    }
    c->target = p->pos;
  } else {
    if (p->character.anim.anim_frame_counter >= p->character.anim.anim_speed) {
      p->character.anim.anim_frame_counter = 0;
      if (p->character.anim.curr_sprite > p->character.first_stopping_sprite)
        p->character.anim.curr_sprite = p->character.first_stopping_sprite;
      else if (!(p->character.anim.curr_sprite == p->character.stopped_sprite))
        p->character.anim.curr_sprite -= 1;
    }
  }
  p->character.anim.anim_frame_counter++;
}
void RecTiles(Structure* structure, Rec rec, tile_type tile) {
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

void AddStructureToEnvObjs(Map* map, Structure* structure) {
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
void RotateEnvObjs(Structure* dest, Structure* source, Rec envobjs, int rot) {
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

void CreateWall(Structure* wallobjs, Rec rec) {
  RecTiles(wallobjs, (Rec){rec.x, rec.y, rec.width, rec.height},
           tt_wall_top);
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

bool IsInBounds(V2 pos) {
  return pos.x >= 0 && pos.y >= 0 && pos.x < map_max_width && pos.y < map_max_height;
}

bool DoesConditionMatch(Condition c1, Condition c2) {
  for (int h = 0; h < 2; h++) {
    if (c1.cond_halves[h] != c2.cond_halves[h]) return false;
  }
  return true;
}

uint8_t GetGroupFromTileType(uint8_t tile) {
  return tile_type_to_tile_group[tile];
}

uint8_t GetSignature(Map* map, V2 pos) {
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

int main(void) {
  InitWindow(0, 0, "Game");

  const int screenWidth = GetMonitorWidth(GetCurrentMonitor());
  const int screenHeight = GetMonitorHeight(GetCurrentMonitor());

  textures[tex_wall] = ResourceTexture(RES_WALL);
  textures[tex_floor] = ResourceTexture(RES_FLOOR);
  textures[tex_outer_corner] = ResourceTexture(RES_WALL_OUTSIDE_CORNER);
  textures[tex_inner_corner] = ResourceTexture(RES_WALL_INSIDE_CORNER);

  Character Chars[num_chars] = {
      (Character){"Wizard", ResourceTexture(RES_WIZARD), 0, 2, 5, 1, (Animation){0, 8, 0, 0}}};

  tiles[tt_floor].texture = tex_floor;
  tiles[tt_wall_top].texture = tiles[tt_wall_left].texture = tiles[tt_wall_right].texture =
      tiles[tt_wall_bottom].texture = tex_wall;
  tiles[tt_wall_outside_corner_topleft].texture = tiles[tt_wall_outside_corner_topright].texture =
      tiles[tt_wall_outside_corner_bottomright].texture =
          tiles[tt_wall_outside_corner_bottomleft].texture = tex_outer_corner;
  tiles[tt_wall_inner_corner_topleft].texture = tiles[tt_wall_inner_corner_topright].texture =
      tiles[tt_wall_inner_corner_bottomright].texture =
          tiles[tt_wall_inner_corner_bottomleft].texture = tex_inner_corner;

  Player player = {0};
  player.pos.x = 150;
  player.pos.y = 150;
  player.width = 64;
  player.height = 64;
  player.speed = 5;
  player.character = Chars[0];

  Camera2D camera = {0};
  camera.target = player.pos;
  camera.offset = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
  camera.rotation = 0.0f;
  camera.zoom = 2.0f;

  Map map = {0};
  CreateRoom(&map, (Rec){0, 0, 10, 10});
  CreateRoom(&map, (Rec){9, 3, 6, 4});
  CreateRoom(&map, (Rec){14, 0, 10, 10});
  BreakWalls(&map);
  PrettyTiles(&map);

  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    MovePlayer(&player, &camera, &map);
    BeginDrawing();

    ClearBackground(RAYWHITE);

    BeginMode2D(camera);

    for (int my = 0; my < map_max_height; my++) {
      for (int mx = 0; mx < map_max_width; mx++) {
        DrawTexturePro(textures[tiles[map.arr[my][mx]].texture],
                       (Rectangle){0, 0, textures[tiles[map.arr[my][mx]].texture].width,
                                   textures[tiles[map.arr[my][mx]].texture].height},
                       (Rectangle){(mx + 1 / 2.0) * tile_size, (my + 1 / 2.0) * tile_size,
                                   tile_size, tile_size},
                       (Vector2){tile_size / 2.0, tile_size / 2.0}, tiles[map.arr[my][mx]].rot * 90,
                       WHITE);
      }
    }
    DrawTexturePro(player.character.sprite_sheet,
                   (Rectangle){player.character.anim.curr_sprite * player.width, 0, player.width,
                               player.height},
                   (Rectangle){player.pos.x + player.width / 2.0,
                               player.pos.y + player.height / 2.0, player.width, player.height},
                   (Vector2){player.width / 2.0, player.height / 2.0},
                   player.character.anim.facing * RAD2DEG, WHITE);
    EndMode2D();
    // const char* text = TextFormat("%f, %f", player.pos.x, player.pos.y);
    // DrawText(text, 0, 0, tile_size, RED);
    EndDrawing();
  }
  for (int t = 0; t < 4; t++) UnloadTexture(wall_texture);
  CloseWindow();
  return 0;
}
