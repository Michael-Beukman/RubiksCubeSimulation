#ifndef ASSORTEDSTRUCTS_H
#define ASSORTEDSTRUCTS_H
#include "vec3d.h"
#include "gameEngine.h"
struct vec2d {
  double u = 0;
  double v = 0;
};
struct triangle {
  vec3d vertices[3];

  Pixel color = WHITE;
};
#endif