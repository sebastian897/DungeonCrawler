#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "map.h"
#include "player.h"
#include "raymath.h"
#include "resources.h"
#include "types.h"

bool debug = false;

void RenderTiles(const Map* map, const Camera2D* camera, int screenWidth, int screenHeight) {
  // culling to the viewport to reduce CPU load
  Vector2 top_left = GetScreenToWorld2D((Vector2){0, 0}, *camera);
  Vector2 bottom_right = GetScreenToWorld2D((Vector2){screenWidth, screenHeight}, *camera);
  int minx = (int)(top_left.x / tile_size) - 1;
  int miny = (int)(top_left.y / tile_size) - 1;
  int maxx = (int)(bottom_right.x / tile_size) + 1;
  int maxy = (int)(bottom_right.y / tile_size) + 1;

  minx = Clamp(minx, 0, map_max_width - 1);
  maxx = Clamp(maxx, 0, map_max_width - 1);
  miny = Clamp(miny, 0, map_max_height - 1);
  maxy = Clamp(maxy, 0, map_max_height - 1);

  // now render unculled area
  for (int my = miny; my <= maxy; my++) {
    for (int mx = minx; mx <= maxx; mx++) {
      MapTile* tile = &tiles[map->arr[my][mx]];
      Texture2D* tex = &textures[tile->texture];
      Vector2 tile_pos = {mx, my};
      DrawTextureRec(*tex, (Rectangle){0, 0, tex->width, tex->height}, GetTilePos(tile_pos), WHITE);
    }
  }
}

void RenderPlayer(const Player* player) {
  for (int adp = 0; adp < ARRAY_LENGTH(anim_draw_priority); adp++) {
    int at = (int)anim_draw_priority[adp];
    const Animation a = player->character.anims[at];
    const Texture2D tex = a.anim_setups[a.anim_action].sprite_sheet;
    const Rectangle source = (Rectangle){a.curr_sprite * tex.height, 0, tex.height, tex.height};
    const Rectangle dest = {player->rec.pos.x + (player->rec.size.width) / 2.0,
                            player->rec.pos.y + (player->rec.size.height) / 2.0, tex.height,
                            tex.height};
    const Vector2 origin = {dest.width / 2, dest.height / 2};  // correct
    DrawTexturePro(tex, source, dest, origin, a.rot * RAD2DEG, WHITE);
  }
}

void MakeRotatedTextures(ResourceID img_res_id, texture_type text_idx_base) {
  Image img = ResourceImage(img_res_id);
  for (int rot = 0; rot < 4; rot++) {
    textures[text_idx_base + rot] = LoadTextureFromImage(img);
    if (rot == 3) break;  // save an unnecessary rotate
    ImageRotateCW(&img);
  }
  UnloadImage(img);
}

void EquipWeapon() {
}

int main(void) {
  InitWindow(0, 0, "Game");

  const int screenWidth = GetMonitorWidth(GetCurrentMonitor());
  const int screenHeight = GetMonitorHeight(GetCurrentMonitor());

  MakeRotatedTextures(RES_WALL, tex_wall_top);
  MakeRotatedTextures(RES_WALL_OUTSIDE_CORNER, tex_outer_corner_topleft);
  MakeRotatedTextures(RES_WALL_INSIDE_CORNER, tex_inner_corner_topleft);

  textures[tex_empty] = ResourceTexture(RES_WALL_BACKGROUND);
  textures[tex_floor] = ResourceTexture(RES_FLOOR);

  Character chars[num_chars] = {
      (Character){
          "Wizard",
          {[at_body] =
               (Animation){0,
                           0,
                           0,
                           0,
                           as_idle,
                           aa_walking,
                           {(AnimationSetup){ResourceTexture(RES_WIZARD), 0, 2, 5, 1, 8, true},
                            (AnimationSetup){ResourceTexture(RES_WIZARD), 0, 2, 5, 1, 8, true}}},
           [at_hands] =
               (Animation){
                   0,
                   0,
                   0,
                   0,
                   as_idle,
                   aa_walking,
                   {(AnimationSetup){ResourceTexture(RES_HANDS_WALKING), 0, 2, 5, 1, 8, true},
                    (AnimationSetup){}}},
           [at_effect] = (Animation){0}}}};

  Weapon weapons[] = {(Weapon){
      (AnimationSetup){ResourceTexture(RES_WIZARD_PRIMARY_EFFECT_ATTACKING), 0, 0, 9, 0, 3, true},
      (AnimationSetup){ResourceTexture(RES_HANDS_ATTACKING), 0, 0, 9, 0, 3, true}, false}};
  Player player = {0};
  player.rec.pos = (Vector2){300, 300};
  player.rec.size = (Size){34, 34};
  player.speed = (Vector2){0};
  player.walking_force = 1;
  player.character = chars[0];
  player.weapons[0] = weapons[0];
  player.character.anims[at_hands].anim_setups[1] = player.weapons[0].hand_attacking_anim;
  player.character.anims[at_effect].anim_setups[1] = player.weapons[0].effect_attacking_anim;
  // player.hitbox.rec = (Rec){{14, 14}, {34, 34}};

  Camera2D camera = {0};
  camera.offset = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
  camera.rotation = 0.0f;
  camera.zoom = 2.0f;
  SetCameraPos(&camera, player.rec.pos);
  player.cam = camera;

  Map map = {0};
  CreateRoom(&map, (Rec){{0, 0}, {10, 10}});
  CreateRoom(&map, (Rec){{9, 3}, {6, 4}});
  CreateRoom(&map, (Rec){{14, 0}, {10, 10}});
  CreateRoom(&map, (Rec){{18, 9}, {4, 5}});
  BreakWalls(&map);
  PrettyTiles(&map);

  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    PlayerMove(&player, &map);
    AnimatePlayer(&player);
    PlayerAttack(&player);
    BeginDrawing();
    ClearBackground(RAYWHITE);
    BeginMode2D(player.cam);
    RenderTiles(&map, &player.cam, screenWidth, screenHeight);
    RenderPlayer(&player);
    EndMode2D();
    const char* hand_anim_text = TextFormat("%f %f", player.speed.x, player.speed.y);
    DrawText(hand_anim_text, 0, 0, tile_size, RED);
    //     const char* body_anim_text =
    //     TextFormat("%d %d %d",
    //                player.character.body_anim.anim_action,
    //                player.character.body_anim.curr_sprite,
    //                player.character.body_anim.anim_state);
    // DrawText(body_anim_text, 0, 100, tile_size, RED);
    EndDrawing();
  }
  for (int i = 0; i < ARRAY_LENGTH(textures); i++) UnloadTexture(textures[i]);
  CloseWindow();
  return 0;
}
