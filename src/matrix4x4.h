#ifndef MATRIX4X4_H
#define MATRIX4X4_H
#include "vec3d.h"
#include <vector>
#include <iostream>
struct matrix4x4 {
  std::vector<std::vector<double>> m =
      std::vector<std::vector<double>>(4, std::vector<double>(4, 0));

  vec3d MultiplyMatrixVector(vec3d &i) {
    vec3d o;
    o.x = i.x * m[0][0] + i.y * m[1][0] + i.z * m[2][0] + i.w * m[3][0];
    o.y = i.x * m[0][1] + i.y * m[1][1] + i.z * m[2][1] + i.w * m[3][1];
    o.z = i.x * m[0][2] + i.y * m[1][2] + i.z * m[2][2] + i.w * m[3][2];
    o.w = i.x * m[0][3] + i.y * m[1][3] + i.z * m[2][3] + i.w * m[3][3];
    return o;
  }

  matrix4x4 multiplyMatrix(matrix4x4 other){
    matrix4x4 result;
    for (int c=0; c<4; ++c){
        for (int r=0; r<4; ++r){
            double sum = 0;
            for (int i=0; i<4; ++i){
                sum += m[r][i] * other.m[i][c];
            }
            result.m[r][c] = sum;
        }
    }
    return result;
  }

  void print(){
      for (int i=0; i< 4; ++i){
          for (int j=0; j<4;++j){
              std::cout << m[i][j] << " ";
          }
          std::cout <<std::endl;
      }
  }
};

#endif // MATRIX4X4_H