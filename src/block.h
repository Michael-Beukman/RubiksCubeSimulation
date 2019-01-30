#ifndef BLOCK_H
#define BLOCK_H
#include "assortedStructs.h"
#include <vector>
struct Face {
  triangle t1;
  triangle t2;

  void setColor(Pixel col) {
    t1.color = col;
    t2.color = col;
  }
  Pixel getColor() { return t1.color; }
};

struct Block {
  std::vector<Face> faces;
  std::vector<triangle> getAllTriangles() {
    std::vector<triangle> ts;
    for (auto &k : faces) {
      ts.push_back(k.t1);
      ts.push_back(k.t2);
    }
    return ts;
  }
};

#endif
