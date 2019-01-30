#ifndef TEXTURE_H
#define TEXTURE_H
#include <SDL2/SDL.h>
#include <fstream>
#include <iostream>
#include <vector>
class Texture {
private:
public:
  int width;
  int height;
  std::vector<std::vector<uint32_t>> texture;
  Texture(std::vector<std::vector<uint32_t>> _texture)
      :  width(_texture[0].size()), height(_texture.size()),texture(_texture) {}
  Texture(std::string filename) {
    std::ifstream file(filename);
    int w, h;
    file >> w >> h;
    if (!file.is_open())
      throw std::out_of_range("Filename " + filename + " not found");

    std::vector<std::vector<uint32_t>> text(h, std::vector<uint32_t>(w, 0));
    for (int i = 0; i < h; ++i) {
      for (int j = 0; j < w; ++j) {
        file >> text[i][j];
      }
    }
    texture = text;
    height = h;
    width = w;
  }

  Texture():width(0), height(0) {

  }

  uint32_t sampleTexture(double x, double y) {
    while (x > 1)
      x = x - (int)x;
    while (y > 1)
      y = y - (int)y;

    int xPos = (int)round(x * (double)(width - 1.0f));
    int yPos = (int)round(y * (double)(height - 1.0f));
    return texture[yPos][xPos];
  }

  int getWidth() { return width; }
  int getHeight() { return height; }
};
#endif // TEXTURE_H