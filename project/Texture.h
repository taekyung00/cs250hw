// Texture.h
// -- bitmaps and texture coordinates (cs250 version)
// cs250 11/16

#ifndef CS250_TEXTURE_H
#define CS250_TEXTURE_H


#include "Affine.h"


class Texture {
  public:
    explicit Texture(unsigned W=64, unsigned H=64);
    Texture(const char *bitmap);
    ~Texture(void);
    Vector uvToRGB(float u, float v);
  private:
    enum { R=0, G=1, B=2 };
    unsigned char *data;
    int width, height, stride;
    Texture& operator=(const Texture&);
    Texture(const Texture&);
};


#endif

