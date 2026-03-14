#include "types.h"

// V2 GetPosOfRec(Rec rec) {
//   return (V2){rec.x, rec.y};
// }

Size GetTileSize() {
  return (Size){tile_size, tile_size};
}

Rec GetTileRec(V2 pos) {
  return (Rec){pos, GetTileSize()};
}

V2 GridPosToPos(V2 v) {
  return (V2){v.x * tile_size, v.y * tile_size};
}

V2 AddV2(V2 v1, V2 v2) {
  return (V2){v1.x + v2.x, v1.y + v2.y};
}

V2 SubV2(V2 v1, V2 v2) {
  return (V2){v1.x - v2.x, v1.y - v2.y};
}

Size SubSize(Size s1, Size s2) {
  return (Size){s1.width - s2.width, s1.height - s2.height};
}

Vector2 V2ToVector2(V2 v) {
  return (Vector2){v.x, v.y};
}

V2 Vector2ToV2(Vector2 v) {
  return (V2){v.x, v.y};
}

ScreenPos V2ToScreenPos(V2 v) {
  return (ScreenPos){v.x * tile_size, v.y * tile_size};
}
