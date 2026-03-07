#include <math.h>
#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "raymath.h"

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
  tt_wall_top,
  tt_wall_left,
  tt_wall_right,
  tt_wall_bottom,
  tt_wall_outside_corner_topleft,
  tt_wall_outside_corner_topright,
  tt_wall_outside_corner_bottomleft,
  tt_wall_outside_corner_bottomright,
  tt_wall_inside_corner_topleft,
  tt_wall_inside_corner_topright,
  tt_wall_inside_corner_bottomleft,
  tt_wall_inside_corner_bottomright,
  tt_count,
} tile_type;

MapTile tiles[tt_count] = {
    [tt_empty] = {0, 0, false},     [tt_floor] = {0, 0, false},
    [tt_wall_top] = {0, 0, true},   [tt_wall_left] = {0, 3, true},
    [tt_wall_right] = {0, 1, true}, [tt_wall_bottom] = {0, 2, true},
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

typedef struct Pattern {
  Condition old_pattern;
  Condition new_pattern;
} Pattern;

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
      // dest->arr[new_pos2.y][new_pos2.x].rot = (source->arr[ty][tx].rot + rot) % 4;
    }
  }
}

void CreateWall(Structure* wallobjs, Rec rec) {
  RecTiles(wallobjs, (Rec){rec.x, rec.y, rec.width, rec.height},
           tt_wall_top);  // create the middle wall squares
  // wallobjs->arr[rec.y][rec.x] =
  //     (MapTile){wall_corner_texture, rotation[0], true};  // create top left corner
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

// int dx[4] = {0, 1, 0, -1};
// int dy[4] = {-1, 0, 1, 0};
// V2 GetAdjacentToCorner(Map* map, V2 pos) {
//   V2 Adj_tile = {0, 0};
//   for (int d = 0; d < 4; d++) {
//     Adj_tile = (V2){dx[d], dy[d]};
//     if (map->arr[pos.y + Adj_tile.y][pos.x + Adj_tile.x].texture.id == wall_corner_texture.id) {
//       return Adj_tile;
//     }
//   }
//   return Adj_tile;
// }

bool IsInBounds(V2 pos) {
  return pos.x >= 0 && pos.y >= 0 && pos.x < map_max_width && pos.y < map_max_height;
}

// {any 00000000 empty 00000001 floor 00000010 wall 00000011}
int text_ids[4] = {0};
uint8_t GetIdFromTextureId(int id) {
  for (uint8_t i = 0; i < 4; i++) {
    if (text_ids[0] == id) {
      return text_ids[0];
    }
  }
  return -2;
}

uint8_t GetHalfNibFromByteAtI(uint8_t byte, int i) {
  uint8_t pos = (0b11000000 >> i * 2);
  uint8_t selected_data = (byte & pos);
  return selected_data >> (3 - i) * 2;
}

uint8_t SetHalfNibInByteAtI(uint8_t byte, uint8_t newbyte, int i) {
  uint8_t data_at_i_pos = (newbyte << (3 - i) * 2);
  return (byte | data_at_i_pos);
}

bool DoesConditionMatch(Condition c1, Condition c2) {
  for (int h = 0; h < 2; h++) {
    if (c1.cond_halves[h] != c2.cond_halves[h]) return false;
  }
  return true;
}

// int tdx[2][4] = {{-1, 0, 1, -1}, {1, -1, 0, 1}};
// int tdy[2][4] = {{-1, -1, -1, 0}, {0, 1, 1, 1}};
// Pattern patterns[1] = {0};
// void ReplaceTileFromPattern(Map* map, Pattern p) {
//   for (int my = 0; my < map_max_height; my++) {
//     for (int mx = 0; mx < map_max_width; mx++) {
//       if (GetIdFromTextureId(map->arr[my][mx].texture.id) > 1) {
//         Condition curr_cond = {0};
//         for (int d1 = 0; d1 < 2; d1++) {
//           for (int d2 = 0; d2 < 4; d2++) {
//             V2 newpos = {mx + tdx[d1][d2], my + tdy[d1][d2]};
//             if (!IsInBounds(newpos)) {
//               curr_cond.cond_halves[d1] =
//                   SetHalfNibInByteAtI(curr_cond.cond_halves[d1], 0b00000001, d2);
//             } else {
//               curr_cond.cond_halves[d1] = SetHalfNibInByteAtI(
//                   curr_cond.cond_halves[d1],
//                   GetIdFromTextureId(map->arr[newpos.y][newpos.x].texture.id), d2);
//             }
//           }
//         }
//         if (DoesConditionMatch(p.old_pattern, curr_cond)) {
//           for (int d1 = 0; d1 < 2; d1++) {
//             for (int d2 = 0; d2 < 4; d2++) {
//               text_ids[GetHalfNibFromByteAtI(p.new_pattern.cond_halves[d1], d2)] map
//                   ->arr[my + t * dy[d2]][mx + t * dx[d2]] = DeepCopyMapTile(p.new_pattern[t]);
//             }
//           }
//         }
//       }
//     }
//   }
// }

// void BreakWallsOnMap(Map* map) {
//   for (int p = 0; p < ARRAY_LENGTH(patterns); p++) ReplaceTileFromPattern(map, patterns[p]);
// for (int my = 0; my < map_max_height; my++) {
//   for (int mx = 0; mx < map_max_width; mx++) {
//     V2 tile_pos = {mx, my};
//     MapTile maptile = map->arr[my][mx];
//     V2 Adj_Corner_Tile = GetAdjacentToCorner(map, tile_pos);
//     if (maptile.texture.id == wall_corner_texture.id && V2Equal(Adj_Corner_Tile, V2Empty()))
//     {
//     }
//   }
// }
// }

int main(void) {
  InitWindow(0, 0, "Game");

  const int screenWidth = GetMonitorWidth(GetCurrentMonitor());
  const int screenHeight = GetMonitorHeight(GetCurrentMonitor());

  char* wall_png = "../resources/wall.png";
  Image wall_img = LoadImage(wall_png);
  ImageCrop(&wall_img, (Rectangle){0, 0, tile_size, tile_size});
  wall_texture = LoadTextureFromImage(wall_img);

  char* wall_corner_png = "../resources/wall_corner.png";
  Image wall_corner_img = LoadImage(wall_corner_png);
  ImageCrop(&wall_corner_img, (Rectangle){0, 0, tile_size, tile_size});
  wall_corner_texture = LoadTextureFromImage(wall_corner_img);

  char* tile_png = "../resources/tile.png";
  Image tile_img = LoadImage(tile_png);
  ImageCrop(&tile_img, (Rectangle){0, 0, tile_size, tile_size});
  tile_texture = LoadTextureFromImage(tile_img);

  textures[tex_wall] = LoadTexture("../resources/wall.png");
  textures[tex_floor] = LoadTexture("../resources/tile.png");

  tiles[tt_floor].texture = tex_floor;
  tiles[tt_wall_top].texture = tiles[tt_wall_left].texture = tiles[tt_wall_right].texture =
      tiles[tt_wall_bottom].texture = tex_wall;

  // text_ids[0] = -1;
  // text_ids[1] = 0;
  // text_ids[2] = tile_texture.id;
  // text_ids[3] = wall_texture.id;
  // patterns[0] = (Pattern){{0b00000010, 0b10000000}, {0b00000010, 0b10000000}};

  Character Chars[num_chars] = {(Character){"Wizard", LoadTexture("../resources/wizard.png"), 0, 2,
                                            5, 1, (Animation){0, 8, 0, 0}}};

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
  // BreakWallsOnMap(&map);

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
    EndDrawing();
  }
  for (int t = 0; t < 4; t++) UnloadTexture(wall_texture);
  CloseWindow();
  return 0;
}