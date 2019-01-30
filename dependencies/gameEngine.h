#ifndef WINDOW_H
#define WINDOW_H
#include "SDL2/SDL.h"
#include "src/texture.h"
#include "src/values.h"
#include "src/vec3d.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <time.h>
#include <vector>

struct Sprite {
  SDL_Texture *texture = nullptr;
  SDL_Surface* surface = nullptr;
  SDL_Rect position;
  SDL_Rect sheetPos = {-1, -1, -1, -1};
  // ~Sprite() { free(); }
  void free() {
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    texture = nullptr;
    surface = nullptr;
  }
};

struct Text {
  std::string message = "";
  int x = 0;
  int y = 0;
  SDL_Color col;
  int fontSize = 24;
  int w = -1;
  int h = -1;
};

class Window {
  public:
  bool bUsePixels=true;
private:
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;
  SDL_Texture *texture;
  SDL_Texture *imgTexture;
  std::string appName = "";
  Uint32 *pixels;
  std::vector<Sprite> sprites;
  std::vector<Text> textBlocks;
  int width;
  int height;
  double timeElapsed;
  bool done = false;

public:
  Window(int _width, int _height, std::string _appName = "");
  ~Window();

  int init(int width, int height);

  void Cleanup();

  void sdlPixelDrawing();

  void sdlPixelDrawing(std::vector<int> pix);

  int getWidth();
  int ScreenWidth();

  int getHeight();
  int ScreenHeight();
  /* Drawing */
  virtual void draw(const int &x, const int &y, const Pixel &color = WHITE);

  void drawLine(int x, int y, int endX, int endY, const Pixel &color = WHITE);

  void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3,
                    const Pixel &color = WHITE);
  /* Filling */
  void fillAll(const Pixel &color = BLACK);

  void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3,
                    const Pixel &color = WHITE);

  /* Loop */
  void Start();
  virtual bool onUpdate(double timeElapsed);

  virtual bool onStart();

  void events(double timeElapsed);

  void handleQuit(SDL_Event &event);
  virtual void handleEvents(SDL_Event &event, double timeElapsed);

  void fillTexturedTriangle(int x1, int y1, double u1, double v1, int x2,
                            int y2, double u2, double v2, int x3, int y3,
                            double u3, double v3, Texture &tex);

  bool isGreaterLessThan(double a, double b, bool checkGeq);

  void fill(int x, int y, int endX, int endY, const Pixel &color = WHITE);

  void addSpriteSheet(std::string filename);

  void removeSpriteSheet();

  void makeSprite(std::string filename, Sprite *s, int bgR = 0, int bgG = 0,
                  int bgB = 0);

  void makeSprite(Uint32* pixels, int width, int height, Sprite*s);
  
  void makeSpriteFromSprFile(std::string filename, Sprite *s);

  void makeSpriteFromSprFile(std::string filename, Sprite *s, std::vector<Pixel> alphaColors);

  void drawSprite(Sprite *s, int x, int y, int endX, int endY);

  void drawPartialSprite(Sprite *s, int x, int y, int endX, int endY,
                         int sheetX, int sheetY, int sheetEndX, int sheetEndY);

  SDL_Rect makeSDL_Rect(int x, int y, int endX, int endY);

  void drawWireFrameModel(
      const std::vector<std::pair<double, double>> &vecModelCoordinates,
      double x, double y, double r = 0.0f, double s = 1.0f,
      const Pixel &color = WHITE);

  void drawText(int x, int y, std::string text, int endX = -1, int endY = -1,
                int fontSize = 24, const Pixel &color = WHITE);

  static std::string pixelToString(const Pixel &p);

  void fillTexturedTriangle(int x1, int y1, double u1, double v1, double w1,
                            int x2, int y2, double u2, double v2, double w2,
                            int x3, int y3, double u3, double v3, double w3,
                            Texture &tex, double *depthBuffer = nullptr);
  Texture getTextureFromImage(std::string filename, int bgR = 0,
                                      int bgG = 0, int bgB = 0);

void projectTexture(const Texture& src, Texture &dst, int sx, int sy, int ex, int ey);

private:
  void fillTopUpTriangle(int x1, int y1, int x2, int y2, int x3, int y3,
                         const Pixel &color = WHITE);
  void fillTopDownTriangle(int x1, int y1, int x2, int y2, int x3, int y3,
                           const Pixel &color = WHITE);

  void fillFlatTriangle(int x1, int y1, int x2, int y2, int x3, int y3,
                        bool isTopUp, const Pixel &color = WHITE);
  /* Utils */
  void drawHorisontalLine(int y, int x, int endX, const Pixel &color = WHITE);

  void drawVerticalLine(int x, int y, int endY, const Pixel &color = WHITE);

  void minMax(int &x, int &y);

  void minMax(double &x, double &y);

  std::vector<vec3d> getSortedPoints(int x1, int y1, int x2, int y2, int x3,
                                     int y3, bool sortOnY = true);
  void drawText();

    
};

#endif // WINDOW_H
