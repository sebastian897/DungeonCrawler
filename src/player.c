#include "player.h"

#include <math.h>
#include <stdbool.h>

#include "map.h"
#include "raylib.h"
#include "types.h"

anim_type anim_draw_priority[] = {[at_hands] = 0, [at_effect] = 1, [at_body] = 2};

static bool PBetween2P(float p, float p2l, float p2h) {
  return p >= p2l && p <= p2h;
}

static bool Collided(Rec r1, Rec r2) {
  return (PBetween2P(r1.pos.x, r2.pos.x, r2.pos.x + r2.size.width - 1) ||
          PBetween2P(r1.pos.x + r1.size.width - 1, r2.pos.x, r2.pos.x + r2.size.width - 1)) &&
         (PBetween2P(r1.pos.y, r2.pos.y, r2.pos.y + r2.size.height - 1) ||
          PBetween2P(r1.pos.y + r1.size.height - 1, r2.pos.y, r2.pos.y + r2.size.height - 1));
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

static V2 GetMousePos() {
  return (V2){GetMouseX(), GetMouseY()};
}

void SetCameraPos(Camera2D* cam, V2 p_pos) {
  if (p_pos.x - cam->offset.x / cam->zoom < 0)
    cam->target.x = cam->offset.x / cam->zoom;
  else if (p_pos.x + cam->offset.x / cam->zoom > map_max_width * tile_size)
    cam->target.x = -cam->offset.x / cam->zoom;
  else
    cam->target.x = p_pos.x;
  if (p_pos.y - cam->offset.y / cam->zoom < 0)
    cam->target.y = cam->offset.y / cam->zoom;
  else if (p_pos.y + cam->offset.y / cam->zoom > map_max_height * tile_size)
    cam->target.y = -cam->offset.y / cam->zoom;
  else
    cam->target.y = p_pos.y;
}

void PlayerMove(Player* p, Camera2D* c, Map* map) {
  V2 dir_vec = {IsKeyDown(KEY_D) - IsKeyDown(KEY_A), IsKeyDown(KEY_S) - IsKeyDown(KEY_W)};
  bool is_moving = !(dir_vec.x == 0 && dir_vec.y == 0);
  if (is_moving) {
    float mag = p->speed;
    float rot = atan2(dir_vec.y, dir_vec.x);
    p->rot = rot;
    V2 new_pos = {p->rec.pos.x + mag * ROUND3DP(cos(rot)), p->rec.pos.y + mag * ROUND3DP(sin(rot))};
    bool x_col_tile = false;
    bool y_col_tile = false;
    V2 col_tile = {-1, -1};
    for (int my = 0; my < map_max_height; my++) {
      for (int mx = 0; mx < map_max_width; mx++) {
        if (!tiles[map->arr[my][mx]].can_col) continue;
        V2 tile = {mx, my};
        bool xcol =
            Collided((Rec){{new_pos.x, p->rec.pos.y}, p->rec.size}, GetTileRec(GridPosToPos(tile)));
        bool ycol =
            Collided((Rec){{p->rec.pos.x, new_pos.y}, p->rec.size}, GetTileRec(GridPosToPos(tile)));
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
      p->rec.pos.x = new_pos.x;
    } else if (col_tile.x >= 0) {
      p->rec.pos.x = GridPosToPos(col_tile).x;
    }
    if (!y_col_tile) {
      p->rec.pos.y = new_pos.y;
    } else if (col_tile.y >= 0) {
      p->rec.pos.y = GridPosToPos(col_tile).y;
    }
    SetCameraPos(c, new_pos);
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

static void RotAttackAnim(Player* player) {
  V2 mouse_pos = GetMousePos();
  V2 player_screenpos = Vector2ToV2(player->cam.offset);
  V2 dir = SubV2(mouse_pos, player_screenpos);
  float rot = atan2(dir.y, dir.x);
  for (int at = 0; at < ARRAY_LENGTH(anim_types); at++) {
    if (player->character.anims[at].anim_action == aa_attacking)
      player->character.anims[at].rot = rot;
  }
}

void AnimatePlayer(Player* player) {
  for (int at = 0; at < ARRAY_LENGTH(anim_types); at++) {
    GoToNextSprite(player, &player->character.anims[at]);
    RotAttackAnim(player);
  }
}
