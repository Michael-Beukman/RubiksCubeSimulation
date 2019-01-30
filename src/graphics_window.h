#ifndef GRAPHICS_WINDOW_H
#define GRAPHICS_WINDOW_H
#include "matrix4x4.h"
#include "vec3d.h"
#include "gameEngine.h"
#include <fstream>
#include "utility.h"
#include <vector>
#include "rubiksCube.h"
#include "rubiksCubeAi.h"

struct mesh {
  std::vector<triangle> tris;

  void loadFromFile(std::string filename, bool isJunk = false) {
    std::ifstream f(filename);
    if (f.is_open()) {
      std::string s;
      std::string junk;
      // junk lines
      if (isJunk) {
        getline(f, junk);
        getline(f, junk);
      }
      std::string type;
      std::vector<vec3d> vertices;
      while (f >> type) {
        if (type == "v") {
          vec3d temp;
          f >> temp.x >> temp.y >> temp.z;
          vertices.push_back(temp);
        }
        if (type == "f") {
          int a, b, c;
          f >> a >> b >> c;
          --a;
          --b;
          --c;
          if (a >= vertices.size() || b >= vertices.size() ||
              c >= vertices.size())
            throw std::out_of_range("face values not in range");
          tris.push_back({{vertices[a], vertices[b], vertices[c]}});
        }
        // f >> type;
      }
    } else {
      throw std::out_of_range("File not found");
    }
  }
};

class Graphics_Window : public Window {
private:
  bool quit;
  mesh meshCube;
  matrix4x4 matProj;
  matrix4x4 matRotateX;
  matrix4x4 matRotateZ;
  matrix4x4 matRotateY;
  double angleX = 0;
  double angleY = 0;
  double angleZ = 0;
  double yaw;
  vec3d vCamera;
  vec3d vLookDirection;
  RubiksCube myCube;

  RubiksCubeAi myCubeAi;

public:
  Graphics_Window(int _width, int _height);
  bool onStart() override;
  bool onUpdate(double timeElapsed) override;

  void handleEvents(SDL_Event &event, double timeElapsed) override;

  private:
  void writeControls();
};

#endif // GRAPHICS_WINDOW_H