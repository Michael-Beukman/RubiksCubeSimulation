#include "rubiksCubeAi.h"
RubiksCubeAi::RubiksCubeAi(RubiksCube *c) : cube(c) {
  solvedCube = new RubiksCube();
  currentState = SOLVED;
}

RubiksCubeAi::RubiksCubeAi() : cube(nullptr) {
  currentState = SOLVED;
  solvedCube = new RubiksCube();
}

void RubiksCubeAi::makeCross() {

  getWhiteCrossPiecesOnTop();
  std::vector<cubePos> positions = {
      {FACE_TOP, 1, 0},
      {FACE_TOP, 0, 1},
      {FACE_TOP, 1, 2},
      {FACE_TOP, 2, 1},
  };
  // counterclockwise from left side
  for (int k = 0; k < 3;
       ++k) { // because sometimes we miss something ¯\_(ツ)_/¯
    auto blocks = cube->getBlocks();
    for (int i = 0; i < 4; ++i) {
      auto p = positions[i];
      int j = getBlockIndexFromPos(p);
      auto b = blocks[j];

      auto next = blocks[getBlockIndexFromPos(positions[(i + 1) % 4])];
      if (isOpposite(b.faces[0].t1.color, next.faces[0].t1.color)) {
        if (i == 2 || i == 3) {
          moves.push(UP);
        }
        addMoves({FRONT, UP, FRONT_REVERSE, UP_REVERSE, FRONT});
        break;
      }
    }
    playAllMoves();
    blocks = cube->getBlocks();
    std::vector<Pixel> cols;
    int index;
    for (int i = 0; i < 4; ++i) {
      cols.push_back(
          blocks[getBlockIndexFromPos(positions[i])].faces[0].t1.color);
      if (cols[i] == RED) {
        index = i;
      }
    }
    if (!(cols[(index + 1) % 4] == GREEN)) {
      addMoves({LEFT, LEFT, RIGHT, RIGHT, UP, UP, LEFT, LEFT, RIGHT, RIGHT});
    }
    playAllMoves();
    blocks = cube->getBlocks();
    while (!(blocks[getBlockIndexFromPos(positions[0])].faces[0].t1.color ==
             ORANGE)) {
      cube->turn(UP);
      blocks = cube->getBlocks();
    }
  }
  // then order
  // then double left, right, front and back to get cross;
  addMoves({LEFT, LEFT, RIGHT, RIGHT, FRONT, FRONT, BACK, BACK});
  playAllMoves();
  hasSolvedCross = true;
}

void RubiksCubeAi::solve() {
  if (cube != nullptr) {
    for (int i = SCRAMBLED; i <= PLL; ++i) {
      nextStep();
    }
  }
}

void RubiksCubeAi::nextStep() {
  switch (currentState) {
  case SCRAMBLED:
    makeCross();
    currentState = WHITE_CROSS;
    break;
  case WHITE_CROSS:
    makeWhiteCorners();
    currentState = FIRST_LAYER;
    break;
  case FIRST_LAYER:
    makeMiddleLayer();
    currentState = SECOND_LAYER;
    break;
  case SECOND_LAYER:
    makeYellowCross();
    currentState = YELLOW_CROSS;
    break;
  case YELLOW_CROSS:
    makeOLL();
    currentState = OLL;
    break;
  case OLL:
    makePLL();
    currentState = SOLVED;
    break;
  case PLL:
    break;
  case SOLVED:
    break;
  }
}

void RubiksCubeAi::getWhiteCrossPiecesOnTop() {
  int sum = 1;
  while (sum) {
    sum = 0;
    std::vector<cubePos> v = getWhiteCrossPieces();
    for (auto p : v) {
      int turnDir;
      int num = this->turnDirToTopEdges(p, turnDir);
      sum += num;
      if (num) {
        for (auto &p2 : v) {
          if (p2.face == FACE_TOP) {
            // if there is a square on the position we want to move to, turn top
            // face
            if ((turnDir == LEFT && p2.x == 1 && p2.y == 0) ||
                (turnDir == RIGHT && p2.x == 1 && p2.y == 2) ||
                (turnDir == FRONT && p2.x == 0) ||
                (turnDir == BACK && p2.x == 2)) {
              moves.push(UP);
            }
          }
        }
      }

      for (int k = 0; k < num; ++k)
        moves.push(turnDir);
      if (num)
        break;
    }
    playAllMoves();
  }
}

std::vector<cubePos> RubiksCubeAi::getWhiteCrossPieces() {
  std::vector<cubePos> v;
  for (int face = FACE_SOUTH; face <= FACE_BOTTOM; ++face) {
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        if (i == j || i == 2 - j)
          continue;
        cubePos p = {face, i, j};
        if (cube->getColor(p).color == WHITE.color) {
          v.push_back(p);
        }
      }
    }
  }
  return v;
}

int RubiksCubeAi::turnDirToTopEdges(cubePos p, int &turnDir) {
  // only for edges
  turnDir = UP;

  if (p.face == FACE_NORTH || p.face == FACE_SOUTH) {
    if (p.x == 1) {
      if (p.y == 0) {
        turnDir = LEFT;
        return p.face == FACE_NORTH ? 1 : 3;
      } else {
        turnDir = RIGHT;
        return p.face == FACE_NORTH ? 3 : 1;
      }
    } else {
      turnDir = (p.face == FACE_NORTH ? BACK : FRONT);
      return 1;
    }
  } else if (p.face == FACE_EAST || p.face == FACE_WEST) {
    if (p.x == 1) {
      if (p.y == 0) {
        turnDir = BACK;
        return 3;
      } else {
        turnDir = FRONT;
        return 1;
      }
    } else {
      turnDir = (p.face == FACE_EAST ? LEFT : RIGHT);
      return 1;
    }
  } else if (p.face == FACE_BOTTOM) {
    if (p.x == 0) {
      turnDir = BACK;
    } else if (p.x == 1) {
      turnDir = (p.y == 0 ? LEFT : RIGHT);
    } else {
      turnDir = FRONT;
    }
    return 2;
  } else {
    // top face
    return 0;
  }
  return 0;
}

int RubiksCubeAi::getBlockIndexFromPos(cubePos p) {

  switch (p.face) {
  case FACE_NORTH:
    // first 9
    return 8 - (3 * p.x + p.y);
    break;
  case FACE_SOUTH:
    return 25 - (3 * p.x + p.y);
    break;
  case FACE_TOP:
    if (p.x == 0) {
      // south
      if (p.y == 2)
        return 17; // west
      if (p.y == 1)
        return 20;
      else
        return 23;
    } else if (p.x == 1) {
      // face middle
      if (p.y == 2)
        return 9; // west
      if (p.y == 1)
        return 12;
      else
        return 14;
    } else {
      if (p.y == 2)
        return 0; // west
      if (p.y == 1)
        return 3;
      else
        return 6;
    }
    break;
  case FACE_BOTTOM:
    break;
  case FACE_EAST:
    break;
  case FACE_WEST:
    break;
  }
  std::cout << "error" << std::endl;
  return 0;
}

bool RubiksCubeAi::isOpposite(Pixel col1, Pixel col2) {
  return (col1 == RED && col2 == ORANGE) || (col1 == ORANGE && col2 == RED) ||

         (col1 == GREEN && col2 == BLUE) || (col1 == BLUE && col2 == GREEN) ||

         (col1 == WHITE && col2 == YELLOW) || (col1 == YELLOW && col2 == WHITE);
}

void RubiksCubeAi::playAllMoves() {
  while (!moves.empty())
    this->makeNextMove();
}

void RubiksCubeAi::makeNextMove() {
  if (!moves.empty()) {
    int m = moves.front();
    cube->turn(m);
    moves.pop();
  }
}

std::vector<cubePos> RubiksCubeAi::getBlockCubePos(int index) {
  return cubePositions[index];
}

std::vector<int> RubiksCubeAi::turnDirToTopCorners(cubePos p, int blockType) {
  if (p.face == FACE_TOP)
    return {};

  switch (blockType) {
  case NWT:
    if (p.face == FACE_WEST) {
      return {RIGHT_REVERSE, FRONT_REVERSE, UP, FRONT, RIGHT};
    } else {
      return {RIGHT_REVERSE, UP, RIGHT};
    }
    break;
  case NET:
    if (p.face == FACE_EAST) {
      return {LEFT, FRONT, UP, FRONT_REVERSE, LEFT_REVERSE};
    } else {
      // back
      return {LEFT, UP_REVERSE, LEFT_REVERSE};
    }
    break;
  case SWT:
    if (p.face == FACE_WEST) {
      // front
      return {FRONT_REVERSE, UP, FRONT};
    } else {
      // right
      return {RIGHT, UP_REVERSE, RIGHT_REVERSE};
    }
    break;
  case SET:
    if (p.face == FACE_EAST) {
      // front
      return {FRONT, UP_REVERSE, FRONT_REVERSE};
    } else {
      // right
      return {LEFT_REVERSE, UP, LEFT};
    }
    break;
  case NWB:
    if (p.face == FACE_WEST) {
      return {BACK, UP, BACK_REVERSE};
    } else {
      return {RIGHT_REVERSE, UP_REVERSE, RIGHT};
    }
    break;
  case NEB:
    if (p.face == FACE_EAST) {
      return {BACK_REVERSE, UP_REVERSE, BACK};
    } else {
      return {LEFT, UP, LEFT_REVERSE};
    }
    break;
  case SWB:
    if (p.face == FACE_WEST) {
      return {FRONT_REVERSE, UP_REVERSE, FRONT};
    } else {
      return {RIGHT, UP, RIGHT_REVERSE};
    }
    break;
  case SEB:
    if (p.face == FACE_EAST) {
      return {FRONT, UP, FRONT_REVERSE};
    } else {
      return {LEFT_REVERSE, UP_REVERSE, LEFT};
    }
    break;
  default:
    break;
  }
  return {};
}

std::vector<int> RubiksCubeAi::topToBottom(int blockType) {
  switch (blockType) {
  case NWT:
    return {RIGHT_REVERSE, UP, UP, RIGHT, UP, RIGHT_REVERSE, UP_REVERSE, RIGHT};
    break;
  case SWT:
    return {RIGHT, UP, UP, RIGHT_REVERSE, UP_REVERSE, RIGHT, UP, RIGHT_REVERSE};
    break;
  case NET:
    return {LEFT, UP, UP, LEFT_REVERSE, UP_REVERSE, LEFT, UP, LEFT_REVERSE};
    break;
  case SET:
    return {LEFT_REVERSE, UP, UP, LEFT, UP, LEFT_REVERSE, UP_REVERSE, LEFT};
    break;
  }
  return {};
}

void RubiksCubeAi::makeWhiteCorners() {
  int c = 0;
  while (getWhiteCornersBottom() && c < MAX_ITERS) {
    playAllMoves();
    ++c;
  }
  playAllMoves();
}

int RubiksCubeAi::getBlockIndex(std::vector<Pixel> colors) {
  auto isSame = [&](std::vector<Pixel> &cols, Block &b) {
    if (cols.size() == b.faces.size()) {
      int sameCount = 0;
      for (auto &col : cols) {
        for (auto face : b.faces) {
          if (col == face.getColor()) {
            ++sameCount;
            break;
          }
        }
      }
      return sameCount == cols.size();
    }
    return false;
  };
  auto blocks = cube->getBlocks();
  for (int i = 0; i < blocks.size(); ++i) {
    if (isSame(colors, blocks[i]))
      return i;
  }
  return -1;
}

int RubiksCubeAi::getTargetBlockIndex(std::vector<Pixel> colors) {
  auto isSame = [&](std::vector<Pixel> &cols, std::vector<Pixel> cols2) {
    if (cols.size() == cols2.size()) {
      int sameCount = 0;
      for (auto &col : cols) {
        for (auto col2 : cols2) {
          if (col == col2) {
            ++sameCount;
            break;
          }
        }
      }
      return sameCount == cols.size();
    }
    return false;
  };
  if (isSame(colors, {RED, WHITE, BLUE})) {
    return SWT; // B;
  }
  if (isSame(colors, {RED, WHITE, GREEN})) {
    return NWT; // B;
  }
  if (isSame(colors, {ORANGE, WHITE, BLUE})) {
    return SET; // B;
  }
  if (isSame(colors, {ORANGE, WHITE, GREEN})) {
    return NET; // B;
  }
  return -1;
}

int RubiksCubeAi::changeBlockIndex(int blockIndex) {
  if (blockIndex == SWB)
    return SWT;
  if (blockIndex == NWB)
    return NWT;
  if (blockIndex == SEB)
    return SET;
  if (blockIndex == NEB)
    return NET;
  if (blockIndex == SWT)
    return SWB;
  if (blockIndex == SET)
    return SEB;
  if (blockIndex == NWT)
    return NWB;
  if (blockIndex == NET)
    return NEB;
  return -1;
}

bool RubiksCubeAi::getWhiteCornersBottom() {
  std::vector<std::vector<Pixel>> blockColors = {
      {RED, WHITE, BLUE},
      {RED, WHITE, GREEN},
      {ORANGE, WHITE, BLUE},
      {ORANGE, WHITE, GREEN},
  };
  for (int i = 0; i < blockColors.size(); ++i) {
    int index = getBlockIndex(blockColors[i]);
    int target = getTargetBlockIndex(blockColors[i]);
    std::vector<cubePos> positions = getBlockCubePos(index);
    cubePos whiteSquarePos;
    bool isCorrect = false;
    bool isOnTop = false;
    for (auto &p : positions) {
      if (index == target) {
        if (p.face == FACE_TOP && cube->getColor(p) == WHITE) {
          // correct
          isCorrect = true;
          break;
        }
        if (cube->getColor(p) == WHITE)
          whiteSquarePos = p;
      } else if (index == changeBlockIndex(target)) {
        if (p.face == FACE_BOTTOM && cube->getColor(p) == WHITE) {
          // correct
          isCorrect = true;
          break;
        }
      } else if (p.face == FACE_TOP && cube->getColor(p) == WHITE) {
        isOnTop = true;
        break;
      }
    }
    if (!isCorrect) {
      if (index == NWT || index == SWT || index == NET || index == SET) {
        int indexToCheck = changeBlockIndex(index);
        int correctIndexToBeAt = changeBlockIndex(target);
        if (indexToCheck != correctIndexToBeAt) {
          moves.push(UP);
          return true;
        }
      }
      // need to change from index to target index, with pos correct;
      auto movesToMake = turnDirToTopCorners(whiteSquarePos, index);
      addMoves(movesToMake);
      // return true;
    } else if (isOnTop) {
      moves.push(UP);
      return true;
    } else {
      // now the block is above its correct pos
      auto m = topToBottom(index);
      addMoves(m);
      // return true;
    }
    if (!isCorrect) {
      return true;
    }
  }

  std::vector<int> corners = {SWB, SEB, NWB, NEB};
  for (auto &c : corners) {
    auto p = getBlockCubePos(c);
    if (!isInCorrectPlace(p)) {
      return true;
    }
  }
  return false;
  // return false;
}

void RubiksCubeAi::makeMiddleLayer() {
  int c = 0;
  while (getEdgePiecesMiddle() && c < MAX_ITERS) {
    playAllMoves();
    ++c;
  }
  playAllMoves();
}

std::vector<int> RubiksCubeAi::getFrontRightSpinner() {
  return {UP,    RIGHT,         UP_REVERSE,    RIGHT_REVERSE,
          FRONT, RIGHT_REVERSE, FRONT_REVERSE, RIGHT};
}

std::vector<int> RubiksCubeAi::getFrontLeftSpinner() {
  return {UP_REVERSE,    LEFT_REVERSE, UP,    LEFT,
          FRONT_REVERSE, LEFT,         FRONT, LEFT_REVERSE};
}

std::vector<int> RubiksCubeAi::getBackRightSpinner() {
  return {UP,    BACK,         UP_REVERSE,    BACK_REVERSE,
          RIGHT, BACK_REVERSE, RIGHT_REVERSE, BACK};
}

std::vector<int> RubiksCubeAi::getBackLeftSpinner() {
  return {UP_REVERSE,   BACK_REVERSE, UP,   BACK,
          LEFT_REVERSE, BACK,         LEFT, BACK_REVERSE};
}

std::vector<int> RubiksCubeAi::getFrontRightSpinnerB() {
  return {UP_REVERSE,    FRONT_REVERSE, UP,    FRONT,
          RIGHT_REVERSE, FRONT,         RIGHT, FRONT_REVERSE};
}

std::vector<int> RubiksCubeAi::getFrontLeftSpinnerB() {
  return {UP,   FRONT,         UP_REVERSE,   FRONT_REVERSE,
          LEFT, FRONT_REVERSE, LEFT_REVERSE, FRONT};
}

std::vector<int> RubiksCubeAi::getBackRightSpinnerB() {
  return {UP_REVERSE,   RIGHT_REVERSE, UP,   RIGHT,
          BACK_REVERSE, RIGHT,         BACK, RIGHT_REVERSE};
}

std::vector<int> RubiksCubeAi::getBackLeftSpinnerB() {
  return {UP,   LEFT,         UP_REVERSE,   LEFT_REVERSE,
          BACK, LEFT_REVERSE, BACK_REVERSE, LEFT};
}

bool RubiksCubeAi::getEdgePiecesMiddle() {
  std::vector<std::vector<Pixel>> blockColors = {
      {RED, BLUE},
      {RED, GREEN},
      {ORANGE, BLUE},
      {ORANGE, GREEN},
  };
  auto getFace = [&](Pixel col) {
    if (col == RED)
      return FACE_WEST;
    if (col == ORANGE)
      return FACE_EAST;
    if (col == BLUE)
      return FACE_SOUTH;
    return FACE_NORTH;
  };
  for (int i = 0; i < blockColors.size(); ++i) {
    int index = getBlockIndex(blockColors[i]);
    auto positions = getBlockCubePos(index);
    if (isInCorrectPlace(positions)) {
      continue;
    }
    if (positions[0].face == FACE_TOP || positions[1].face == FACE_TOP) {
      cubePos posOnSide;
      Pixel colOnSide;
      Pixel colOnTop;
      if (positions[0].face == FACE_TOP) {
        posOnSide = positions[1];
        colOnSide = cube->getColor(positions[1]);
        colOnTop = cube->getColor(positions[0]);
      } else {
        posOnSide = positions[0];
        colOnSide = cube->getColor(positions[0]);
        colOnTop = cube->getColor(positions[1]);
      }
      int targetFace = getFace(colOnSide);
      if (targetFace != posOnSide.face) {
        moves.push(UP);
        return true;
      } else {
        // now it's on the correct spot, now only do spinner
        std::vector<int> movesToDo;
        if (colOnSide == BLUE) { // south face then
          if (colOnTop == RED) {
            movesToDo = getFrontRightSpinner();
          } else {
            movesToDo = getFrontLeftSpinner();
          }
        } else if (colOnSide == RED) {
          if (colOnTop == GREEN) {
            movesToDo = getBackLeftSpinner();
          } else {
            movesToDo = getFrontRightSpinnerB();
          }
        } else if (colOnSide == GREEN) {
          if (colOnTop == RED) {
            movesToDo = getBackRightSpinnerB();
          } else {
            movesToDo = getBackLeftSpinnerB();
          }
        } else { // orange
          if (colOnTop == GREEN) {
            movesToDo = getBackLeftSpinner();
          } else {
            movesToDo = getFrontLeftSpinnerB();
          }
        }

        addMoves(movesToDo);
        return movesToDo.size() > 0;
      }
    } else {
      // then in one of the side pockets
      std::vector<int> movesToDo;
      if (index == 1) { // nwm
        movesToDo = getBackRightSpinner();
      } else if (index == 7) { // nem
        movesToDo = getBackLeftSpinner();
      } else if (index == 18) { // swm
        movesToDo = getFrontRightSpinner();
      } else if (index == 24) { // sem
        movesToDo = getFrontLeftSpinner();
      }
      addMoves(movesToDo);
      return movesToDo.size() > 0;
    }
    break;
  }
  return false;
}

RubiksCubeAi::~RubiksCubeAi() { delete solvedCube; }

void RubiksCubeAi::addMoves(const std::vector<int> m) {
  for (auto &move : m) {
    moves.push(move);
  }
}

void RubiksCubeAi::makeYellowCross() {
  int c = 0;
  while (doYellowCross() && c <= MAX_ITERS) {
    if (c % 3 == 0)
      moves.push(UP);
    playAllMoves();
    ++c;
  }
  playAllMoves();
}

bool RubiksCubeAi::doYellowCross() {
  // top face
  auto topFace = cube->getFaceData(FACE_TOP);
  // classify which pattern
  int numYellows = 0;
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      if (i == 1 || j == 1)
        numYellows += topFace[i][j] == YELLOW;
    }
  }
  // do things
  if (numYellows != 5 && numYellows != 9) {
    addMoves({FRONT, RIGHT, UP_REVERSE, RIGHT_REVERSE, FRONT_REVERSE,
              LEFT_REVERSE, UP, LEFT});
  }
  if (numYellows == 5)
    return false;
  return true;
}

void RubiksCubeAi::makeOLL() {
  int c = 0;
  int i = 8;
  while (doOLL() && c < MAX_ITERS) {
    if (c % i == 0) {
      moves.push(UP);
    }
    playAllMoves();
    c++;
  }
}

bool RubiksCubeAi::doOLL() {
  // top face
  auto topFace = cube->getFaceData(FACE_TOP);
  // classify which pattern
  int numYellows = 0;
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      numYellows += topFace[i][j] == YELLOW;
    }
  }
  if (numYellows == 6 && !(topFace[2][2] == YELLOW)) {
    if (topFace[2][0] == YELLOW) {
      moves.push(UP);
      return true;
    } else if (topFace[0][0] == YELLOW) {
      addMoves({UP, UP});
      return true;
    } else if (topFace[0][2] == YELLOW) {
      moves.push(UP_REVERSE);
      return true;
    }
  }
  // do things
  // std::cout << numYellows << std::endl;
  if (numYellows != 9) {
    addMoves({LEFT, UP, LEFT_REVERSE, UP, LEFT, UP, UP, LEFT_REVERSE});
    return true;
  } else {
    return false;
  }
}

void RubiksCubeAi::makePLL() {
  int c = 0;
  while (doPLLCorners() && c < MAX_ITERS) {
    playAllMoves();
    ++c;
  }
  c = 0;
  while (doPLLEdges() && c < MAX_ITERS) {
    playAllMoves();
    ++c;
  }
  playAllMoves();
}

bool RubiksCubeAi::doPLLCorners() {
  // first do corners
  // swt
  // set
  // nwt
  // net

  auto pSWT = getBlockCubePos(SWT);
  auto pSET = getBlockCubePos(SET);
  auto pNWT = getBlockCubePos(NWT);
  auto pNET = getBlockCubePos(NET);
  auto getPosOnFace = [&](int face, std::vector<cubePos> v) {
    for (auto &p : v) {
      if (p.face == face)
        return p;
    }
    return v[0];
  };
  // count correct corners
  int corr = 0;
  int correctCornerFace = -1;
  std::vector<std::vector<std::vector<cubePos>>> positions = {
      {pSWT, pSET}, {pSET, pNET}, {pNWT, pSWT}, {pNWT, pNET}};
  std::vector<int> faces = {FACE_SOUTH, FACE_EAST, FACE_WEST, FACE_NORTH};
  for (int i = 0; i < faces.size(); ++i) {
    int faceTemp = faces[i];
    cubePos Left = getPosOnFace(faceTemp, positions[i][0]);
    cubePos Right = getPosOnFace(faceTemp, positions[i][1]);
    if (cube->getColor(Left) == cube->getColor(Right)) {
      ++corr;
      correctCornerFace = faceTemp;
    }
  }
  if (corr == 0) {

  } else if (corr == 1) {
    if (correctCornerFace == FACE_EAST) {
      moves.push(UP);
    } else if (correctCornerFace == FACE_WEST) {
      moves.push(UP_REVERSE);
    } else if (correctCornerFace == FACE_SOUTH) {
      addMoves({UP, UP});
    }
  } else {
    // done
    return false;
  }

  addMoves({RIGHT_REVERSE, FRONT, RIGHT_REVERSE, BACK, BACK, RIGHT,
            FRONT_REVERSE, RIGHT_REVERSE, BACK, BACK, RIGHT, RIGHT});
  return true;
}

bool RubiksCubeAi::doPLLEdges() {
  int index = getBlockIndex({RED, YELLOW, BLUE});
  auto positions = getBlockCubePos(index);
  cubePos redPos;
  for (auto &p : positions) {
    if (cube->getColor(p) == RED) {
      redPos = p;
      break;
    }
  }
  switch (redPos.face) {
  case FACE_EAST:
    addMoves({UP, UP});
    return true;
    break;
  case FACE_NORTH:
    addMoves({UP});
    return true;
    break;
  case FACE_SOUTH:
    addMoves({UP_REVERSE});
    return true;
    break;
  }
  // edge pieces
  // 5 scenarios
  // all fixed :-) done,
  auto currPosEastEdge = getBlockCubePos(E_EDGE);
  auto currPosWestEdge = getBlockCubePos(W_EDGE);
  auto currPosSouthEdge = getBlockCubePos(S_EDGE);
  auto currPosNorthEdge = getBlockCubePos(N_EDGE);
  std::vector<std::vector<cubePos>> v = {currPosEastEdge, currPosWestEdge,
                                         currPosSouthEdge, currPosNorthEdge};
  for (std::vector<cubePos> &arr : v) {
    arr.erase(std::remove_if(
                  arr.begin(), arr.end(),
                  [&](cubePos &col) { return cube->getColor(col) == YELLOW; }),
              arr.end());
  }
  cubePos eastEdge = v[0][0];
  cubePos westEdge = v[1][0];
  cubePos southEdge = v[2][0];
  cubePos northEdge = v[3][0];

  // count wrong
  int countWrong = 0;
  cubePos correctFace;
  if (!(cube->getColor(eastEdge) == ORANGE))
    ++countWrong;
  else
    correctFace = eastEdge;
  if (!(cube->getColor(westEdge) == RED))
    ++countWrong;
  else
    correctFace = westEdge;
  if (!(cube->getColor(southEdge) == BLUE))
    ++countWrong;
  else
    correctFace = southEdge;
  if (!(cube->getColor(northEdge) == GREEN))
    ++countWrong;
  else
    correctFace = northEdge;

  if (countWrong == 0) {
    // yay
    return false;
  }
  if (countWrong == 3) {
    std::vector<int> preMoves = {};
    std::vector<int> postMoves = {};
    if (correctFace.face == FACE_NORTH) {
      preMoves = {UP, UP};
      postMoves = {UP, UP};
    } else if (correctFace.face == FACE_EAST) {
      preMoves = {UP_REVERSE};
      postMoves = {UP};
    } else if (correctFace.face == FACE_WEST) {
      preMoves = {UP};
      postMoves = {UP_REVERSE};
    }
    addMoves(preMoves);
    // uperm
    addMoves({RIGHT, RIGHT, UP_REVERSE, RIGHT_REVERSE, UP_REVERSE, RIGHT, UP,
              RIGHT, UP, RIGHT, UP_REVERSE, RIGHT});
    addMoves(postMoves);
    return true;
  } else {
    // 4
    if (cube->getColor(eastEdge) == RED) {
      // double one
      addMoves({LEFT,  LEFT,  RIGHT, RIGHT, UP,    UP,        LEFT,  LEFT,
                RIGHT, RIGHT, UP,    LEFT,  LEFT,  RIGHT,     RIGHT, UP,
                UP,    LEFT,  LEFT,  RIGHT, RIGHT, UP_REVERSE});
      return true;
    } else {
      // other one
      // todo
      // z perm
      addMoves({RIGHT, UP, RIGHT, RIGHT, UP_REVERSE, RIGHT_REVERSE, UP_REVERSE,
                RIGHT, UP, RIGHT_REVERSE, UP_REVERSE, RIGHT_REVERSE, UP,
                RIGHT_REVERSE, UP, RIGHT, UP, UP});
      return true;
    }
  }
  // opposites
  // adjacents
  // three way split
  return false;
}

bool RubiksCubeAi::isInCorrectPlace(std::vector<cubePos> positions) {
  for (auto &p : positions) {
    if (!(cube->getColor(p) == solvedCube->getColor(p)))
      return false;
  }
  return true;
}

void RubiksCubeAi::reset() { currentState = SCRAMBLED; }

std::string RubiksCubeAi::getStatus() {
  switch (currentState) {
  case SCRAMBLED:
    return "SCRAMBLED";
  case WHITE_CROSS:
    return "WHITE CROSS";
  case FIRST_LAYER:
    return "FIRST LAYER";
  case SECOND_LAYER:
    return "SECOND LAYER";
  case YELLOW_CROSS:
    return "YELLOW CROSS";
  case OLL:
    return "OLL";
  case PLL:
    return "PLL";
  case SOLVED:
    return "SOLVED";
  }
}