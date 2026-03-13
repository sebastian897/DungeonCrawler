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
  num_chars = 1
};

typedef struct V2 {
  int x;
  int y;
} V2;

typedef struct Size {
  int width;
  int height;
} Size;


typedef struct Rec {
  V2 pos;
  Size size;
} Rec;

typedef struct ScreenPos {
  float x;
  float y;
} ScreenPos;

// V2 GetPosOfRec(Rec rec);
V2 AddV2(V2 v1, V2 v2);
V2 SubV2(V2 v1, V2 v2);
Size SubSize(Size s1, Size s2);
Vector2 V2ToVector2(V2 v);
V2 Vector2ToV2(Vector2 v);
ScreenPos V2ToScreenPos(V2 v);
