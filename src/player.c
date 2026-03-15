#include "player.h"

#include <math.h>
#include <stdbool.h>

#include "map.h"
#include "raylib.h"
#include "raymath.h"
#include "types.h"

anim_type anim_draw_priority[] = {at_hands, at_effect, at_body};

// static bool Collided(Rec r1, Rec r2) {
//   return r1.x < r2.x + r2.width && r1.x + r1.width > r2.x && r1.y < r2.y + r2.height &&
//          r1.y + r1.height > r2.y;
// }

Vector2 GetPosOfCollidedObj(Rec stat_hb, Rec mov_hb, V2I dir) {
  Vector2 new_pos = mov_hb.pos;
  if (dir.x < 0)
    new_pos.x = stat_hb.pos.x + stat_hb.size.width;
  else if (dir.x > 0)
    new_pos.x = stat_hb.pos.x - mov_hb.size.width;

  if (dir.y < 0)
    new_pos.y = stat_hb.pos.y + stat_hb.size.height;
  else if (dir.y > 0)
    new_pos.y = stat_hb.pos.y - mov_hb.size.height;
  return new_pos;
}

bool CheckColRecs(Rec rec1, Rec rec2) {
  return ((rec1.pos.x < (rec2.pos.x + rec2.size.width) &&
           (rec1.pos.x + rec1.size.width) > rec2.pos.x) &&
          (rec1.pos.y < (rec2.pos.y + rec2.size.height) &&
           (rec1.pos.y + rec1.size.height) > rec2.pos.y));
}

static void GoToNextSprite(Player* p, Animation* anim) {
  anim->anim_frame_counter++;
  if (anim->anim_frame_counter >= anim->anim_setups[anim->anim_action].anim_speed) {
    anim->anim_frame_counter = 0;
    if (anim->anim_state == as_active) {
      if (anim->curr_sprite >= anim->anim_setups[anim->anim_action].last_active_sprite) {
        if (anim->anim_action == aa_attacking)
          anim->anim_state = as_inactive;
        else
          anim->curr_sprite = anim->anim_setups[anim->anim_action].first_active_sprite;
      } else {
        anim->curr_sprite += 1;
      }
    }
    if (anim->anim_state == as_inactive) {
      if (anim->curr_sprite >= anim->anim_setups[anim->anim_action].first_active_sprite)
        anim->curr_sprite = anim->anim_setups[anim->anim_action].first_inactive_sprite;
      if (anim->curr_sprite > anim->anim_setups[anim->anim_action].initial_sprite)
        anim->curr_sprite -= 1;
      if (anim->curr_sprite <= anim->anim_setups[anim->anim_action].initial_sprite)
        anim->anim_state = as_idle;
    }
    if (anim->anim_state == as_idle) {
      anim->curr_sprite = anim->anim_setups[aa_walking].initial_sprite;
      anim->anim_action = aa_walking;
      anim->anim_state = as_inactive;
      anim->anim_frame_counter = 0;
      anim->rot = p->rot;
    }
  }
}

void SetCameraPos(Camera2D* cam, Vector2 p_pos) {
  int map_width = map_max_width * tile_size;
  int map_height = map_max_height * tile_size;
  if (p_pos.x < cam->offset.x / cam->zoom)
    cam->target.x = cam->offset.x / cam->zoom;
  else if (p_pos.x + cam->offset.x / cam->zoom > map_width)
    cam->target.x = map_width - cam->offset.x / cam->zoom;
  else
    cam->target.x = p_pos.x;

  if (p_pos.y < cam->offset.y / cam->zoom)
    cam->target.y = cam->offset.y / cam->zoom;
  else if (p_pos.y + cam->offset.y / cam->zoom > map_height)
    cam->target.y = map_height - cam->offset.y / cam->zoom;
  else
    cam->target.y = p_pos.y;
}

void PlayerMove(Player* p, Map* map) {
  V2I dir_vec = {IsKeyDown(KEY_D) - IsKeyDown(KEY_A), IsKeyDown(KEY_S) - IsKeyDown(KEY_W)};
  bool is_moving = !(dir_vec.x == 0 && dir_vec.y == 0);
  if (is_moving) {
    float mag = p->speed;
    float rot = atan2(dir_vec.y, dir_vec.x);
    p->rot = rot;
    Vector2 new_pos = {p->rec.pos.x + mag * ROUND3DP(cos(rot)),
                       p->rec.pos.y + mag * ROUND3DP(sin(rot))};
    bool x_col_tile = false;
    bool y_col_tile = false;
    V2I col_tile = {-1, -1};
    Vector2 new_player_pos_x = {new_pos.x, p->rec.pos.y};
    Vector2 new_player_pos_y = {p->rec.pos.x, new_pos.y};
    for (int my = 0; my < map_max_height; my++) {
      for (int mx = 0; mx < map_max_width; mx++) {
        V2I tile = {mx, my};
        if (!tiles[map->arr[tile.y][tile.x]].can_col) continue;
        Rec tile_hitbox = GetTileHitbox(GetTilePos(V2IToVector2(tile)));
        Vector2 pos_before_col = GetPosOfCollidedObj(tile_hitbox, p->rec, dir_vec);
        if (CheckColRecs((Rec){new_player_pos_x, p->rec.size}, tile_hitbox)) {
          x_col_tile = true;
          col_tile.x = pos_before_col.x;
        }
        if (CheckColRecs((Rec){new_player_pos_y, p->rec.size}, tile_hitbox)) {
          y_col_tile = true;
          col_tile.y = pos_before_col.y;
        }
      }
    }
    Vector2 new_pos_after_col = p->rec.pos;
    if (!x_col_tile) {
      new_pos_after_col.x = new_pos.x;
    } else if (col_tile.x >= 0) {
      new_pos_after_col.x = col_tile.x;
    }
    if (!y_col_tile) {
      new_pos_after_col.y = new_pos.y;
    } else if (col_tile.y >= 0) {
      new_pos_after_col.y = col_tile.y;
    }
    p->rec.pos = new_pos_after_col;
    SetCameraPos(&p->cam, new_pos_after_col);
    for (int at = 0; at < ARRAY_LENGTH(anim_types); at++) {
      if (p->character.anims[at].anim_action == aa_walking) {
        p->character.anims[at].anim_state = as_active;
        p->character.anims[at].rot = p->rot;
      }
    }
  } else {
    for (int at = 0; at < ARRAY_LENGTH(anim_types); at++) {
      if (p->character.anims[at].anim_action == aa_walking)
        p->character.anims[at].anim_state = as_inactive;
    }
  }
}

void PlayerAttack(Player* player) {
  bool is_attacking = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
  if (is_attacking) {
    for (int at = 0; at < ARRAY_LENGTH(anim_types); at++) {
      if (at > 0 && player->character.anims[at].anim_action != aa_attacking) {
        player->character.anims[at].anim_frame_counter = 0;
        player->character.anims[at].anim_action = aa_attacking;
        player->character.anims[at].anim_state = as_active;
      }
    }
  }
}

static void RotAttackAnim(Player* player, Animation* anim) {
  if (anim->anim_action != aa_attacking) return;
  Vector2 mouse_world = GetScreenToWorld2D(GetMousePosition(), player->cam);
  Vector2 dir = Vector2Subtract(mouse_world, GetRecCenter(player->rec));
  float rot = atan2(dir.y, dir.x);
  anim->rot = rot;
}

void AnimatePlayer(Player* player) {
  for (int at = 0; at < ARRAY_LENGTH(anim_types); at++) {
    GoToNextSprite(player, &player->character.anims[at]);
    RotAttackAnim(player, &player->character.anims[at]);
  }
}
