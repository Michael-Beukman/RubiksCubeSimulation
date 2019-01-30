#include "rubiksCube.h"
#include <random>
bool hasTurned = false;
RubiksCube::RubiksCube()
    : data(6,
           std::vector<std::vector<Pixel>>(3, std::vector<Pixel>(3, WHITE))) {
  std::vector<triangle> all;
  fillData();
  makeBlocks();
}

void RubiksCube::makeBlocks() {
  blocks.clear();
  for (int face1 : {FACE_NORTH, FACE_MIDDLE, FACE_SOUTH}) {
    for (int face2 : {FACE_WEST, FACE_MIDDLE, FACE_EAST}) {
      for (int face3 : {FACE_TOP, FACE_MIDDLE, FACE_BOTTOM}) {
        if (face1 == face3 && face1 == FACE_MIDDLE && face2 == FACE_MIDDLE)
          continue;
        int oX = 1;
        int oY = 1;
        int oZ = 1;
        if (face1 == FACE_NORTH)
          oZ = 2;
        else if (face1 == FACE_SOUTH)
          oZ = 0;

        if (face2 == FACE_WEST)
          oX = 0;
        else if (face2 == FACE_EAST)
          oX = 2;

        if (face3 == FACE_BOTTOM)
          oY = 0;
        else if (face3 == FACE_TOP)
          oY = 2;
        Pixel col1 = BLACK, col2 = BLACK, col3 = BLACK;
        getColor(face1, face2, face3, col1, col2, col3);
        Block b = makeBlock(face1, face2, face3, oX, oY, oZ, col1, col2, col3);
        this->blocks.push_back(b);
      }
    }
  }
}

std::vector<triangle> RubiksCube::getAllTriangles() {
  std::vector<triangle> returnVal;
  for (auto b : blocks) {
    auto ts = b.getAllTriangles();
    returnVal.insert(returnVal.end(), ts.begin(), ts.end());
  }
  return returnVal;
}

void RubiksCube::getColor(int face1, int face2, int face3, Pixel &col1,
                          Pixel &col2, Pixel &col3) {
  if (face1 >= 0) {
    int i1 = getFaceIndex(face1, face2);
    int i2 = getFaceIndex(face1, face3);
    if (face1 == FACE_SOUTH)
      col1 = data[face1][i2][i1]; // hack
    else
      col1 = data[face1][i2][i1];
  } else
    col1 = BLACK;

  if (face2 >= 0) {
    int i1 = getFaceIndex(face2, face1);
    int i2 = getFaceIndex(face2, face3);
    col2 = data[face2][i2][i1]; 
  } else
    col2 = BLACK;

  if (face3 >= 0) {
    int i1 = getFaceIndex(face3, face1);
    int i2 = getFaceIndex(face3, face2);

    if (face3 == FACE_TOP)
      col3 = data[face3][i1][i2];
    else
      col3 = data[face3][i1][i2];
  } else
    col3 = BLACK;
}

void RubiksCube::fillData() {
  std::vector<int> faces = {FACE_BOTTOM, FACE_TOP,  FACE_SOUTH,
                            FACE_NORTH,  FACE_EAST, FACE_WEST};
  std::vector<Pixel> cols = {WHITE, YELLOW, BLUE, GREEN, ORANGE, RED};
  for (int i = 0; i < faces.size(); ++i) {
    for (int j = 0; j < 3; ++j) {
      for (int k = 0; k < 3; ++k) {
        data[faces[i]][j][k] = cols[i];
      }
    }
  }
}

int RubiksCube::getFaceIndex(int face, int face2) {

  if (face2 < 0)
    return 1;
  return indices[face][face2];
  if (face2 == FACE_TOP || face2 == FACE_EAST || face2 == FACE_NORTH)
    return 0;
  else if (face2 == FACE_BOTTOM || face2 == FACE_WEST || face2 == FACE_SOUTH)
    return 2;
  return 1;
}

void RubiksCube::turn(int turnDir) {
  if (turnDir > BACK) { // REVERSE
    for (int i = 0; i < 3; ++i)
      turn(turnDir - 6);
  }
  std::vector<int> facesToChange;
  std::vector<int> facesToRotate;
  std::vector<int> coord1, coord2;
  switch (turnDir) {
  case LEFT:
    facesToChange = {FACE_TOP, FACE_NORTH, FACE_BOTTOM, FACE_SOUTH};
    facesToRotate = {FACE_EAST};
    coord1 = {0, 1, 2};
    coord2 = {0, 0, 0};
    break;

  case RIGHT:
    facesToChange = {FACE_TOP, FACE_SOUTH, FACE_BOTTOM, FACE_NORTH};
    facesToRotate = {FACE_WEST};
    coord1 = {0, 1, 2};
    coord2 = {2, 2, 2};
    break;

  case UP:
    facesToChange = {FACE_SOUTH, FACE_WEST, FACE_NORTH, FACE_EAST};
    facesToRotate = {FACE_TOP};
    coord1 = {0, 0, 0};
    coord2 = {0, 1, 2};
    break;

  case DOWN:
    facesToChange = {FACE_SOUTH, FACE_EAST, FACE_NORTH, FACE_WEST};
    facesToRotate = {FACE_BOTTOM};
    coord1 = {2, 2, 2};
    coord2 = {0, 1, 2};
    break;
  case FRONT:
    facesToChange = {FACE_TOP, FACE_WEST, FACE_BOTTOM, FACE_EAST};
    facesToRotate = {FACE_SOUTH};
    coord2 = {2, 2, 2};
    coord1 = {0, 1, 2};
    break;
  case BACK:
    facesToChange = {FACE_TOP, FACE_EAST, FACE_BOTTOM, FACE_WEST};
    facesToRotate = {FACE_NORTH};
    coord1 = {2, 2, 2};
    coord2 = {0, 1, 2};
    break;
  default:
    return;
  }

  auto switchCols = [&](int face1, int face2) {
    for (int k = 0; k < 3; ++k) {
      int newX, newY;
      mapCoords(face2, face1, coord1[k], coord2[k], newX, newY);
      data[face1][newX][newY] = data[face2][coord1[k]][coord2[k]];
    }
  };
  std::vector<Pixel> temp;
  if (turnDir == FRONT)
    turnFront();
  else if (turnDir == BACK)
    turnBack();
  else {
    for (int i = 0; i < 3; ++i) {
      int newX, newY;
      mapCoords(facesToChange[0], facesToChange[facesToChange.size() - 1],
                coord1[i], coord2[i], newX, newY);
      temp.push_back(data[facesToChange[0]][newX][newY]);
    }

    for (int i = 0; i < facesToChange.size() - 1; ++i) {
      switchCols(facesToChange[i], facesToChange[i + 1]);
    }
    for (int i = 0; i < 3; ++i) {
      int newX = coord1[i], newY = coord2[i];
      int c = newX;
      if (coord1[0] == coord1[1])
        c = newY;
      data[facesToChange[facesToChange.size() - 1]][coord1[i]][coord2[i]] =
          temp[c];
    }
  }
  // somehow do rotations
  auto &faceData = data[facesToRotate[0]];
  std::vector<std::vector<Pixel>> tempData(3, std::vector<Pixel>(3, WHITE));
  auto change = [&](int a, int b) {
    int t1 = b;
    int t2 = a;
    if (facesToRotate[0] == FACE_EAST) {
      t1 = a;
      t2 = b;
    }
    int x1 = t1 % 3;
    int y1 = t1 / 3;
    int x2 = t2 % 3;
    int y2 = t2 / 3;
    tempData[y1][x1] = faceData[y2][x2];
  };
  change(0, 6);
  change(1, 3);
  change(2, 0);
  change(3, 7);
  change(4, 4);
  change(5, 1);
  change(6, 8);
  change(7, 5);
  change(8, 2);
  faceData = tempData;
  makeBlocks();
}

void RubiksCube::update() {
  if (!hasTurned) {
    makeBlocks();
  }

  hasTurned = true;
}
void RubiksCube::mapCoords(int faceFrom, int faceTo, int x, int y, int &newX,
                           int &newY) {
  // hacky way to change the colors when turning
  newX = x;
  newY = y;
  if (faceFrom == FACE_WEST && faceTo == FACE_SOUTH) {

    newX = 2 - x;
    newY = 2 - y;
  }
  if (faceFrom == FACE_SOUTH && faceTo == FACE_EAST) {

    newX = 2 - x;
    // newY = 2-y;
  }
  if (faceFrom == FACE_EAST && faceTo == FACE_NORTH) {
    newY = 2 - y;
  }

  if (faceFrom == FACE_EAST && faceTo == FACE_SOUTH) {
    newX = 2 - x;
  }
  if (faceFrom == FACE_SOUTH && faceTo == FACE_WEST) {
    newX = 2 - x;
    newY = 2 - y;
  }
  if (faceFrom == FACE_NORTH && faceTo == FACE_EAST) {
    newY = 2 - y;
  }

  if (faceFrom == FACE_TOP && faceTo == FACE_EAST) {
  }
}

void RubiksCube::mapRotations(int face, int x, int y, int &newX, int &newY) {
  newX = x;
  newY = y;
  if (face == FACE_TOP || face == FACE_BOTTOM) {
    newX = 2 - x;
    newY = 2 - y;
  } else if (face == FACE_WEST) {
    newY = 2 - y;
    newX = 2 - x;
  }
}

void RubiksCube::turnFront() {
  std::vector<Pixel> temp = {data[FACE_BOTTOM][2][0], data[FACE_BOTTOM][2][1],
                             data[FACE_BOTTOM][2][2]};

  for (int i = 0; i < 3; ++i)
    data[FACE_BOTTOM][2][i] = data[FACE_WEST][2 - i][2];

  for (int i = 0; i < 3; ++i)
    data[FACE_WEST][i][2] = data[FACE_TOP][0][i];

  for (int i = 0; i < 3; ++i)
    data[FACE_TOP][0][i] = data[FACE_EAST][2 - i][2];

  for (int i = 0; i < 3; ++i) {
    data[FACE_EAST][i][2] = temp[i];
  }
}

void RubiksCube::turnBack() {
  std::vector<Pixel> temp = {data[FACE_BOTTOM][0][0], data[FACE_BOTTOM][0][1],
                             data[FACE_BOTTOM][0][2]};
  // for (int i = 0; i < 3; ++i)
  //     data[FACE_EAST][i][2] = data[FACE_BOTTOM][2][i];
  for (int i = 0; i < 3; ++i)
    data[FACE_BOTTOM][0][i] = data[FACE_EAST][i][0];

  for (int i = 0; i < 3; ++i)
    data[FACE_EAST][2 - i][0] = data[FACE_TOP][2][i];

  for (int i = 0; i < 3; ++i)
    data[FACE_TOP][2][i] = data[FACE_WEST][i][0];

  for (int i = 0; i < 3; ++i) {
    data[FACE_WEST][2 - i][0] = temp[i];
  }
}

bool RubiksCube::isSolved() {
  for (int i = 0; i < 6; ++i) {
    Pixel col = data[i][0][0];
    for (int j = 0; j < 3; ++j) {
      for (int k = 0; k < 3; ++k) {
        if (data[i][j][k].color != col.color)
          return false;
      }
    }
  }
  return true;
}

void RubiksCube::randomScramble() {
  // std::default_random_engine generator;
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> distribution(1, 2);
  std::uniform_int_distribution<int> turnDir(0, 11);
  std::uniform_int_distribution<int> numTurns(1, 2);
  for (int i = 0; i < 35; ++i) {
    int dice_roll = distribution(mt);
    if ((double)dice_roll / 2.0 <= 0.5) {
      int r = turnDir(mt);
      int n = numTurns(mt);
      for (int i = 0; i < n; ++i)
        this->turn(r);
    }
  }
}

Pixel RubiksCube::getColor(const cubePos &pos) {
  return data[pos.face][pos.x][pos.y];
}

Block RubiksCube::getBlock(cubePos pos) {
  switch (pos.face) {
  case FACE_NORTH:
    // first 9
    break;
  }

  return blocks[0];
}

void RubiksCube::printBlocks() {
  int i = 0;
  for (auto b : blocks) {
    std::cout << "Block " << i << std::endl;
    for (auto f : b.faces) {
      std::cout << "Face " << Window::pixelToString(f.t1.color) << std::endl;
    }
    ++i;
  }
}

std::vector<Block> RubiksCube::getBlocks() { return blocks; }

void RubiksCube::setColor(const cubePos &pos, const Pixel col) {
  data[pos.face][pos.x][pos.y] = col;
  this->makeBlocks();
}


std::vector<std::vector<Pixel>> RubiksCube::getFaceData(int face){
  return data[face];  
}