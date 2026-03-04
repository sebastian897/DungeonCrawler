#include <math.h>
#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "raymath.h"

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

#define map_max_width 64
#define map_max_height 64
#define structure_max_width 16
#define structure_max_height 16
enum { tile_size = 64, hallway_size = 2, num_chars = 1 };
Texture2D wall_texture;
Texture2D wall_corner_texture;
Texture2D tile_texture;

typedef struct Animation {
  int curr_sprite;
  int anim_speed;
  Vector2 facing;
} Animation;

typedef struct Character {
  char* name;
  Texture2D sprite_sheet;
  int stil_sprite;
  int start_walk_sprite;
  int end_walk_sprite;
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

typedef struct ScreenPos {
  int x;
  int y;
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

ScreenPos Vector2ToScreenPos(V2 v) {
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
  return (PBetween2P(r1.x, r2.x, r2.x + tile_size) ||
          PBetween2P(r1.x + tile_size, r2.x, r2.x + tile_size)) &&
         (PBetween2P(r1.y, r2.y, r2.y + tile_size) ||
          PBetween2P(r1.y + tile_size, r2.y, r2.y + tile_size));
}

int rotation[4] = {0, 1, 2, 3};
void MovePlayer(Player* p, Camera2D* c, Map* map) {
  V2 dir = {IsKeyDown(KEY_D) - IsKeyDown(KEY_A), IsKeyDown(KEY_S) - IsKeyDown(KEY_W)};
  int newx = p->pos.x + dir.x * p->speed;
  int newy = p->pos.y + dir.y * p->speed;
  bool xcol = false;
  bool ycol = false;
  for (int my = 0; my < map_max_height; my++) {
    for (int mx = 0; mx < map_max_width; mx++) {
      if (!map->arr[my][mx].can_col) continue;
      if (Collided((ScreenPos){newx, p->pos.y}, Vector2ToScreenPos((V2){mx, my}))) xcol = true;
      if (Collided((ScreenPos){p->pos.x, newy}, Vector2ToScreenPos((V2){mx, my}))) ycol = true;
    }
  }
  if (!xcol) p->pos.x = newx;
  if (!ycol) p->pos.y = newy;
  if (!(dir.x == 0 && dir.y == 0)) p->character.anim.facing = (Vector2){dir.x, dir.y};
  c->target = p->pos;
}

void RecSqrs(Structure* map, Rec rec, Texture2D texture, float rot, bool can_col) {
  for (int y = 0; y < rec.height; y++) {
    for (int x = 0; x < rec.width; x++) {
      map->arr[rec.y + y][rec.x + x] = (MapTile){texture, rot, can_col};
    }
  }
}

int texture[4] = {2, 0, 1, 3};
int rx[4] = {0, 1, 1, 0};
int ry[4] = {0, 0, 1, 1};

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
    Rec new_wall = {rec.x, rec.y, rec.width - 1, 1};
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

int main(void) {
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "Game");

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

  Character Chars[num_chars] = {
      (Character){"Wizard", LoadTexture("../resources/wizard.png"), 0, 2, 5}};

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
  camera.zoom = 1.0f;

  Map map = {0};
  CreateRoom(&map, (Rec){0, 0, 10, 10});
  CreateRoom(&map, (Rec){10, 3, 4, 4});

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
    DrawTexturePro(
        player.character.sprite_sheet,
        (Rectangle){player.character.anim.curr_sprite * player.character.sprite_sheet.width, 0,
                    player.width, player.height},
        (Rectangle){player.pos.x + player.width / 2.0, player.pos.y + player.height / 2.0,
                    player.width, player.height},
        (Vector2){player.width / 2.0, player.height / 2.0},
        (atan2(player.character.anim.facing.y, player.character.anim.facing.x) + PI / 2) * RAD2DEG,
        WHITE);
    // DrawRectangleRec((Rectangle){player.pos.x, player.pos.y, player.width, player.height},
    //                  (Color){0, 0, 0, 255});
    EndMode2D();

    EndDrawing();
  }
  for (int t = 0; t < 4; t++) UnloadTexture(wall_texture);
  CloseWindow();
  return 0;
}