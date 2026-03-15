#pragma once
#include <raylib.h>
#include <stdint.h>

#define ARRAY_LENGTH(x) ((int)(sizeof(x) / sizeof((x)[0])))
#define ROUND3DP(x) (round((x) * 1000.0) / 1000.0)

enum {
  map_max_width = 64,
  map_max_height = 64,
  tile_size = 64,
  structure_max_width = 16,
  structure_max_height = 16,
  hallway_size = 2,
  num_chars = 1,
  num_weapons = 1,
};

typedef struct V2I {
  int x;
  int y;
} V2I;

typedef struct Size {
  int width;
  int height;
} Size;

typedef struct Rec {
  Vector2 pos;
  Size size;
} Rec;

// typedef struct ScreenPos {
//   float x;
//   float y;
// } ScreenPos;

// Vector2 GetPosOfRec(Rec rec);
Size GetTileSize();
Rec GetTileHitbox(Vector2 pos);
Vector2 GetTilePos(Vector2 v);
Vector2 GetPosOfRec(Rec rec);
Size GetSizeOfRec(Rec rec);
V2I V2IAdd(V2I v1, V2I v2);
V2I V2ISub(V2I v1, V2I v2);
V2I Vector2ToV2I(Vector2 v);
Vector2 V2IToVector2(V2I v);
Size SizeAdd(Size v1, Size v2);
Size SizeSub(Size v1, Size v2);
Rec TransformRec(Rec rec, Vector2 pos, Size size);
Rectangle RecToRectangle(Rec rec);
Vector2 GridPosToPos(V2I v);
Vector2 GetRecCenter(Rec rec);
// Vector2 AddVector2(Vector2 v1, Vector2 Vector2);
// Vector2 SubVector2(Vector2 v1, Vector2 Vector2);
Size SubSize(Size s1, Size s2);
// Vector2 Vector2ToVector2(Vector2 v);
// Vector2 Vector2ToVector2(Vector2 v);
// ScreenPos Vector2ToScreenPos(Vector2 v);
