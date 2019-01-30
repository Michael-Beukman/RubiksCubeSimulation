#ifndef UTILITY_H
#define UTILITY_H
#include "assortedStructs.h"
#include "block.h"
#include "matrix4x4.h"
#include "pixel.h"
#include "values.h"
#include <cmath>
enum faceDirections {
  FACE_SOUTH = 0,
  FACE_NORTH = 1,
  FACE_EAST = 2,
  FACE_WEST = 3,
  FACE_TOP = 4,
  FACE_BOTTOM = 5,
  FACE_MIDDLE = -1
};

matrix4x4 makeIdentity();

matrix4x4 makeZRotationMatrix(double angle);

matrix4x4 makeXRotationMatrix(double angle);

matrix4x4 makeYRotationMatrix(double angle);

matrix4x4 makeTranslationMatrix(double x, double y, double z);

matrix4x4 makeProjectionMatrix(double near, double far, double aspectRatio,
                               double fovRad);

matrix4x4 makePointAt(vec3d pos, vec3d &target, vec3d &up);

matrix4x4 quickInverse(matrix4x4 &m);

vec3d vectorInterectPlane(vec3d plane_p, vec3d plane_n, vec3d lineStart,
                          vec3d lineEnd, double &t);

int Triangle_ClipAgainstPlane(vec3d plane_p, vec3d plane_n, triangle &in_tri,
                              triangle &out_tri1, triangle &out_tri2);

std::vector<triangle> makeCube(double offsetX = 0, double offsetY = 0,
                               double offsetZ = 0);

std::vector<triangle> makeRubiksCube();

std::vector<triangle> makeBottomCorner(double offsetX = 0, double offsetY = 0,
                                       double offsetZ = 0);

std::vector<triangle> makeFace(int faceType, double offsetX = 0,
                               double offsetY = 0, double offsetZ = 0,
                               Pixel color = WHITE);

Block makeBlock(int face1, int face2, int face3 = -1, double xOffset = 0.0,
                double yOffset = 0.0, double zOffset = 0.0,
                Pixel color1 = WHITE, Pixel color2 = WHITE,
                Pixel color3 = WHITE);

#endif // UTILITY_H