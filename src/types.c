#include "types.h"

#include <math.h>

#include "raylib.h"
#include "raymath.h"

// Vector2 GetPosOfRec(Rec rec) {
//   return (Vector2){rec.x, rec.y};
// }

Size GetTileSize() {
  return (Size){tile_size, tile_size};
}

Vector2 GetTilePos(Vector2 v) {
  Size tile_size = GetTileSize();
  return (Vector2){v.x * tile_size.width, v.y * tile_size.height};
}

Rec GetTileHitbox(Vector2 pos) {
  Size size = GetTileSize();
  return (Rec){pos, size};
}

Rec TransformRec(Rec rec, Vector2 pos, Size size) {
  Vector2 new_pos = Vector2Add(rec.pos, pos);
  Size new_size = SizeAdd(rec.size, size);
  return (Rec){new_pos, new_size};
}

Rectangle RecToRectangle(Rec rec) {
  return (Rectangle){rec.pos.x, rec.pos.y, rec.size.width, rec.size.height};
}

Vector2 GridPosToPos(V2I v) {
  return (Vector2){v.x * tile_size, v.y * tile_size};
}

// Vector2 GetPosOfRec(Rec rec) {
//   return (Vector2){rec.x, rec.x};
// }

// Size GetSizeOfRec(Rec rec) {
//   return (Size){rec.width, rec.height};
// }

V2I Vector2ToV2(Vector2 v) {
  return (V2I){v.x, v.y};
}

Size SizeAdd(Size v1, Size v2) {
  return (Size){v1.width + v2.width, v1.height + v2.height};
}

Size SizeSub(Size v1, Size v2) {
  return (Size){v1.width - v2.width, v1.height - v2.height};
}

V2I V2IAdd(V2I v1, V2I v2) {
  return (V2I){v1.x + v2.x, v1.y + v2.y};
}

V2I V2ISub(V2I v1, V2I v2) {
  return (V2I){v1.x - v2.x, v1.y - v2.y};
}

Size SubSize(Size s1, Size s2) {
  return (Size){s1.width - s2.width, s1.height - s2.height};
}

V2I Vector2ToV2I(Vector2 v) {
  return (V2I){v.x, v.y};
}

Vector2 V2IToVector2(V2I v) {
  return (Vector2){v.x, v.y};
}

// ScreenPos Vector2ToScreenPos(Vector2 v) {
//   return (ScreenPos){v.x * tile_size, v.y * tile_size};
// }
