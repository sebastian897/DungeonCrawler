#pragma once
#include <raylib.h>

#include "types.h"
extern MapTile tiles[tt_count];
extern Texture2D textures[tex_count];

void CreateRoom(Map* map, Rec rec);
void PrettyTiles(Map* map);
void BreakWalls(Map* map);
void AddContentsToStruct(Structure* dest, Structure* source);
void ShiftStructure(Structure* dest, Structure* source, V2 v);
