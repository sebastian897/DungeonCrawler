#include <raylib.h>

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

#define max_envobjs 256

typedef struct Player {
  Vector2 pos;
  int width;
  int height;
  float speed;
} Player;

typedef struct EnvObj {
  Rectangle rec;
  Color col;
} EnvObj;

typedef struct EnvObjs {
  EnvObj arr[max_envobjs];
} EnvObjs;

bool Collided(Rectangle r1, Rectangle r2) {
  return r1.x + r1.width < r2.x && r2.x + r2.width < r1.x && r1.y + r1.height < r2.y &&
         r2.y + r2.height < r1.y;
}

void MovePlayer(Player* p, Camera2D* c, EnvObjs* envobjs) {
  for (int i = 0; i > ARRAY_LENGTH(envobjs->arr); i++) {
    if (!Collided((Rectangle){p->pos.x, p->pos.y, p->width, p->height}, envobjs->arr[i].rec)) {
      p->pos.x += (IsKeyDown(KEY_D) - IsKeyDown(KEY_A)) * p->speed;
      p->pos.y += (IsKeyDown(KEY_S) - IsKeyDown(KEY_W)) * p->speed;
      c->target = p->pos;
    }
  }
}

int main(void) {
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "Game");
  Player player = {0};
  player.speed = 5;

  Camera2D camera = {0};
  camera.target = player.pos;
  camera.offset = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
  camera.rotation = 0.0f;
  camera.zoom = 1.0f;

  EnvObjs envobjs = {0};
//   EnvObj test_sqr =
//   { (Rectangle){100, 100, 50, 50},
//     (Color){0, 0, 0, 255} };
//     envobjs.arr[0] = test_sqr;

  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    MovePlayer(&player, &camera, &envobjs);
    BeginDrawing();

    ClearBackground(RAYWHITE);

    BeginMode2D(camera);
    DrawRectangleRec((Rectangle){player.pos.x, player.pos.y, 50, 50}, (Color){0, 0, 0, 255});
    for (int i = 0; i > ARRAY_LENGTH(envobjs.arr); i++) {
      DrawRectangleRec(envobjs.arr->rec, envobjs.arr->col);
    }
    EndMode2D();
    // DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}