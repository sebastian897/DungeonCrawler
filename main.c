#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

#define max_envobjs 256
enum { tile_size = 65 };
Texture2D wall_texture;
Texture2D wall_corner_texture;
Texture2D tile_texture;

typedef struct Player {
  Vector2 pos;
  int width;
  int height;
  float speed;
} Player;

typedef struct EnvObj {
  Rectangle rec;
  Texture2D texture;
  int rot;
  bool can_col;
} EnvObj;

typedef struct EnvObjs {
  EnvObj arr[max_envobjs];
  int count;
} EnvObjs;

bool PBetween2P(float p, float p2l, float p2h) {
  return p >= p2l && p <= p2h;
}

bool Collided(Rectangle r1, Rectangle r2) {
  return (PBetween2P(r1.x, r2.x, r2.x + r2.width) ||
          PBetween2P(r1.x + r1.width, r2.x, r2.x + r2.width)) &&
         (PBetween2P(r1.y, r2.y, r2.y + r2.height) ||
          PBetween2P(r1.y + r1.height, r2.y, r2.y + r2.height));
}

void MovePlayer(Player* p, Camera2D* c, EnvObjs* envobjs) {
  float newx = p->pos.x + (IsKeyDown(KEY_D) - IsKeyDown(KEY_A)) * p->speed;
  float newy = p->pos.y + (IsKeyDown(KEY_S) - IsKeyDown(KEY_W)) * p->speed;
  bool xcol = false;
  bool ycol = false;
  for (int i = 0; i < envobjs->count; i++) {
    if (!envobjs->arr[i].can_col) continue;
    if (Collided((Rectangle){newx, p->pos.y, p->width, p->height}, envobjs->arr[i].rec))
      xcol = true;
    if (Collided((Rectangle){p->pos.x, newy, p->width, p->height}, envobjs->arr[i].rec))
      ycol = true;
  }
  if (!xcol) p->pos.x = newx;
  if (!ycol) p->pos.y = newy;
  c->target = p->pos;
}

void AddRecSqrs(EnvObjs* envobjs, Rectangle rec, Texture2D texture, float rot, bool can_col) {
  for (int y = 0; y < rec.height; y++) {
    for (int x = 0; x < rec.width; x++) {
      envobjs->arr[envobjs->count] = (EnvObj){
          (Rectangle){rec.x + (x * tile_size), rec.y + (y * tile_size), tile_size, tile_size},
          texture, rot, can_col};
      envobjs->count++;
    }
  }
}

int ddx[4] = {1, 1, 0, 0};
int ddy[4] = {0, 0, 1, 1};
int dsx[4] = {0, 0, 0, 1};
int dsy[4] = {0, 1, 0, 0};
int texture[4] = {2, 0, 1, 3};
int corner_texture[4] = {0, 1, 2, 3};
int cx[4] = {0, 1, 1, 0};
int cy[4] = {0, 0, 1, 1};
int rx[4] = {0, 1, 1, 0};
int ry[4] = {0, 0, 1, 1};
void CreateRoom(Rectangle rec, EnvObjs* envobjs) {
  for (int d = 0; d < 4; d++) {
    AddRecSqrs(envobjs,
               (Rectangle){rec.x + (dsx[d] * (rec.width) + ddx[d]) * tile_size,
                           rec.y + (dsy[d] * (rec.height) + ddy[d]) * tile_size,
                           (ddx[d] * (rec.width - 2) + 1), (ddy[d] * (rec.height - 2) + 1)},
               wall_texture, texture[d], true);
    envobjs->arr[envobjs->count] =
        (EnvObj){(Rectangle){rec.x + (cx[d] * (rec.width)) * tile_size, rec.y + (cy[d] *
        (rec.height))*tile_size, tile_size, tile_size},
                 wall_corner_texture, corner_texture[d], false};
    envobjs->count++;
  }
  AddRecSqrs(envobjs,
             (Rectangle){rec.x + tile_size, rec.y + tile_size, (rec.width - 2), (rec.height - 2)},
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

  EnvObjs envobjs = {0};
  CreateRoom((Rectangle){0, 0, 10, 10}, &envobjs);

  // envobjs.arr[0] =
  //     (EnvObj){(Rectangle){0 , 0, tile_size, tile_size},
  //              wall_texture, 0, false};
  //     envobjs.count++;

  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    MovePlayer(&player, &camera, &envobjs);
    BeginDrawing();

    ClearBackground(RAYWHITE);

    BeginMode2D(camera);

    for (int i = 0; i < envobjs.count; i++) {
      DrawTexturePro(envobjs.arr[i].texture, (Rectangle){0, 0, tile_size, tile_size},
                     (Rectangle){envobjs.arr[i].rec.x + rx[envobjs.arr[i].rot] * tile_size,
                                 envobjs.arr[i].rec.y + ry[envobjs.arr[i].rot] * tile_size,
                                 tile_size, tile_size},
                     (Vector2){0, 0}, envobjs.arr[i].rot * 90, WHITE);
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