#include <math.h>
#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>

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

typedef struct MapTile {
  Texture2D texture;
  int rot;
  bool can_col;
} MapTile;

typedef struct Map {
  MapTile arr[map_max_height][map_max_width];
} Map;

typedef struct MapPos {
  int x;
  int y;
} MapPos;

typedef struct Pattern {
  MapTile old_pattern[4];
  MapTile new_pattern[4];
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
  MapTile arr[map_max_height][map_max_width];
  V2 pos;
} Structure;

MapTile DeepCopyMapTile(MapTile tile) {
  return (MapTile){tile.texture, tile.rot, tile.can_col};
}

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

void ShiftStructure(Structure* source, Structure* dest, V2 v) {
  for (int ty = 0; ty < structure_max_height; ty++) {
    for (int tx = 0; tx < structure_max_width; tx++) {
      if (source->arr[ty][tx].texture.id == 0) continue;
      V2 newpos = AddV2((V2){tx, ty}, v);
      dest->arr[newpos.y][newpos.x] = DeepCopyMapTile(source->arr[ty][tx]);
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
        if (!map->arr[my][mx].can_col) continue;
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
void RecSqrs(Structure* structure, Rec rec, Texture2D texture, float rot, bool can_col) {
  for (int y = 0; y < rec.height; y++) {
    for (int x = 0; x < rec.width; x++) {
      structure->arr[rec.y + y][rec.x + x] = (MapTile){texture, rot, can_col};
    }
  }
}

int texture[4] = {2, 0, 1, 3};
int rx[4] = {0, 1, 1, 0};
int ry[4] = {0, 0, 1, 1};
int wrx[4] = {1, 0, 1, 0};
int wry[4] = {0, 1, 0, 1};

void AddStructureToEnvObjs(Map* map, Structure* tiles) {
  // for (int my = 0; my < map_max_height; my++) {
  for (int ty = 0; ty < structure_max_height; ty++) {
    for (int tx = 0; tx < structure_max_width; tx++) {
      if (tiles->arr[ty][tx].texture.id == 0) continue;
      V2 newpos = AddV2(tiles->pos, (V2){tx, ty});
      map->arr[newpos.y][newpos.x] = DeepCopyMapTile(tiles->arr[ty][tx]);
    }
  }
}

int sxw[4] = {0, 0, 1, 0};
int syw[4] = {0, 0, 0, 1};
int sxh[4] = {0, 1, 0, 0};
int syh[4] = {0, 0, 1, 0};
void RotateEnvObjs(Structure* source, Structure* dest, Rec envobjs, int rot) {
  V2 shift = {sxw[rot] * envobjs.width + sxh[rot] * envobjs.height,
              syw[rot] * envobjs.width +
                  syh[rot] * envobjs.height};  // add to the shape coods so is not negative
  for (int ty = envobjs.y; ty < envobjs.height + envobjs.y; ty++) {
    for (int tx = envobjs.x; tx < envobjs.width + envobjs.x;
         tx++) {  // only loop through the exact coods of the envobjs
      if (source->arr[ty][tx].texture.id == 0) continue;
      V2 old_pos = (V2){tx, ty};
      V2 new_pos =
          AddV2(Vector2ToV2(Vector2Rotate(V2ToVector2(SubV2(old_pos, GetPosOfRec(envobjs))),
                                          rot * 90 * DEG2RAD)),
                GetPosOfRec(envobjs));  // just rotate the coods of the tiles
      V2 rot_factor = SubV2(
          new_pos, (V2){(rx[rot] * 1), (ry[rot] * 1)});  // shift the top left corner of each tile
      V2 new_pos2 = AddV2(rot_factor, shift);
      dest->arr[new_pos2.y][new_pos2.x] = DeepCopyMapTile(source->arr[ty][tx]);
      dest->arr[new_pos2.y][new_pos2.x].rot = (source->arr[ty][tx].rot + rot) % 4;
    }
  }
}

void CreateWall(Structure* wallobjs, Rec rec) {
  RecSqrs(wallobjs, (Rec){rec.x + 1, rec.y, rec.width - 1, rec.height}, wall_texture, texture[0],
          true);  // create the middle wall squares
  wallobjs->arr[rec.y][rec.x] =
      (MapTile){wall_corner_texture, rotation[0], true};  // create top left corner
}

void CreateRoom(Map* map, Rec rec) {
  for (int d = 0; d < 4; d++) {
    Structure new_wall_objs = {0};
    Rec new_wall = {rec.x, rec.y, (wrx[d] * rec.width + wry[d] * rec.height) - 1, 1};
    CreateWall(&new_wall_objs, new_wall);

    Structure rot_new_wall_objs = {0};
    RotateEnvObjs(&new_wall_objs, &rot_new_wall_objs, new_wall, d);

    Structure shift_new_wall_objs = {0};
    V2 shift = {sxh[d] * (rec.width - 1) + sxw[d], syh[d] * (rec.height - 1) + syw[d]};
    ShiftStructure(&rot_new_wall_objs, &shift_new_wall_objs, shift);

    AddStructureToEnvObjs(map, &shift_new_wall_objs);
  }
  Structure new_floor_objs = {0};
  RecSqrs(&new_floor_objs, (Rec){rec.x + 1, rec.y + 1, (rec.width - 2), (rec.height - 2)},
          tile_texture, 0, false);
  AddStructureToEnvObjs(map, &new_floor_objs);
}

int dx[4] = {0, 1, 0, -1};
int dy[4] = {-1, 0, 1, 0};
V2 GetAdjacentToCorner(Map* map, V2 pos) {
  V2 Adj_tile = {0, 0};
  for (int d = 0; d < 4; d++) {
    Adj_tile = (V2){dx[d], dy[d]};
    if (map->arr[pos.y + Adj_tile.y][pos.x + Adj_tile.x].texture.id == wall_corner_texture.id) {
      return Adj_tile;
    }
  }
  return Adj_tile;
}
Pattern patterns[1] = {0};
void ReplaceTileFromPattern(Map* map, Pattern p) {
  for (int my = 0; my < map_max_height; my++) {
    for (int mx = 0; mx < map_max_width; mx++) {
      // if (my == 1 && mx == 8) {
      //   bool as = false;
      // }

      for (int d = 0; d < 4; d++) {
        bool ispattern = true;
        for (int t = 0; t < 4; t++) {
          V2 newpos = {my + t * dy[d], mx + t * dx[d]};
          if (newpos.x<0&&newpos.y<0&&newpos.x>map_max_width&&newpos.y>map_max_height) break;
          if (map->arr[newpos.y][newpos.x].texture.id != p.old_pattern[t].texture.id) {
            ispattern = false;
          }
        }
        // if (ispattern) {
        //   for (int t = 0; t < 4; t++) {
        //     map->arr[my + t * dy[d]][mx + t * dx[d]] = DeepCopyMapTile(p.new_pattern[t]);
        //   }
        // }
      }
    }
  }
}

void BreakWallsOnMap(Map* map) {
  for (int p = 0; p < ARRAY_LENGTH(patterns); p++) ReplaceTileFromPattern(map, patterns[p]);
  // for (int my = 0; my < map_max_height; my++) {
  //   for (int mx = 0; mx < map_max_width; mx++) {
  //     V2 tile_pos = {mx, my};
  //     MapTile maptile = map->arr[my][mx];
  //     V2 Adj_Corner_Tile = GetAdjacentToCorner(map, tile_pos);
  //     if (maptile.texture.id == wall_corner_texture.id && V2Equal(Adj_Corner_Tile, V2Empty())) {
  //     }
  //   }
  // }
}

int main(void) {
  InitWindow(0, 0, "Game");

  const int screenWidth = GetMonitorWidth(GetCurrentMonitor());
  const int screenHeight = GetMonitorHeight(GetCurrentMonitor());

  char* wall_png = "../resources/wall.png";
  // char* wall_png = "./resources/wall.png";
  Image wall_img = LoadImage(wall_png);
  ImageCrop(&wall_img, (Rectangle){0, 0, tile_size, tile_size});
  wall_texture = LoadTextureFromImage(wall_img);

  char* wall_corner_png = "../resources/wall_corner.png";
  // char* wall_corner_png = "./resources/wall_corner.png";
  Image wall_corner_img = LoadImage(wall_corner_png);
  ImageCrop(&wall_corner_img, (Rectangle){0, 0, tile_size, tile_size});
  wall_corner_texture = LoadTextureFromImage(wall_corner_img);

  char* tile_png = "../resources/tile.png";
  // char* tile_png = "./resources/tile.png";
  Image tile_img = LoadImage(tile_png);
  ImageCrop(&tile_img, (Rectangle){0, 0, tile_size, tile_size});
  tile_texture = LoadTextureFromImage(tile_img);

  char* wall_background_png = "../resources/wall_background.png";
  // char* wall_background_png = "./resources/wall_background.png";
  Image wall_background_img = LoadImage(wall_background_png);
  ImageCrop(&wall_background_img, (Rectangle){0, 0, tile_size, tile_size});
  wall_background_texture = LoadTextureFromImage(wall_background_img);

  patterns[0] =
      (Pattern){{(MapTile){tile_texture, 0, true}, (MapTile){wall_texture, 0, true},
                 (MapTile){wall_texture, 0, true}, (MapTile){tile_texture, 0, true}},

                {(MapTile){tile_texture, 0, true}, (MapTile){tile_texture, 0, true},
                 (MapTile){tile_texture, 0, true}, (MapTile){tile_texture, 0, true}}};

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
  CreateRoom(&map, (Rec){10, 0, 4, 3});
  CreateRoom(&map, (Rec){10, 3, 4, 4});
  BreakWallsOnMap(&map);

  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    MovePlayer(&player, &camera, &map);
    BeginDrawing();

    ClearBackground(RAYWHITE);

    BeginMode2D(camera);

    for (int my = 0; my < map_max_height; my++) {
      for (int mx = 0; mx < map_max_width; mx++) {
        DrawTexturePro(
            map.arr[my][mx].texture,
            (Rectangle){0, 0, map.arr[my][mx].texture.width, map.arr[my][mx].texture.height},
            (Rectangle){(mx + 1 / 2.0) * tile_size, (my + 1 / 2.0) * tile_size, tile_size,
                        tile_size},
            (Vector2){tile_size / 2.0, tile_size / 2.0}, map.arr[my][mx].rot * 90, WHITE);
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