#include "gameEngine.h"
Window::Window(int _width, int _height, std::string _appName)
    : width(_width), height(_height), appName(_appName) {

  pixels = new Uint32[width * height];

  memset(pixels, 0, width * height * sizeof(Uint32));
  init(width, height);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                              SDL_TEXTUREACCESS_STREAMING, width, height);
  imgTexture = NULL;
}

Window::~Window() { Cleanup(); }

int Window::init(int width, int height) {

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {

    return 1;
  } else {
    TTF_Init();
    window = SDL_CreateWindow(appName.c_str(), SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, width, height,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE |
                                  SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == NULL) {
      return 1;
    } else {
      renderer = SDL_CreateRenderer(
          window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
      return 0;
    }
  }
  // Set blending function
  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
  SDL_SetTextureAlphaMod(texture, 255);
}

void Window::Cleanup() {
  SDL_DestroyRenderer(renderer);
  renderer = NULL;
  if (window)
    SDL_DestroyWindow(window);
  window = NULL;
  delete[] pixels;
  SDL_DestroyTexture(texture);
  if (imgTexture)
    SDL_DestroyTexture(imgTexture);
  TTF_Quit();
  SDL_Quit();
}

void Window::sdlPixelDrawing() {
  SDL_RenderClear(renderer);
  if (bUsePixels)
    SDL_RenderCopy(renderer, texture, NULL, NULL);
  for (auto &s : sprites) {
    SDL_Rect dist = s.position;
    SDL_Rect src = s.sheetPos;
    if (src.x == -1) {
      SDL_RenderCopy(renderer, s.texture, NULL, &dist);
    } else {
      SDL_RenderCopy(renderer, s.texture, &src, &dist);
    }
  }
  drawText();
  SDL_RenderPresent(renderer);
  if (bUsePixels)SDL_UpdateTexture(texture, NULL, pixels, width *
  sizeof(Uint32)); 
  sprites.clear();
  textBlocks.clear();
}

int Window::getWidth() { return width; }

int Window::getHeight() { return height; }

/* Drawing */
void Window::draw(const int &x, const int &y, const Pixel &color) {
  if (x >= width || x < 0 || y >= height || y < 0)
    throw std::out_of_range("x or y not in range");
  pixels[y * width + x] = color.color;
}

void Window::drawLine(int x, int y, int endX, int endY, const Pixel &color) {
  if (endX < x) {
    std::swap(endX, x);
    std::swap(endY, y);
  }
  double dx = abs(endX - x);
  double dy = abs(endY - y);
  if (dx == 0) {
    drawVerticalLine(x, y, endY, color);
  } else if (dy == 0) {
    drawHorisontalLine(y, x, endX, color);
  } else {
    double maxi = std::max(dx, dy);
    dx = (double)dx / (double)maxi;
    dy = (double)(endY - y) / (double)maxi;
    double xWalk = x;
    double yWalk = y;
    while (xWalk <= endX) {
      draw((int)xWalk, (int)yWalk, color);
      xWalk += dx;
      yWalk += dy;
    }
  }
}

void Window::Window::drawTriangle(int x1, int y1, int x2, int y2, int x3,
                                  int y3, const Pixel &color) {
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x3, y3, color);
  drawLine(x3, y3, x1, y1, color);
}

/* Filling */
void Window::fillAll(const Pixel &color) {
  std::fill(pixels, pixels + width * height, color.color);
  return;
  fill(0, 0, ScreenWidth(), ScreenHeight(), color);
}

void Window::fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3,
                          const Pixel &color) {
  std::vector<vec3d> points = getSortedPoints(x1, y1, x2, y2, x3, y3);
  if (points[0].y == points[1].y) {
    // top down
    fillTopDownTriangle(x1, y1, x2, y2, x3, y3, color);
  } else if (points[1].y == points[2].y) {
    // top up
    fillTopUpTriangle(x1, y1, x2, y2, x3, y3, color);
  } else {
    // neither top up nor top down
    vec3d midPoint = points[1];
    vec3d start = points[0];
    vec3d end = points[2];

    vec3d directionVec = end - start;

    // now any point on that line can be represented as start + lambda *
    // directionVec we want the value of lambda that satisfies: start.y +
    // dir.y = midPoint.y
    if (directionVec.y != 0) {
      double lambda = (midPoint.y - start.y) / (double)directionVec.y;
      double xPos = start.x + lambda * directionVec.x;
      // now just draw the two flat triangles;
      fillTopUpTriangle(start.x, start.y, xPos, midPoint.y, midPoint.x,
                        midPoint.y, color);
      fillTopDownTriangle(end.x, end.y, xPos, midPoint.y, midPoint.x,
                          midPoint.y, color);
    } else {
      throw std::overflow_error(
          "direction vec's y is 0, which means it is flat!");
    }
  }
}

void Window::fillTopUpTriangle(int x1, int y1, int x2, int y2, int x3, int y3,
                               const Pixel &color) {
  fillFlatTriangle(x1, y1, x2, y2, x3, y3, true, color);
}

void Window::fillTopDownTriangle(int x1, int y1, int x2, int y2, int x3, int y3,
                                 const Pixel &color) {

  fillFlatTriangle(x1, y1, x2, y2, x3, y3, false, color);
}

void Window::fillFlatTriangle(int x1, int y1, int x2, int y2, int x3, int y3,
                              bool isTopUp, const Pixel &color) {
  std::vector<vec3d> points = getSortedPoints(x1, y1, x2, y2, x3, y3);

  vec3d point;
  vec3d b;
  vec3d c;
  if (isTopUp) {
    point = points[0];
    b = points[1];
    c = points[2];
  } else {
    // bottom point is down
    b = points[0];
    c = points[1];
    point = points[2];
  }

  vec3d line1 = b - point;
  vec3d line2 = c - point;
  line1.normalize();
  line2.normalize();

  vec3d walkingA = point.copy();
  vec3d walkingB = point.copy();

  while (isGreaterLessThan(walkingA.y, b.y, !isTopUp)) {
    walkingA =
        walkingA + line1 * (1 / (double)(line1.y != 0 ? abs(line1.y) : 1));
    walkingB =
        walkingB + line2 * (1 / (double)(line2.y != 0 ? abs(line2.y) : 1));
    drawHorisontalLine((int)walkingA.y, (int)walkingA.x, (int)walkingB.x,
                       color);
  }
}
/* Utils */
void Window::drawHorisontalLine(int y, int x, int endX, const Pixel &color) {
  minMax(x, endX);
  for (int xWalk = x; xWalk <= endX; ++xWalk) {
    draw(xWalk, y, color);
  }
}

void Window::drawVerticalLine(int x, int y, int endY, const Pixel &color) {
  minMax(y, endY);
  for (int yWalk = y; yWalk <= endY; ++yWalk) {
    draw(x, yWalk, color);
  }
}

void Window::minMax(int &x, int &y) {
  if (x > y)
    std::swap(x, y);
}

void Window::minMax(double &x, double &y) {
  if (x > y)
    std::swap(x, y);
}

std::vector<vec3d> Window::getSortedPoints(int x1, int y1, int x2, int y2,
                                           int x3, int y3, bool sortOnY) {
  std::vector<vec3d> points = {{(double)x1, (double)y1},
                               {(double)x2, (double)y2},
                               {(double)x3, (double)y3}};
  if (sortOnY) {
    std::sort(points.begin(), points.end(), [&](vec3d i1, vec3d i2) {
      if (i1.y == i2.y) {
        return i1.x > i2.x;
      }
      return i1.y < i2.y;
    });
  } else {
    std::sort(points.begin(), points.end(), [&](vec3d i1, vec3d i2) {
      if (i1.x == i2.x) {
        return i1.y > i2.y;
      }
      return i1.x < i2.x;
    });
  }
  return points;
}
/* Loop */
void Window::Start() {
  bool quit = !onStart();
  auto tp1 = std::chrono::system_clock::now();
  auto tp2 = std::chrono::system_clock::now();
  double totalTime = 0.0;
  while (!quit) {
    tp2 = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsedTime = tp2 - tp1;

    timeElapsed = elapsedTime.count();
    totalTime += timeElapsed;
    SDL_SetWindowTitle(
        window, (appName + " " + std::to_string((int)round(1.0 / timeElapsed)))
                    .c_str());
    tp1 = tp2;
    events(timeElapsed);
    if (done)
      break;
    quit = !onUpdate(timeElapsed);
    sdlPixelDrawing();
  }
  std::cout << "loop end" << std::endl;
}

bool Window::onUpdate(double timeElapsed) { return true; }

bool Window::onStart() { return true; }

void Window::events(double timeElapsed) {
  SDL_Event event;
  handleEvents(event, timeElapsed);
  while (SDL_PollEvent(&event) != 0) {
    handleEvents(event, timeElapsed);
    if (event.type == SDL_QUIT) {
      done = true;
    }
  }
}

void Window::handleQuit(SDL_Event &event) {}

void Window::handleEvents(SDL_Event &event, double timeElapsed) {}

void Window::fillTexturedTriangle(int x1, int y1, double u1, double v1, int x2,
                                  int y2, double u2, double v2, int x3, int y3,
                                  double u3, double v3, Texture &tex) {
  if (y2 < y1) {
    std::swap(y1, y2);
    std::swap(x1, x2);
    std::swap(u1, u2);
    std::swap(v1, v2);
  }

  if (y3 < y1) {
    std::swap(y1, y3);
    std::swap(x1, x3);
    std::swap(u1, u3);
    std::swap(v1, v3);
  }

  if (y3 < y2) {
    std::swap(y2, y3);
    std::swap(x2, x3);
    std::swap(u2, u3);
    std::swap(v2, v3);
  }

  int dy1 = y2 - y1;
  int dx1 = x2 - x1;
  double dv1 = v2 - v1;
  double du1 = u2 - u1;

  int dy2 = y3 - y1;
  int dx2 = x3 - x1;
  double dv2 = v3 - v1;
  double du2 = u3 - u1;

  double daxStep = 0, dbxStep = 0;
  double du1Step = 0, dv1Step = 0;
  double du2Step = 0, dv2Step = 0;

  if (dy1)
    daxStep = dx1 / (double)abs(dy1);
  if (dy2)
    dbxStep = dx2 / (double)abs(dy2);

  if (dy1)
    du1Step = du1 / (double)abs(dy1);
  if (dy1)
    dv1Step = dv1 / (double)abs(dy1);

  if (dy2)
    du2Step = du2 / (double)abs(dy2);
  if (dy2)
    dv2Step = dv2 / (double)abs(dy2);

  if (dy1) {
    // top half of triangle
    for (int i = y1; i <= y2; ++i) {

      int ax = x1 + (double)(i - y1) * daxStep;
      int bx = x1 + (double)(i - y1) * dbxStep;
      double tex_su = u1 + (double)(i - y1) * du1Step;
      double tex_sv = v1 + (double)(i - y1) * dv1Step;

      double tex_eu = u1 + (double)(i - y1) * du2Step;
      double tex_ev = v1 + (double)(i - y1) * dv2Step;

      if (ax > bx) {
        std::swap(ax, bx);
        std::swap(tex_su, tex_eu);
        std::swap(tex_sv, tex_ev);
      }

      double texU, texV;

      texU = tex_su;
      texV = tex_sv;

      double tStep;
      if (ax != bx)
        tStep = 1.0 / ((double)(bx - ax));
      else
        tStep = 0;

      double t = 0.0f;

      for (int j = ax; j <= bx; ++j) {
        texU = (1.0 - t) * tex_su + t * tex_eu;
        texV = (1.0 - t) * tex_sv + t * tex_ev;
        draw(j, i, tex.sampleTexture(texU, texV));
        t += tStep;
      }
    }
  }

  dy1 = y3 - y2;
  dx1 = x3 - x2;
  dv1 = v3 - v2;
  du1 = u3 - u2;

  if (dy1)
    daxStep = dx1 / (double)abs(dy1);
  if (dy2)
    dbxStep = dx2 / (double)abs(dy2);

  du1Step = 0, dv1Step = 0;
  du2Step = 0, dv2Step = 0;
  if (dy1)
    du1Step = du1 / (double)abs(dy1);
  if (dy1)
    dv1Step = dv1 / (double)abs(dy1);

  if (dy2)
    du2Step = du2 / (double)abs(dy2);
  if (dy2)
    dv2Step = dv2 / (double)abs(dy2);
  if (dy1) {

    for (int i = y2; i <= y3; ++i) {
      int ax = x2 + (double)(i - y2) * daxStep;
      int bx = x1 + (double)(i - y1) * dbxStep;
      double tex_su = u2 + (double)(i - y2) * du1Step;
      double tex_sv = v2 + (double)(i - y2) * dv1Step;

      double tex_eu = u1 + (double)(i - y1) * du2Step;
      double tex_ev = v1 + (double)(i - y1) * dv2Step;

      if (ax > bx) {
        std::swap(ax, bx);
        std::swap(tex_su, tex_eu);
        std::swap(tex_sv, tex_ev);
      }
      double texU, texV;

      texU = tex_su;
      texV = tex_sv;

      double tStep;
      if (ax != bx)
        tStep = 1.0 / ((double)(bx - ax));
      else
        tStep = 0;

      double t = 0.0f;

      for (int j = ax; j <= bx; ++j) {
        texU = (1.0 - t) * tex_su + t * tex_eu;
        texV = (1.0 - t) * tex_sv + t * tex_ev;
        // std::cout << j << "\n";
        draw(j, i, tex.sampleTexture(texU, texV));
        t += tStep;
      }
    }
  }
}

bool Window::isGreaterLessThan(double a, double b, bool checkGeq) {
  if (checkGeq)
    return a > b;
  return a < b;
}
void Window::fill(int x, int y, int endX, int endY, const Pixel &color) {
  int dx = (endX - x > 0 ? 1 : -1);
  int dy = (endY - y > 0 ? 1 : -1);
  for (int j = y; j != endY; ++j) {
    for (int i = x; i != endX; ++i) {
      draw(i, j, color);
    }
  }
}

void Window::addSpriteSheet(std::string filename) {
  // Load the image
  SDL_Surface *loadedSurface = IMG_Load(filename.c_str());
  // Check that it loaded successfully
  if (loadedSurface == nullptr)
    throw std::runtime_error(std::string("Couldn't load sprite sheet: ") +
                             IMG_GetError());

  if (imgTexture) {
    SDL_DestroyTexture(imgTexture);
    imgTexture = NULL;
  }
  imgTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
  SDL_SetTextureBlendMode(imgTexture, SDL_BLENDMODE_BLEND);
  if (imgTexture == nullptr)
    throw std::runtime_error(std::string("Couldn't create sprite texture: ") +
                             SDL_GetError());

  SDL_FreeSurface(loadedSurface);
}

void Window::removeSpriteSheet() {
  if (imgTexture) {
    SDL_DestroyTexture(imgTexture);
    imgTexture = NULL;
  }
}

void Window::makeSprite(std::string filename, Sprite *s, int bgR, int bgG,
                        int bgB) {
  auto isTextFile = [](std::string name) {
    if (name.substr(name.length() - 4) == ".txt") {
      return true;
    }
    return false;
  };
  if (isTextFile(filename)) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cout << "Could not open file " << filename << std::endl;
      return;
    }
    int w, h;
    file >> w >> h;
    Uint32 *spritePixels = new Uint32[w * h];

    memset(spritePixels, 0, w * h * sizeof(Uint32));
    for (int j = 0; j < h; ++j) {
      for (int i = 0; i < w; ++i) {

        file >> spritePixels[i + j * w];
      }
    }
    s->surface = SDL_CreateRGBSurfaceFrom((void *)spritePixels, w, h, 32,
                                          w * sizeof(Uint32), 0x000000ff,
                                          0x0000ff00, 0x00ff0000, 0xff000000);
    s->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                                   SDL_TEXTUREACCESS_STATIC, w, h);
    SDL_UpdateTexture(s->texture, NULL, spritePixels, w * sizeof(Uint32));
  } else {
    // Load the image
    SDL_Surface *loadedSurface = IMG_Load(filename.c_str());
    // Check that it loaded successfully
    if (loadedSurface == nullptr)
      throw std::runtime_error(std::string("Couldn't load sprite sheet: ") +
                               IMG_GetError());

    SDL_SetColorKey(loadedSurface, SDL_TRUE,
                    SDL_MapRGB(loadedSurface->format, bgR, bgG, bgB));

    s->texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    s->surface = loadedSurface;
    if (s->texture == nullptr)
      throw std::runtime_error(std::string("Couldn't create sprite texture: ") +
                               SDL_GetError());

    // SDL_FreeSurface(loadedSurface);
  }
  SDL_SetTextureBlendMode(s->texture, SDL_BLENDMODE_BLEND);
}

void Window::drawSprite(Sprite *s, int x, int y, int endX, int endY) {
  x *= 2;
  y *= 2;
  endX *= 2;
  endY *= 2;
  int minX = std::min(x, endX);
  int minY = std::min(y, endY);

  int w = abs(endX - x);
  int h = abs(endY - y);

  Sprite newS;
  newS.texture = s->texture;
  newS.position = makeSDL_Rect(x, y, endX, endY); // {minX, minY, w, h};
  sprites.push_back(newS);
}

SDL_Rect Window::makeSDL_Rect(int x, int y, int endX, int endY) {

  int minX = std::min(x, endX);
  int minY = std::min(y, endY);

  int w = abs(endX - x);
  int h = abs(endY - y);

  return {minX, minY, w, h};
}

void Window::drawPartialSprite(Sprite *s, int x, int y, int endX, int endY,
                               int sheetX, int sheetY, int sheetEndX,
                               int sheetEndY) {
  x *= 2;
  y *= 2;
  endX *= 2;
  endY *= 2;

  Sprite newS;
  newS.texture = s->texture;
  newS.position = makeSDL_Rect(x, y, endX, endY); // {minX, minY, w, h};
  newS.sheetPos = makeSDL_Rect(sheetX, sheetY, sheetEndX, sheetEndY);
  sprites.push_back(newS);
}

void Window::drawWireFrameModel(
    const std::vector<std::pair<double, double>> &vecModelCoordinates, double x,
    double y, double r, double s, const Pixel &color) {
  // pair.first = x coordinate
  // pair.second = y coordinate

  // Create translated model vector of coordinate pairs
  std::vector<std::pair<double, double>> vecTransformedCoordinates;
  int verts = vecModelCoordinates.size();
  vecTransformedCoordinates.resize(verts);

  // Rotate
  for (int i = 0; i < verts; i++) {
    vecTransformedCoordinates[i].first =
        vecModelCoordinates[i].first * cosf(r) -
        vecModelCoordinates[i].second * sinf(r);
    vecTransformedCoordinates[i].second =
        vecModelCoordinates[i].first * sinf(r) +
        vecModelCoordinates[i].second * cosf(r);
  }

  // Scale
  for (int i = 0; i < verts; i++) {
    vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first * s;
    vecTransformedCoordinates[i].second =
        vecTransformedCoordinates[i].second * s;
  }

  // Translate
  for (int i = 0; i < verts; i++) {
    vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first + x;
    vecTransformedCoordinates[i].second =
        vecTransformedCoordinates[i].second + y;
  }

  // Draw Closed Polygon
  for (int i = 0; i < verts + 1; i++) {
    int j = (i + 1);
    drawLine(vecTransformedCoordinates[i % verts].first,
             vecTransformedCoordinates[i % verts].second,
             vecTransformedCoordinates[j % verts].first,
             vecTransformedCoordinates[j % verts].second, color);
  }
}

int Window::ScreenWidth() { return width; }

int Window::ScreenHeight() { return height; }

void Window::drawText(int x, int y, std::string text, int endX, int endY,
                      int fontSize, const Pixel &color) {

  Text t;
  t.x = x;
  t.y = y;
  t.message = text;
  t.col = {color.red, color.blue, color.green};
  if (endX >= 0 && endY >= 0) {
    t.w = abs(endX - x);
    t.h = abs(endY - y);
  }
  t.fontSize = fontSize;
  textBlocks.push_back(t);
}

void Window::drawText() {

  for (auto text : textBlocks) {
    TTF_Font *font =
        TTF_OpenFont("/Library/Fonts/Andale Mono.ttf",
                     text.fontSize); // this opens a font style and sets a size
    if (font == NULL) {
      throw std::runtime_error("font is null");
    }
    SDL_Surface *surfaceMessage =
        TTF_RenderText_Solid(font, text.message.c_str(), text.col);

    SDL_Texture *Message = SDL_CreateTextureFromSurface(
        renderer, surfaceMessage); // now you can convert it into a texture

    int texW = text.w;
    int texH = text.h;
    if (texW == -1) {
      SDL_QueryTexture(Message, NULL, NULL, &texW, &texH);
    }
    SDL_Rect dstrect = {text.x, text.y, texW, texH};
    SDL_RenderCopy(renderer, Message, NULL, &dstrect);
    SDL_DestroyTexture(Message);
    SDL_FreeSurface(surfaceMessage);
    TTF_CloseFont(font);
  }
}

std::string Window::pixelToString(const Pixel &p) {
  if (p == WHITE) {
    return "WHITE";
  }
  if (p == YELLOW) {
    return "YELLOW";
  }
  if (p == ORANGE) {
    return "ORANGE";
  }
  if (p == RED) {
    return "RED";
  }
  if (p == GREEN) {
    return "GREEN";
  }

  if (p == BLUE) {
    return "BLUE";
  }
  if (p == MAGENTA) {
    return "MAGENTA";
  }

  return std::to_string(p.color);
}

void Window::fillTexturedTriangle(int x1, int y1, double u1, double v1,
                                  double w1, int x2, int y2, double u2,
                                  double v2, double w2, int x3, int y3,
                                  double u3, double v3, double w3, Texture &tex,
                                  double *depthBuffer) {

  if (y2 < y1) {
    std::swap(y1, y2);
    std::swap(x1, x2);
    std::swap(u1, u2);
    std::swap(v1, v2);
    std::swap(w1, w2);
  }

  if (y3 < y1) {
    std::swap(y1, y3);
    std::swap(x1, x3);
    std::swap(u1, u3);
    std::swap(v1, v3);
    std::swap(w1, w3);
  }

  if (y3 < y2) {
    std::swap(y2, y3);
    std::swap(x2, x3);
    std::swap(u2, u3);
    std::swap(v2, v3);
    std::swap(w2, w3);
  }

  int dy1 = y2 - y1;
  int dx1 = x2 - x1;
  double dv1 = v2 - v1;
  double du1 = u2 - u1;
  double dw1 = w2 - w1;

  int dy2 = y3 - y1;
  int dx2 = x3 - x1;
  double dv2 = v3 - v1;
  double du2 = u3 - u1;
  double dw2 = w3 - w1;

  double daxStep = 0, dbxStep = 0;
  double du1Step = 0, dv1Step = 0;
  double du2Step = 0, dv2Step = 0;
  double dw1Step = 0, dw2Step = 0;
  if (dy1)
    daxStep = dx1 / (double)abs(dy1);
  if (dy2)
    dbxStep = dx2 / (double)abs(dy2);

  if (dy1) {
    du1Step = du1 / (double)abs(dy1);
    dv1Step = dv1 / (double)abs(dy1);
    dw1Step = dw1 / (double)abs(dy1);
  }
  if (dy2) {
    du2Step = du2 / (double)abs(dy2);
    dv2Step = dv2 / (double)abs(dy2);
    dw2Step = dw2 / (double)abs(dy2);
  }

  if (dy1) {
    // top half of triangle
    for (int i = y1; i <= y2; ++i) {

      int ax = x1 + (double)(i - y1) * daxStep;
      int bx = x1 + (double)(i - y1) * dbxStep;
      double tex_su = u1 + (double)(i - y1) * du1Step;
      double tex_sv = v1 + (double)(i - y1) * dv1Step;
      double tex_sw = w1 + (double)(i - y1) * dw1Step;

      double tex_eu = u1 + (double)(i - y1) * du2Step;
      double tex_ev = v1 + (double)(i - y1) * dv2Step;
      double tex_ew = w1 + (double)(i - y1) * dw2Step;

      if (ax > bx) {
        std::swap(ax, bx);
        std::swap(tex_su, tex_eu);
        std::swap(tex_sv, tex_ev);
        std::swap(tex_sw, tex_ew);
      }

      double texU, texV, texW;

      double tStep;
      if (ax != bx)
        tStep = 1.0 / ((double)(bx - ax));
      else
        tStep = 0;

      double t = 0.0f;

      for (int j = ax; j <= bx; ++j) {
        texU = (1.0 - t) * tex_su + t * tex_eu;
        texV = (1.0 - t) * tex_sv + t * tex_ev;
        texW = (1.0 - t) * tex_sw + t * tex_ew;

        double sampleU = texU;
        double sampleV = texV;
        if (texW != 0) {
          sampleU /= texW;
          sampleV /= texW;
        }
        if (depthBuffer) {
          if (texW > depthBuffer[i * ScreenWidth() + j]) {
            depthBuffer[i * ScreenWidth() + j] = texW;
            draw(j, i, tex.sampleTexture(sampleU, sampleV));
          }
        } else {
          draw(j, i, tex.sampleTexture(sampleU, sampleV));
        }
        t += tStep;
      }
    }
  }
  dy1 = y3 - y2;
  dx1 = x3 - x2;
  dv1 = v3 - v2;
  du1 = u3 - u2;
  dw1 = w3 - w2;

  if (dy1)
    daxStep = dx1 / (double)abs(dy1);
  if (dy2)
    dbxStep = dx2 / (double)abs(dy2);

  du1Step = 0, dv1Step = 0;
  du2Step = 0, dv2Step = 0;
  if (dy1) {
    du1Step = du1 / (double)abs(dy1);
    dv1Step = dv1 / (double)abs(dy1);
    dw1Step = dw1 / (double)abs(dy1);
  }

  if (dy2) {
    du2Step = du2 / (double)abs(dy2);
    dv2Step = dv2 / (double)abs(dy2);
    dw2Step = dw2 / (double)abs(dy2);
  }
  if (dy1) {

    for (int i = y2; i <= y3; ++i) {
      int ax = x2 + (double)(i - y2) * daxStep;
      int bx = x1 + (double)(i - y1) * dbxStep;
      double tex_su = u2 + (double)(i - y2) * du1Step;
      double tex_sv = v2 + (double)(i - y2) * dv1Step;
      double tex_sw = w2 + (double)(i - y2) * dw1Step;

      double tex_eu = u1 + (double)(i - y1) * du2Step;
      double tex_ev = v1 + (double)(i - y1) * dv2Step;
      double tex_ew = w1 + (double)(i - y1) * dw2Step;

      if (ax > bx) {
        std::swap(ax, bx);
        std::swap(tex_su, tex_eu);
        std::swap(tex_sv, tex_ev);
        std::swap(tex_sw, tex_ew);
      }
      double texU, texV, texW;

      texU = tex_su;
      texV = tex_sv;
      texW = tex_sw;

      double tStep;
      if (ax != bx)
        tStep = 1.0 / ((double)(bx - ax));
      else
        tStep = 0;

      double t = 0.0f;

      for (int j = ax; j <= bx; ++j) {
        texU = (1.0 - t) * tex_su + t * tex_eu;
        texV = (1.0 - t) * tex_sv + t * tex_ev;
        texW = (1.0 - t) * tex_sw + t * tex_ew;

        double sampleU = texU;
        double sampleV = texV;
        if (texW != 0) {
          sampleU /= texW;
          sampleV /= texW;
        }
        if (depthBuffer) {
          if (texW > depthBuffer[i * ScreenWidth() + j]) {
            depthBuffer[i * ScreenWidth() + j] = texW;
            draw(j, i, tex.sampleTexture(sampleU, sampleV));
          }
        } else {
          draw(j, i, tex.sampleTexture(sampleU, sampleV));
        }
        t += tStep;
      }
    }
  }
}

Texture Window::getTextureFromImage(std::string filename, int bgR, int bgG,
                                    int bgB) {

  // Load the image
  SDL_Surface *loadedSurface = IMG_Load(filename.c_str());
  // Check that it loaded successfully
  if (loadedSurface == nullptr)
    throw std::runtime_error(std::string("Couldn't load sprite sheet: ") +
                             IMG_GetError());

  SDL_SetColorKey(loadedSurface, SDL_TRUE,
                  SDL_MapRGB(loadedSurface->format, bgR, bgG, bgB));

  int bpp = loadedSurface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to retrieve */
  int w = loadedSurface->w;
  int h = loadedSurface->h;
  std::vector<std::vector<uint32_t>> vTex(h, std::vector<uint32_t>(w, 0));
  for (int x = 0; x < w; ++x) {
    for (int y = 0; y < h; ++y) {

      Uint8 *p =
          (Uint8 *)loadedSurface->pixels + y * loadedSurface->pitch + x * bpp;

      switch (bpp) {
      case 1:
        vTex[y][x] = (uint32_t)*p;
        break;

      case 2:
        vTex[y][x] = (uint32_t) * (Uint16 *)p;
        break;

      case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
          vTex[y][x] = p[0] << 16 | p[1] << 8 | p[2];
        else
          vTex[y][x] = (p[0] | p[1] << 8 | p[2] << 16);
        break;

      case 4:
        vTex[y][x] = *(Uint32 *)p;
        break;

      default:
        vTex[y][x] = 0; /* shouldn't happen, but avoids warnings */
      }
    }
  }
  SDL_FreeSurface(loadedSurface);
  return Texture(vTex);
}

void Window::projectTexture(const Texture &src, Texture &dst, int sx, int sy,
                            int ex, int ey) {
  std::vector<std::vector<uint32_t>> newTex(ey - sy,
                                            std::vector<uint32_t>(ex - sx));
  for (int x = sx; x < ex; ++x) {
    for (int y = sy; y < ey; ++y) {
      newTex[y - sy][x - sx] = src.texture[y][x];
    }
  }
  dst.texture = newTex;
  dst.width = ex - sx;
  dst.height = ey - sy;
}

void Window::makeSprite(Uint32 *pixels, int width, int height, Sprite *s) {
  s->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                                 SDL_TEXTUREACCESS_STATIC, width, height);
  s->surface = SDL_CreateRGBSurfaceFrom((void *)pixels, width, height, 32,
                                        width * sizeof(Uint32), 0x000000ff,
                                        0x0000ff00, 0x00ff0000, 0xff000000);
  SDL_UpdateTexture(s->texture, NULL, pixels, width * sizeof(Uint32));
}

void Window::makeSpriteFromSprFile(std::string filename, Sprite *s) {
  makeSpriteFromSprFile(filename, s, {});
}

void Window::makeSpriteFromSprFile(std::string filename, Sprite *s,
                                   std::vector<Pixel> alphaColors) {
  short *pixels;
  int width, height;
  auto ReadData = [&](std::istream &is) {
    is.read((char *)&width, sizeof(int));
    is.read((char *)&height, sizeof(int));
    pixels = new short[width * height];
    is.read((char *)pixels, width * height * sizeof(short));
    // wchar_t* chars = new wchar_t[width*height];
    // is.read((char *)chars, width * height * sizeof(wchar_t));
    // for (int i=0; i<width*height; ++i){
    //   if (i<100)
    //   std::cout << chars[i] << " " << pixels[i]<<std::endl ;
    //   if (chars[i]==629679496) pixels[i] = 0x10;
    // }
    // std::cout <<width*height<< std::endl;
  };

  std::ifstream ifs;
  ifs.open(filename, std::ifstream::binary);
  if (ifs.is_open()) {
    ReadData(ifs);
  } else {
    throw std::runtime_error("Could not open file " + filename);
  }
  s->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                                 SDL_TEXTUREACCESS_STATIC, width, height);
  Uint32 *p = new Uint32[width * height];
  Uint32 format = SDL_GetWindowPixelFormat(window);
  SDL_PixelFormat *mappingFormat = SDL_AllocFormat(format);
  for (int i = 0; i < width * height; ++i) {

    auto s = pixels[i];

    p[i] = 0;
    Pixel temp = BLACK;
    for (int k : {0xF, 0xF0})
      switch (s & k) {
      case 0x10:
      case 0x01:
        temp = temp + DARK_BLUE;
        break;
      case 0x20:
      case 0x02:
        temp = temp + DARK_GREEN;
        break;
      case 0x30:
      case 0x03:
        temp = temp + DARK_CYAN;
        break;
      case 0x40:
      case 0x04:
        temp = temp + DARK_RED;
        break;
      case 0x50:
      case 0x05:
        temp = temp + DARK_MAGENTA;
        break;

      case 0x60:
      case 0x06:
        temp = temp + DARK_YELLOW;
        break;
      case 0x70:
      case 0x07:
        temp = temp + GREY;
        break;
      case 0x80:
      case 0x08:
        temp = temp + DARK_GREY;
        break;
      case 0x90:
      case 0x09:
        temp = temp + BLUE;
        break;
      case 0xA0:
      case 0x0A:
        temp = temp + GREEN;
        break;
      case 0xB0:
      case 0x0B:
        temp = temp + CYAN;
        break;
      case 0xC0:
      case 0x0C:
        temp = temp + RED;
        break;
      case 0xD0:
      case 0x0D:
        temp = temp + MAGENTA;
        break;
      case 0xE0:
      case 0x0E:
        temp = temp + YELLOW;
        break;
      case 0xF0:
      case 0x0F:
        temp = temp + WHITE;
        break;
      default:
        break;
      }

    p[i] = temp.color;
    if (p[i] == 0)
      p[i] = SDL_MapRGBA(mappingFormat, 0xFF, 0xFF, 0xFF, 0x00);
    for (auto &c : alphaColors) {
      if (p[i] == c.color) {
        p[i] = SDL_MapRGBA(mappingFormat, 0xFF, 0xFF, 0xFF, 0x00);
        break;
      }
    }
  }
  s->surface = SDL_CreateRGBSurfaceFrom((void *)p, width, height, 32,
                                        width * sizeof(Uint32), 0x000000ff,
                                        0x0000ff00, 0x00ff0000, 0xff000000);
  SDL_SetTextureBlendMode(s->texture, SDL_BLENDMODE_BLEND);
  SDL_UpdateTexture(s->texture, NULL, p, width * sizeof(Uint32));
  delete[] pixels;
  delete[] p;
}