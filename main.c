#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>

#include "raymath.h"

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

#define map_max_width 256
#define map_max_height 256
enum { tile_size = 64, hallway_size = 2 };
Texture2D wall_texture;
Texture2D wall_corner_texture;
Texture2D tile_texture;

typedef struct Player {
  Vector2 pos;
  int width;
  int height;
  float speed;
} Player;

typedef struct MapTile {
  Vector2 pos;
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

Vector2 GetPosOfRec(Rec rec) {
  return (Vector2){rec.x, rec.y};
}

bool PBetween2P(float p, float p2l, float p2h) {
  return p >= p2l && p <= p2h;
}

bool Collided(Vector2 r1, Vector2 r2) {
  return (PBetween2P(r1.x, r2.x, r2.x + tile_size) ||
          PBetween2P(r1.x + tile_size, r2.x, r2.x + tile_size)) &&
         (PBetween2P(r1.y, r2.y, r2.y + tile_size) ||
          PBetween2P(r1.y + tile_size, r2.y, r2.y + tile_size));
}

void MovePlayer(Player* p, Camera2D* c, Map* map) {
  float newx = p->pos.x + (IsKeyDown(KEY_D) - IsKeyDown(KEY_A)) * p->speed;
  float newy = p->pos.y + (IsKeyDown(KEY_S) - IsKeyDown(KEY_W)) * p->speed;
  bool xcol = false;
  bool ycol = false;
  for (int my = 0; my < map_max_height; my++) {
    for (int mx = 0; mx < map_max_width; mx++) {
      if (!map->arr[my][mx].can_col) continue;
      if (Collided((Vector2){newx, p->pos.y}, map->arr[my][mx].pos)) xcol = true;
      if (Collided((Vector2){p->pos.x, newy}, map->arr[my][mx].pos)) ycol = true;
    }
  }
  if (!xcol) p->pos.x = newx;
  if (!ycol) p->pos.y = newy;
  c->target = p->pos;
}

void RecSqrs(Map* map, Rec rec, Texture2D texture, float rot, bool can_col) {
  for (int y = 0; y < rec.height; y++) {
    for (int x = 0; x < rec.width; x++) {
      map->arr[rec.x + x][rec.y + y] =
          (MapTile){(Vector2){rec.x + x, rec.y + y}, texture, rot, can_col};
    }
  }
}

int texture[4] = {2, 0, 1, 3};
int corner_texture[4] = {0, 1, 2, 3};
int rx[4] = {0, 1, 1, 0};
int ry[4] = {0, 0, 1, 1};

void AddObjsToEnvObjs(Map* map, Map newenvobjs) {
  for (int my = 0; my < map_max_height; my++) {
    for (int mx = 0; mx < map_max_width; mx++) {
      map->arr[mx][my] = newenvobjs.arr[mx][my];
    }
  }
}
void RotateEnvObjs(Map* map, Vector2 origin, int rot) {
  for (int my = 0; my < map_max_height; my++) {
    for (int mx = 0; mx < map_max_width; mx++) {
      Vector2 new_pos = Vector2Rotate(Vector2Subtract(map->arr[mx][my].pos, origin), PI / 2 * rot);
      map->arr[mx][my].pos = (Vector2){origin.x + new_pos.x,origin.y + new_pos.y};
      map->arr[mx][my].rot = (map->arr[mx][my].rot + rot) % 4;
    }
  }
}

void Wall(Map* wallobjs, Rec rec) {
  RecSqrs(wallobjs, (Rec){rec.x + tile_size, rec.y, rec.width, 1}, wall_texture, texture[0], true);
  wallobjs->arr[rec.x][rec.y] =
      (MapTile){(Vector2){rec.x, rec.y}, wall_corner_texture, corner_texture[0], true};
}

void CreateRoom(Map* map, Rec rec) {
  for (int d = 0; d < 4; d++) {
    Map new_wall_objs = {0};
    Rec new_wall = {rec.x + (rx[d] * ((rec.width - 2) + 1)),
                    rec.y + (ry[d] * ((rec.height - 2) + 1)), rec.width - 2, 1};
    Wall(&new_wall_objs, new_wall);
    RotateEnvObjs(&new_wall_objs, GetPosOfRec(new_wall), d);
    AddObjsToEnvObjs(map, new_wall_objs);
  }
  RecSqrs(map, (Rec){rec.x + tile_size, rec.y + tile_size, (rec.width - 2), (rec.height - 2)},
          tile_texture, 0, false);
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

  Player player = {0};
  player.pos.x = 150;
  player.pos.y = 150;
  player.width = 64;
  player.height = 64;
  player.speed = 5;

  Camera2D camera = {0};
  camera.target = player.pos;
  camera.offset = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
  camera.rotation = 0.0f;
  camera.zoom = 1.0f;

  Map map = {0};
  CreateRoom(&map, (Rec){0, 0, 10, 10});

  // Wall(&map, (Rec){0 , 0, 5,1});
  // map.arr[0] =
  //     (MapTile){(Rec){0 , 0, tile_size, tile_size},
  //              wall_texture, 0, false};
  //     map.count++;
  // RotateEnvObjs(&map, (Vector2){0,0}, 1);
  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    MovePlayer(&player, &camera, &map);
    BeginDrawing();

    ClearBackground(RAYWHITE);

    BeginMode2D(camera);

    for (int my = 0; my < map_max_height; my++) {
      for (int mx = 0; mx < map_max_width; mx++) {
        DrawTexturePro(map.arr[mx][my].texture, (Rectangle){0, 0, tile_size, tile_size},
                       (Rectangle){map.arr[mx][my].pos.x + rx[map.arr[mx][my].rot] * tile_size,
                                   map.arr[mx][my].pos.y + ry[map.arr[mx][my].rot] * tile_size,
                                   tile_size, tile_size},
                       (Vector2){0, 0}, map.arr[mx][my].rot * 90, WHITE);
      }
    }
    DrawRectangleRec((Rectangle){player.pos.x, player.pos.y, player.width, player.height},
                     (Color){0, 0, 0, 255});
    EndMode2D();

    EndDrawing();
  }
  for (int t = 0; t < 4; t++) UnloadTexture(wall_texture);
  CloseWindow();
  return 0;
}