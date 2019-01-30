#ifndef RUBIKSCUBEAI_H
#define RUBIKSCUBEAI_H
#include "rubiksCube.h"
#include <map>
#include <queue>
typedef std::vector<std::pair<int, int>> vii;

class RubiksCubeAi {
private:
  const int MAX_ITERS = 50;
  enum blockPositions {
    NWT = 0,
    NET = 6,
    SWT = 17,
    SET = 23,
    NWB = 2,
    NEB = 8,
    SWB = 19,
    SEB = 25,

    E_EDGE = 14,
    W_EDGE = 9,
    N_EDGE = 3,
    S_EDGE = 20
  };
  RubiksCube *cube;
  RubiksCube *solvedCube;
  std::queue<int> moves;
  std::map<int, std::vector<cubePos>> cubePositions = {
      {NWT, {{FACE_NORTH, 0, 2}, {FACE_WEST, 0, 0}, {FACE_TOP, 2, 2}}},
      {NET, {{FACE_NORTH, 0, 0}, {FACE_EAST, 0, 0}, {FACE_TOP, 2, 0}}},
      {SWT, {{FACE_SOUTH, 2, 2}, {FACE_WEST, 0, 2}, {FACE_TOP, 0, 2}}},
      {SET, {{FACE_SOUTH, 2, 0}, {FACE_EAST, 0, 2}, {FACE_TOP, 0, 0}}},
      {NWB, {{FACE_NORTH, 2, 2}, {FACE_WEST, 2, 0}, {FACE_BOTTOM, 0, 2}}},
      {NEB, {{FACE_NORTH, 2, 0}, {FACE_EAST, 2, 0}, {FACE_BOTTOM, 0, 0}}},
      {SWB, {{FACE_SOUTH, 0, 2}, {FACE_WEST, 2, 2}, {FACE_BOTTOM, 2, 2}}},
      {SEB, {{FACE_SOUTH, 0, 0}, {FACE_EAST, 2, 2}, {FACE_BOTTOM, 2, 0}}},

      // back face
      {1, {{FACE_NORTH, 1, 2}, {FACE_WEST, 1, 0}}},
      {3, {{FACE_NORTH, 0, 1}, {FACE_TOP, 2, 1}}}, //back edge
      {4, {{FACE_NORTH, 1, 1}}},
      {5, {{FACE_NORTH, 2, 1}, {FACE_BOTTOM, 0, 1}}},
      {7, {{FACE_NORTH, 1, 0}, {FACE_EAST, 1, 0}}},

      // west
      {9, {{FACE_WEST, 0, 1}, {FACE_TOP, 1, 2}}}, // west edge
      {10, {{FACE_WEST, 1, 1}}},
      {11, {{FACE_WEST, 2, 1}, {FACE_BOTTOM, 1, 2}}},
      {12, {{FACE_TOP, 1, 1}}},
      {13, {{FACE_BOTTOM, 1, 1}}},

      // east
      {14, {{FACE_EAST, 0, 1}, {FACE_TOP, 1, 0}}}, // east edge
      {15, {{FACE_EAST, 1, 1}}},
      {16, {{FACE_EAST, 2, 1}, {FACE_BOTTOM, 1, 0}}},
      {18, {{FACE_SOUTH, 1, 2}, {FACE_WEST, 1, 2}}},
      {20, {{FACE_SOUTH, 2, 1}, {FACE_TOP, 0, 1}}}, // south face

      // SOUTH
      {21, {{FACE_SOUTH, 1, 1}}},
      {22, {{FACE_SOUTH, 0, 1}, {FACE_BOTTOM, 2, 1}}},
      {24, {{FACE_SOUTH, 1, 0}, {FACE_EAST, 1, 2}}}};
  bool hasSolvedCross = false;

  enum {
    SCRAMBLED = 0,
    WHITE_CROSS = 1,
    FIRST_LAYER = 2,
    SECOND_LAYER = 3,
    YELLOW_CROSS = 4,
    OLL = 5,
    PLL = 6,
    SOLVED = 7
  } currentState;

public:
  RubiksCubeAi(RubiksCube *c);
  RubiksCubeAi();
  ~RubiksCubeAi();

  void solve();
  void makeNextMove();
  void nextStep();

  void reset();

  std::string getStatus();

  void goToYellowCross(){currentState = SECOND_LAYER;}
private:
  std::vector<cubePos> getWhiteCrossPieces();
  int turnDirToTopEdges(cubePos p, int &turnDir);
  std::vector<int> turnDirToTopCorners(cubePos p, int blockType);
  void getWhiteCrossPiecesOnTop();

  void putWhiteCornersIntoFace();
  std::vector<std::vector<cubePos>>
  getWhiteCornerBlocks(std::vector<int> &indices);

  int getBlockIndexFromPos(cubePos p);
  bool isOpposite(Pixel col1, Pixel col2);
  std::vector<cubePos> getBlockCubePos(int index);
  std::vector<int> topToBottom(int blockType);
  void playAllMoves();

  int changeBlockIndex(int blockIndex);

  void addMoves(const std::vector<int> m);
  void makeCross();

  void makeWhiteCorners();
  bool getWhiteCornersBottom();

  void makeMiddleLayer();
  bool getEdgePiecesMiddle();

  void makeYellowCross();
  bool doYellowCross();

  void makeOLL();
  bool doOLL();

  void makePLL();
  bool doPLLCorners();
  bool doPLLEdges();
  //
  std::vector<int> getFrontRightSpinner();
  std::vector<int> getFrontLeftSpinner();
  std::vector<int> getBackRightSpinner();
  std::vector<int> getBackLeftSpinner();

  std::vector<int> getFrontRightSpinnerB();
  std::vector<int> getFrontLeftSpinnerB();
  std::vector<int> getBackRightSpinnerB();
  std::vector<int> getBackLeftSpinnerB();

  int getBlockIndex(std::vector<Pixel> colors);
  int getTargetBlockIndex(std::vector<Pixel> colors);

  bool isInCorrectPlace(std::vector<cubePos> positions);
};

#endif