#include <stdbool.h>
#include <math.h>

#include "player.h"
#include "map.h"

static bool PBetween2P(float p, float p2l, float p2h) {
  return p >= p2l && p <= p2h;
}

static bool Collided(ScreenPos r1, ScreenPos r2) {
  return (PBetween2P(r1.x, r2.x, r2.x + tile_size - 1) ||
          PBetween2P(r1.x + tile_size - 1, r2.x, r2.x + tile_size - 1)) &&
         (PBetween2P(r1.y, r2.y, r2.y + tile_size - 1) ||
          PBetween2P(r1.y + tile_size - 1, r2.y, r2.y + tile_size - 1));
}

static void GoToNextSprite(Animation* anim) {
  anim->anim_frame_counter++;
  if (anim->anim_frame_counter >= anim->anim_setups[anim->anim_action].anim_speed) {
    anim->anim_frame_counter = 0;
    if (anim->anim_state == as_active) {
      if (anim->curr_sprite == anim->anim_setups[anim->anim_action].last_active_sprite) {
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
      else if (anim->curr_sprite > anim->anim_setups[anim->anim_action].initial_sprite)
        anim->curr_sprite -= 1;
      else {
        anim->anim_state = as_idle;
      }
    }
    if (anim->anim_state == as_idle) {
      anim->anim_action = aa_walking;
    }
  }
}

static V2 GetMousePos() {
  return (V2){GetMouseX(), GetMouseY()};
}

int rotation[4] = {0, 1, 2, 3};
void PlayerMove(Player* p, Camera2D* c, Map* map) {
  V2 dir_vec = {IsKeyDown(KEY_D) - IsKeyDown(KEY_A), IsKeyDown(KEY_S) - IsKeyDown(KEY_W)};
  // V2 dir_vec = {-1, 1};
  bool is_moving = !(dir_vec.x == 0 && dir_vec.y == 0);
  if (is_moving) {
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

    c->target = p->pos;
    if (p->character.body_anim.anim_action == aa_walking) {
      p->character.body_anim.anim_state = as_active;
      p->character.body_anim.rot = rot;
    }
    if (p->character.hand_anim.anim_action == aa_walking) {
      p->character.hand_anim.rot = rot;
      p->character.hand_anim.anim_state = as_active;
    }
  } else {
    if (p->character.body_anim.anim_action == aa_walking)
      p->character.body_anim.anim_state = as_inactive;
    if (p->character.hand_anim.anim_action == aa_walking)
      p->character.hand_anim.anim_state = as_inactive;
  }
}

void PlayerAttack(Player* player) {
  bool is_attacking = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
  if (is_attacking) {
    player->character.hand_anim.anim_action = aa_attacking;
    player->character.hand_anim.anim_state = as_active;
    player->weapon.effect_anim.anim_action = aa_attacking;
    player->weapon.effect_anim.anim_state = as_active;
    // StartAnimation(&player->character.hand_anim, player->weapon.hand_attacking_anim);
  }
}

static void RotAttackAnim(Player* player) {
  V2 mouse_pos = GetMousePos();
  V2 player_screenpos = Vector2ToV2(player->cam.offset);
  V2 dir = SubV2(mouse_pos, player_screenpos);
  float rot = atan2(dir.y, dir.x);

  if (player->character.hand_anim.anim_action == aa_attacking)
    player->character.hand_anim.rot = rot;
  if (player->weapon.effect_anim.anim_action == aa_attacking) player->weapon.effect_anim.rot = rot;
}

void AnimatePlayer(Player* player) {
  GoToNextSprite(&player->character.body_anim);
  GoToNextSprite(&player->character.hand_anim);
  GoToNextSprite(&player->weapon.effect_anim);
  RotAttackAnim(player);
}


