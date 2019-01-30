#include "graphics_window.h"
#include "utility.h"
#include <map>
#include <unordered_map>
#include <list>

Graphics_Window::Graphics_Window(int _width, int _height)
    : Window(_width, _height, "3D Projection"), quit(false),
      texture("assets/textures/brick_wall_thick.txt"), myCube(),
      myCubeAi(&myCube) {}

bool Graphics_Window::onStart() {
  meshCube.tris = myCube.getAllTriangles();

  matProj = makeProjectionMatrix(0.1, 1000.0,
                                 (double)getHeight() / (double)getWidth(),
                                 1.0 / tan(90.0 * 0.5 / 180.0 * 3.14159));
  vCamera = {0, 0, 0};
  yaw = 0;
  //testing
  // int corr =0, bad=0;
  // int count = 500;
  // for (int i=0;i<count; ++i){
  //   std::cout << i <<std::endl;
  //   myCube.randomScramble();
  //   myCubeAi.reset();
  //   myCube.update();
  //   myCubeAi.solve();
  //   if (myCube.isSolved()) ++corr;
  //   else ++bad;
  // }
  // std::cout << corr << " correct out of "<<count << " " << bad << " wrong out of  " << count<<std::endl;
  // return false;
  return true;
}

bool Graphics_Window::onUpdate(double timeElapsed) {
  myCube.update();
  meshCube.tris = myCube.getAllTriangles();
  int numTrianglesRendered = 0;
  fillAll(BLACK);
  std::string s = "Solved: ";
  std::string s2 = (myCube.isSolved() ? "True" : "False");
  writeControls();
  drawText(0, 0, s + s2);
  drawText(0,100,myCubeAi.getStatus());
  // Rotation Z
  matRotateZ = makeZRotationMatrix(angleZ);
  matRotateY = makeYRotationMatrix(angleY);
  // Rotation X
  matRotateX = makeXRotationMatrix(angleX);

  matrix4x4 matTrans = makeTranslationMatrix(0, 0, 8.0f);
  matrix4x4 matWorld = makeIdentity();
  matWorld = matRotateZ.multiplyMatrix(matRotateX);
  matWorld = matWorld.multiplyMatrix(matRotateY);
  matWorld = matWorld.multiplyMatrix(matTrans);

  vec3d vUp = {0, -1, 0};
  vec3d vTarget = {0, 0, 1};
  matrix4x4 matRotateCamera = makeYRotationMatrix(yaw);
  vLookDirection = matRotateCamera.MultiplyMatrixVector(vTarget);
  vTarget = vCamera + vLookDirection;
  matrix4x4 matCamera = makePointAt(vCamera, vTarget, vUp);

  matrix4x4 matView = quickInverse(matCamera);

  std::vector<triangle> trianglesToRasterize;
  for (auto tri : meshCube.tris) {

    triangle triProjected, triTransformed, triViewed;
    // Transform
    for (int i = 0; i < 3; ++i)
      triTransformed.vertices[i] =
          matWorld.MultiplyMatrixVector(tri.vertices[i]);

    vec3d normal, line1, line2;
    line1 = triTransformed.vertices[1] - triTransformed.vertices[0];
    line2 = triTransformed.vertices[2] - triTransformed.vertices[0];
    normal = line1.crossProduct(line2);

    normal.normalize();

    vec3d cameraRay =
        triTransformed.vertices[0] - vCamera; // vec from camera to point

    if (normal.dotProduct(cameraRay) < 0) {
      // Illumination
      vec3d light_direction = vLookDirection.copy();
      light_direction = light_direction * -1;
      light_direction.normalize();
      double dp = std::max(0.1, normal.dotProduct(light_direction));
      uint8_t col = dp * 255;
      assert(col >= 0 && col <= 255);
      Pixel color(col);
      triViewed.color = tri.color;
      // Convert from world space --> View Space;
      for (int i = 0; i < 3; ++i)
        triViewed.vertices[i] =
            matView.MultiplyMatrixVector(triTransformed.vertices[i]);

      // depth clipping

      int numClippedTriangles = 0;
      triangle clipped[2];
      numClippedTriangles =
          Triangle_ClipAgainstPlane({0.0f, 0.0f, 0.5f}, {0.0f, 0.0f, 1.0f},
                                    triViewed, clipped[0], clipped[1]);
      for (int i = 0; i < numClippedTriangles; ++i) {
        for (int k = 0; k < 3; ++k) {

          triProjected.vertices[k] =
              matProj.MultiplyMatrixVector(clipped[i].vertices[k]);
        }

        triProjected.color = clipped[i].color;
        for (int k = 0; k < 3; ++k)
          triProjected.vertices[k] =
              triProjected.vertices[k] / triProjected.vertices[k].w;

        vec3d vOffsetView = {1, 1, 0};
        for (int k = 0; k < 3; ++k)
          triProjected.vertices[k] = triProjected.vertices[k] + vOffsetView;
        for (int k = 0; k < 3; ++k) {
          triProjected.vertices[k].x *= 0.5f * (float)getWidth();
          triProjected.vertices[k].y *= 0.5f * (float)getHeight();
        }

        // Store triangle for sorting
        trianglesToRasterize.push_back(triProjected);
      }
    }
  }
  sort(trianglesToRasterize.begin(), trianglesToRasterize.end(),
       [](triangle &t1, triangle &t2) {
         double z1 = (t1.vertices[0].z + t1.vertices[1].z + t1.vertices[2].z);
         double z2 = (t2.vertices[0].z + t2.vertices[1].z + t2.vertices[2].z);
         return z1 > z2;
       });
  for (auto triToraster : trianglesToRasterize) {
    triangle clipped[2];
    std::list<triangle> listTriangles;
    listTriangles.push_back(triToraster);
    int nNewTriangles = 1;
    for (int p = 0; p < 4; p++) {
      int nTrisToAdd = 0;
      while (nNewTriangles > 0) {
        triangle test = listTriangles.front();
        listTriangles.pop_front();
        nNewTriangles--;
        switch (p) {
        case 0:
          nTrisToAdd =
              Triangle_ClipAgainstPlane({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
                                        test, clipped[0], clipped[1]);
          break;
        case 1:
          nTrisToAdd = Triangle_ClipAgainstPlane(
              {0.0f, (float)getHeight() - 1, 0.0f}, {0.0f, -1.0f, 0.0f}, test,
              clipped[0], clipped[1]);
          break;
        case 2:
          nTrisToAdd =
              Triangle_ClipAgainstPlane({0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
                                        test, clipped[0], clipped[1]);
          break;
        case 3:
          nTrisToAdd = Triangle_ClipAgainstPlane(
              {(float)getWidth() - 1, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, test,
              clipped[0], clipped[1]);
          break;
        }

        for (int w = 0; w < nTrisToAdd; w++)
          listTriangles.push_back(clipped[w]);
      }
      nNewTriangles = listTriangles.size();
    }
    for (auto &tri : listTriangles) {
      try {
        numTrianglesRendered++;
        fillTriangle((int)tri.vertices[0].x, (int)tri.vertices[0].y,
                     (int)tri.vertices[1].x, (int)tri.vertices[1].y,
                     (int)tri.vertices[2].x, (int)tri.vertices[2].y, tri.color);

        drawTriangle((int)tri.vertices[0].x, (int)tri.vertices[0].y,
                     (int)tri.vertices[1].x, (int)tri.vertices[1].y,
                     (int)tri.vertices[2].x, (int)tri.vertices[2].y, BLACK);
      } catch (std::exception e) {
      }
    }
  }
  return !quit;
}

void Graphics_Window::handleEvents(SDL_Event &event, double timeElapsed) {
  auto state = SDL_GetKeyboardState(NULL);
  double speed = 20;
  vec3d forward = vLookDirection * speed * timeElapsed;
  vec3d up = {0, 1.0, 0};
  if (state[SDL_SCANCODE_RIGHT]) {
    vec3d sideways = forward.crossProduct(up);
    vCamera = vCamera + sideways;
  }
  if (state[SDL_SCANCODE_LEFT]) {
    vec3d sideways = forward.crossProduct(up);
    vCamera = vCamera - sideways;
  }
  if (state[SDL_SCANCODE_UP]) {
    vCamera.y += speed * timeElapsed;
  }
  if (state[SDL_SCANCODE_DOWN]) {
    vCamera.y -= speed * timeElapsed;
  }
  if (state[SDL_SCANCODE_A]) {
    yaw -= 2.0f * timeElapsed;
  }
  if (state[SDL_SCANCODE_D]) {
    yaw += 2.0f * timeElapsed;
  }
  if (state[SDL_SCANCODE_W]) {
    vCamera = vCamera + forward;
  }
  if (state[SDL_SCANCODE_S]) {
    vCamera = vCamera - forward;
  }
  if (state[SDL_SCANCODE_X]) {
    if (state[SDL_SCANCODE_LSHIFT]) {
      angleX -= timeElapsed;
    } else {
      angleX += timeElapsed;
    }
  }
  if (state[SDL_SCANCODE_Z]) {
    if (state[SDL_SCANCODE_LSHIFT]) {
      angleZ -= timeElapsed;
    } else {
      angleZ += timeElapsed;
    }
  }

  if (state[SDL_SCANCODE_Y]) {
    if (state[SDL_SCANCODE_LSHIFT]) {
      angleY -= timeElapsed;
    } else {
      angleY += timeElapsed;
    }
  }
  switch (event.type) {
  case SDL_QUIT:
    quit = true;
    break;
  case SDL_KEYDOWN:
    int turnDir = -1;
    switch (event.key.keysym.sym) {
    case SDLK_ESCAPE:
      quit = true;
      break;
    case SDLK_j:
      turnDir = LEFT;
      break;
    case SDLK_k:
      turnDir = RIGHT;
      break;
    case SDLK_u:
      turnDir = UP;
      break;
    case SDLK_i:
      turnDir = DOWN;
      break;
    case SDLK_f:
      turnDir = FRONT;
      break;
    case SDLK_b:
      turnDir = BACK;
      break;
    case SDLK_r:
      myCube.randomScramble();
      myCubeAi.reset();
      break;
    case SDLK_m:
      myCubeAi.makeNextMove();
      break;
    case SDLK_q:
      myCubeAi.solve();
      break;
      case SDLK_1:
      myCubeAi.goToYellowCross();
      break;
    case SDLK_p:
      myCubeAi.nextStep();
      break;
    default:
      break;
    }
    if (turnDir != -1) {
      if (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT]) {
        turnDir += 6;
      }
      myCube.turn(turnDir);
    }
    break;
  }
}


void Graphics_Window::writeControls(){
  int x = (getWidth() - 200 ) *2; //something wrong with positions xD
  int y = 0;
  std::vector<std::pair<std::string, std::string>> vals = {
    {"W","Forwards"},
    {"S","Backwards"},
    {"A","Turn left"},
    {"D","Turn right"},
    {"UP", "Up"},
    {"DOWN", "Down"},
    {"LEFT", "Left"},
    {"RIGHT", "Right"},
    {"",""},
    {"R", "Scramble"},
    {"Q", "Solve"},
    {"P", "Step once"},
    {"U", "Cube Up"},    
    {"I", "Cube Down"},    
    {"F", "Cube Front"},   
    {"B", "Cube Back"},   
    {"J", "Cube Left"},   
    {"K", "Cube Right"},   
    {"Shift", "Reverse direction"}
  };

  for (auto &p: vals){
    drawText(x,y, p.first + (p.first == "" ? "" : ": ")+p.second);
    y+= 40;
  }
}