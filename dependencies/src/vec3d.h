#ifndef VEC3D_H
#define VEC3D_H
#include <string>
#include <cmath>
class vec3d {
public:
  double x, y, z;
  double w = 1;
  vec3d(double _x = 0, double _y = 0, double _z = 0) : x(_x), y(_y), z(_z) {}

  vec3d operator+(vec3d other) {
    return {x + other.x, y + other.y, z + other.z};
  }

  vec3d operator*(double number) {
    return {x * number, y * number, z * number};
  }

  vec3d operator-(vec3d other) {
    return {x - other.x, y - other.y, z - other.z};
  }

  vec3d operator/(double number) {
    return {x / number, y / number, z / number};
  }

  vec3d crossProduct(vec3d other) {
    vec3d newV;
    newV.x = y * other.z - z * other.y;
    newV.y = z * other.x - x * other.z;
    newV.z = x * other.y - y * other.x;

    return newV;
  }

  double length() { return sqrt(x * x + y * y + z * z); }

  void normalize() {
    double val = length();
    if (val == 0)
      return;
    x /= val;
    y /= val;
    z /= val;
  }

  double dotProduct(vec3d other) {
    return x * other.x + y * other.y + z * other.z;
  }

  vec3d copy() { return {x, y, z}; }

  std::string toString() {
    return "x: " + std::to_string(x) + " y: " + std::to_string(y) +
           " z: " + std::to_string(z);
  }
};

#endif // VEC3D_H