#ifndef PIXEL_H
#define PIXEL_H
#include <stdint.h>

class Pixel {
private:
  uint32_t getColor() {
    return (uint32_t)(alpha << 24) + (uint32_t)(blue << 16) + (uint32_t)(green << 8) + (uint32_t)red;
  }

public:
  uint8_t red, green, blue, alpha;
  uint32_t color;
  Pixel(uint8_t _red, uint8_t _green, uint8_t _blue, uint8_t _alpha = 255)
      : red(_red), green(_green), blue(_blue), alpha(_alpha) {
    color = getColor();
  }
  Pixel(uint32_t _color) : color(_color) {
    alpha = color & (uint32_t)(255 << 24);
    blue = color & (uint32_t)(255 << 16);
    green = color & (uint32_t)(255 << 8);
    red = color & (uint32_t)(255 << 0);
  }
  Pixel (uint8_t col): red(col), green(col), blue(col){
    color=getColor();
  }
  Pixel(){
    red = 0;
    blue = 0;
    green = 0;
    alpha = 255;
    color = getColor();
  }
  bool operator==(Pixel &other){return other.color == color;}
  bool operator==(const Pixel &other) const{return other.color == color;}
  Pixel operator+(const Pixel& other) const{
    if (other.color == (255<<24)) return *this;
    if (color == (255<<24)) return other;
    Pixel p;
    uint8_t r = (uint8_t)(((int)red + (int)other.red)/2);
    uint8_t  b = (uint8_t)(((int)blue + (int)other.blue)/2);
    uint8_t g = (uint8_t)(((int)green + (int)other.green)/2);
    p.red = r;
    p.green = g;
    p.blue = b;
    p.color = p.getColor();
    return p;
    // return Pixel (red, green, blue);
  }

  friend std::ostream& operator<< (std::ostream& stream, const Pixel& color){
    stream << "red: " << (int)color.red << ", green: " << (int)color.green << ", blue: "<<(int)color.blue;
    return stream;
  }
};

#endif // PIXEL_H