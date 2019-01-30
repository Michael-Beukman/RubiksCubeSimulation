#include "utility.h"
matrix4x4 makeIdentity() {
  matrix4x4 m;
  m.m[0][0] = 1.0f;
  m.m[1][1] = 1.0f;
  m.m[2][2] = 1.0f;
  m.m[3][3] = 1.0f;
  return m;
}

matrix4x4 makeZRotationMatrix(double angle) {
  matrix4x4 matRotateZ;
  matRotateZ.m[0][0] = cos(angle);
  matRotateZ.m[0][1] = sin(angle);
  matRotateZ.m[1][0] = -sin(angle);
  matRotateZ.m[1][1] = cos(angle);
  matRotateZ.m[2][2] = 1;
  matRotateZ.m[3][3] = 1;
  return matRotateZ;
}

matrix4x4 makeXRotationMatrix(double angle) {
  matrix4x4 matRotateX;
  matRotateX.m[0][0] = 1.0;
  matRotateX.m[1][1] = cos(angle);
  matRotateX.m[1][2] = sin(angle);
  matRotateX.m[2][1] = -sin(angle);
  matRotateX.m[2][2] = cos(angle);
  matRotateX.m[3][3] = 1.0;
  return matRotateX;
}

matrix4x4 makeYRotationMatrix(double angle) {
  matrix4x4 matRotateY;
  matRotateY.m[0][0] = cos(angle);
  matRotateY.m[0][2] = sin(angle);
  matRotateY.m[2][0] = -sin(angle);
  matRotateY.m[1][1] = 1.0;
  matRotateY.m[2][2] = cos(angle);
  matRotateY.m[3][3] = 1.0;
  return matRotateY;
}

matrix4x4 makeTranslationMatrix(double x, double y, double z) {
  matrix4x4 matrix = makeIdentity();
  matrix.m[3][0] = x;
  matrix.m[3][1] = y;
  matrix.m[3][2] = z;
  return matrix;
}

matrix4x4 makeProjectionMatrix(double near, double far, double aspectRatio,
                               double fovRad) {
  matrix4x4 matProj;
  matProj.m[0][0] = aspectRatio * fovRad;
  matProj.m[1][1] = fovRad;
  matProj.m[2][2] = far / (far - near);
  matProj.m[3][2] = (-far * near) / (far - near);
  matProj.m[2][3] = 1.0;
  matProj.m[3][3] = 0.0;
  return matProj;
}

matrix4x4 makePointAt(vec3d pos, vec3d &target, vec3d &up) {
  vec3d newForward = target - pos;
  newForward.normalize();

  vec3d a = newForward * up.dotProduct(newForward);
  vec3d newUp = up - a;
  newUp.normalize();

  vec3d newRight = newUp.crossProduct(newForward);

  matrix4x4 pointAt;
  pointAt.m[0][0] = newRight.x;
  pointAt.m[0][1] = newRight.y;
  pointAt.m[0][2] = newRight.z;
  pointAt.m[0][3] = 0.0;

  pointAt.m[1][0] = newUp.x;
  pointAt.m[1][1] = newUp.y;
  pointAt.m[1][2] = newUp.z;
  pointAt.m[1][3] = 0.0;

  pointAt.m[2][0] = newForward.x;
  pointAt.m[2][1] = newForward.y;
  pointAt.m[2][2] = newForward.z;
  pointAt.m[2][3] = 0.0;

  pointAt.m[3][0] = pos.x;
  pointAt.m[3][1] = pos.y;
  pointAt.m[3][2] = pos.z;
  pointAt.m[3][3] = 1.0;
  return pointAt;
}

matrix4x4 quickInverse(matrix4x4 &m) {
  matrix4x4 matrix;
  matrix.m[0][0] = m.m[0][0];
  matrix.m[0][1] = m.m[1][0];
  matrix.m[0][2] = m.m[2][0];
  matrix.m[0][3] = 0.0f;
  matrix.m[1][0] = m.m[0][1];
  matrix.m[1][1] = m.m[1][1];
  matrix.m[1][2] = m.m[2][1];
  matrix.m[1][3] = 0.0f;
  matrix.m[2][0] = m.m[0][2];
  matrix.m[2][1] = m.m[1][2];
  matrix.m[2][2] = m.m[2][2];
  matrix.m[2][3] = 0.0f;
  matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] +
                     m.m[3][2] * matrix.m[2][0]);
  matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] +
                     m.m[3][2] * matrix.m[2][1]);
  matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] +
                     m.m[3][2] * matrix.m[2][2]);
  matrix.m[3][3] = 1.0f;
  return matrix;
}

vec3d vectorInterectPlane(vec3d plane_p, vec3d plane_n, vec3d lineStart,
                          vec3d lineEnd, double &t) {

  plane_n.normalize();

  double plane_d = -plane_n.dotProduct(plane_p);
  double ad = lineStart.dotProduct(plane_n);
  double bd = lineEnd.dotProduct(plane_n);
  if (bd != ad) {
    t = (-plane_d - ad) / (bd - ad);
    vec3d lineStartToEnd = lineEnd - lineStart;
    vec3d lineToIntersect = lineStartToEnd * t;
    return lineStart + lineToIntersect;
  } else {
    t = 1;
    vec3d lineStartToEnd = lineEnd - lineStart;
    return lineStartToEnd + lineStartToEnd * (double)(-plane_d - ad);
  }
}

int Triangle_ClipAgainstPlane(vec3d plane_p, vec3d plane_n, triangle &in_tri,
                              triangle &out_tri1, triangle &out_tri2) {
  plane_n.normalize();

  // calc inside
  auto dist = [&](vec3d p) {
    return plane_n.dotProduct(p) - plane_n.dotProduct(plane_p);
  };

  vec3d *insidePoints[3];
  int insidePointsCount = 0;

  vec3d *outsidePoints[3];
  int outsidePointsCount = 0;

  double d0 = dist(in_tri.vertices[0]);
  double d1 = dist(in_tri.vertices[1]);
  double d2 = dist(in_tri.vertices[2]);
  if (d0 >= 0) {
    insidePoints[insidePointsCount++] = &in_tri.vertices[0];

  } else {
    outsidePoints[outsidePointsCount++] = &in_tri.vertices[0];
  }
  if (d1 >= 0) {
    insidePoints[insidePointsCount++] = &in_tri.vertices[1];

  } else {
    outsidePoints[outsidePointsCount++] = &in_tri.vertices[1];
  }
  if (d2 >= 0) {
    insidePoints[insidePointsCount++] = &in_tri.vertices[2];

  } else {
    outsidePoints[outsidePointsCount++] = &in_tri.vertices[2];
  }

  if (insidePointsCount == 0) {
    return 0;
  }

  if (insidePointsCount == 3) {
    out_tri1 = in_tri;
    return 1;
  }

  if (insidePointsCount == 1 && outsidePointsCount == 2) {
    out_tri1.color = in_tri.color;

    double t;
    out_tri1.vertices[0] = *insidePoints[0];

    out_tri1.vertices[1] = vectorInterectPlane(
        plane_p, plane_n, *insidePoints[0], *outsidePoints[0], t);

    out_tri1.vertices[2] = vectorInterectPlane(
        plane_p, plane_n, *insidePoints[0], *outsidePoints[1], t);

    return 1;
  }

  if (insidePointsCount == 2 && outsidePointsCount == 1) {
    // Triangle should be clipped. As two points lie inside the plane,
    // the clipped triangle becomes a "quad". Fortunately, we can
    // represent a quad with two new triangles

    // Copy appearance info to new triangles
    out_tri1.color = in_tri.color;

    out_tri2.color = in_tri.color;

    // The first triangle consists of the two inside points and a new
    // point determined by the location where one side of the triangle
    // intersects with the plane

    double t;

    out_tri1.vertices[0] = *insidePoints[0];

    out_tri1.vertices[1] = *insidePoints[1];

    out_tri1.vertices[2] = vectorInterectPlane(
        plane_p, plane_n, *insidePoints[0], *outsidePoints[0], t);

    // The second triangle is composed of one of he inside points, a
    // new point determined by the intersection of the other side of the
    // triangle and the plane, and the newly created point above
    out_tri2.vertices[0] = *insidePoints[1];

    out_tri2.vertices[1] = out_tri1.vertices[2];

    out_tri2.vertices[2] = vectorInterectPlane(
        plane_p, plane_n, *insidePoints[1], *outsidePoints[0], t);
    return 2; // Return two newly formed triangles which form a quad
  }
  return 0;
}

std::vector<triangle> makeCube(double offsetX, double offsetY, double offsetZ) {
  return {
      // SOUTH
      {{{0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ},
        {0.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ},
        {1.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ}},
       YELLOW},
      {{{0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ},
        {1.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ},
        {1.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ}},
       YELLOW},

      // EAST
      {{{1.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ},
        {1.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ},
        {1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ}},
       GREEN},
      {{{1.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ},
        {1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ},
        {1.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ}},
       GREEN},

      // NORTH
      {{{1.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ},
        {1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ},
        {0.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ}},
       WHITE},
      {{{1.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ},
        {0.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ},
        {0.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ}},
       WHITE},

      // WEST
      {{{0.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ},
        {0.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ},
        {0.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ}},
       BLUE},
      {{{0.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ},
        {0.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ},
        {0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ}},
       BLUE},

      // TOP
      {{{0.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ},
        {0.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ},
        {1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ}},
       RED},
      {{{0.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ},
        {1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ},
        {1.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ}},
       RED},

      // BOTTOM
      {{{1.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ},
        {0.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ},
        {0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ}},
       MAGENTA},
      {{{1.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ},
        {0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ},
        {1.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ}},
       MAGENTA}};
}

std::vector<triangle> makeBottomCorner(double offsetX, double offsetY,
                                       double offsetZ) {
  return {

      // SOUTH
      {{{0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ},
        {0.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ},
        {1.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ}},
       YELLOW},
      {{{0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ},
        {1.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ},
        {1.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ}},
       YELLOW},

      // EAST
      {{{1.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ},
        {1.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ},
        {1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ}},
       GREEN},
      {{{1.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ},
        {1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ},
        {1.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ}},
       GREEN},
      // BOTTOM
      {{{1.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ},
        {0.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ},
        {0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ}},
       MAGENTA},
      {{{1.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ},
        {0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ},
        {1.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ}},
       MAGENTA}};
}

std::vector<triangle> makeRubiksCube() {
  std::vector<triangle> all;
  all = {};
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
        
        Block b = makeBlock(face1, face2, face3, oX, oY, oZ, WHITE, RED, BLUE);
        // std::cout << "b " << b.getAllTriangles().size() << " " <<std::endl;
        auto tri = b.getAllTriangles();
        all.insert(all.end(), tri.begin(),
                   tri.end());
      }
    }
  }

  Face f;
  // auto ss = makeFace(FACE_SOUTH);
  // auto b = makeBlock(FACE_BOTTOM, FACE_SOUTH, -1);
  // all = b.getAllTriangles(); //{ss[0], ss[1]}; //makeBlock(FACE_BOTTOM,
  // FACE_EAST, FACE_SOUTH).getAllTriangles();
  return all;
}

std::vector<triangle> makeFace(int faceType, double offsetX, double offsetY,
                               double offsetZ, Pixel color) {

  std::vector<triangle> ts;
  switch (faceType) {
  case FACE_BOTTOM:
    ts = {{{{1.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ},
            {0.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ},
            {0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ}},
           color},
          {{{1.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ},
            {0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ},
            {1.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ}},
           color}};
    break;
  case FACE_TOP:
    ts = {{{{0.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ},
            {0.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ},
            {1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ}},
           color},
          {{{0.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ},
            {1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ},
            {1.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ}},
           color}};
    break;
  case FACE_SOUTH:
    ts = {{{{0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ},
            {0.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ},
            {1.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ}},
           color},
          {{{0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ},
            {1.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ},
            {1.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ}},
           color}};
    break;
  case FACE_NORTH:
    ts = {{{{1.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ},
            {1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ},
            {0.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ}},
           color},
          {{{1.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ},
            {0.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ},
            {0.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ}},
           color}};
    break;

  case FACE_WEST:
    ts = {{{{0.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ},
            {0.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ},
            {0.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ}},
           color},
          {{{0.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ},
            {0.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ},
            {0.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ}},
           color}};
    break;
  case FACE_EAST:
    ts = {{{{1.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ},
            {1.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ},
            {1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ}},
           color},
          {{{1.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ},
            {1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ},
            {1.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ}},
           color}};
    break;
  default:
    ts = {{{{1.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ},
            {1.0f + offsetX, 1.0f + offsetY, 0.0f + offsetZ},
            {1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ}},
           color},
          {{{1.0f + offsetX, 0.0f + offsetY, 0.0f + offsetZ},
            {1.0f + offsetX, 1.0f + offsetY, 1.0f + offsetZ},
            {1.0f + offsetX, 0.0f + offsetY, 1.0f + offsetZ}},
           color}};
  }
  return ts;
}

Block makeBlock(int face1, int face2, int face3, double xOffset, double yOffset,
                double zOffset, Pixel color1, Pixel color2, Pixel color3) {
  Block b;
  std::vector<int> opts;
  std::vector<Pixel> cols;
  if (face1 >= 0) {
    opts.push_back(face1);
    cols.push_back(color1);
  }
  if (face2 >= 0) {
    opts.push_back(face2);
    cols.push_back(color2);
  }
  if (face3 >= 0) {
    opts.push_back(face3);
    cols.push_back(color3);
  }

  for (int k=0; k<opts.size(); ++k) {
    auto p = makeFace(opts[k], xOffset, yOffset, zOffset, cols[k]);
    Face f;
    f.t1 = p[0];
    f.t2 = p[1];
    b.faces.push_back(f);
  }
  return b;
}
