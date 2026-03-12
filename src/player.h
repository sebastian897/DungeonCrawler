#pragma once
#include <raylib.h>

#include "types.h"

void PlayerMove(Player* p, Camera2D* c, Map* map);
void PlayerAttack(Player* player);
void AnimatePlayer(Player* player);
