#ifndef RUBIKSCUBE_H
#define RUBIKSCUBE_H
#include "assortedStructs.h"
#include "block.h"
#include "gameEngine.h"
#include "utility.h"
#include <iostream>
#include <vector>
enum turnDirections {
  LEFT = 0,
  RIGHT = 1,
  DOWN = 2,
  UP = 3,
  FRONT = 4,
  BACK = 5,
  LEFT_REVERSE = 6,
  RIGHT_REVERSE = 7,
  DOWN_INDEX = 8,
  UP_REVERSE = 9,
  FRONT_REVERSE = 10,
  BACK_REVERSE = 11
};
struct cubePos {
  int face = 0;
  int x = 0;
  int y = 0;

  bool operator==(const cubePos &other) const{
    return face == other.face && x == other.x && y == other.y;
  }
  friend std::ostream &operator<<(std::ostream &stream, const cubePos &pos) {
    stream << "face: " << pos.face << ", x: " << pos.x << ", y: " << pos.y;
    return stream;
  }
};
class RubiksCube {
private:
  std::vector<std::vector<std::vector<Pixel>>>
      data; // [6, 3, 3] which face -> row->col;

  std::vector<std::vector<int>> indices = {
      // SOUTH
      {
          1, 1, 0, 2, 2, 0 // todo
      },
      // NORTH
      {1, 1, 0, 2, 0, 2},
      // EAST
      {2, 0, 1, 1, 0, 2},
      // WEST
      {2, 0, 1, 1, 0, 2},
      // TOP
      {0, 2, 0, 2, 1, 1},
      // BOTTOM
      {2, 0, 0, 2, 1, 1}};
  std::vector<Block> blocks;

public:
  std::vector<Block> getBlocks();
  void getColor(int face1, int face2, int face3, Pixel &col1, Pixel &col2,
                Pixel &col3);
  void fillData();
  int getFaceIndex(int face, int face2);
  void makeBlocks();
  void mapCoords(int faceFrom, int faceTo, int x, int y, int &newX, int &newY);
  void mapRotations(int face, int x, int y, int &newX, int &newY);

  void turnFront();
  void turnBack();

public:
  std::vector<triangle> getAllTriangles();
  RubiksCube();
  void turn(int turnDir = LEFT);
  void update();

  bool isSolved();
  void randomScramble();

  Pixel getColor(const cubePos &pos);
  void setColor(const cubePos &pos, const Pixel col);
  Block getBlock(cubePos pos);

  void printBlocks();
  std::vector<std::vector<Pixel>> getFaceData(int face);
};

#endif