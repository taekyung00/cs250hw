// Raster.h
// -- elementary raster graphics with z-buffer
// cs250 3/17

#ifndef CS250_RASTER_H
#define CS250_RASTER_H

#include <cassert>
#include <cstring>


class Raster {
  public:
    typedef unsigned char byte;
    Raster(byte *rgb_data, float *zbuffer, int width, int height, int stride);
    void GotoPoint(int x, int y);
    void WriteZ(float z);
    float GetZ(void);
    void SetColor(byte r, byte g, byte b);
    void WritePixel(void);
    void IncrementX(void);
    void DecrementX(void);
    void IncrementY(void);
    void DecrementY(void);
    int Width(void) const;
    int Height(void) const;
    int Stride(void) const;
  private:
    byte *buffer;
    float *zbuffer;
    int width, height, stride;
    int current_index, current_x, current_y;
    int current_indexz;
    byte red, green, blue;
};


///////////////////////////////////////////////////////////////////////////////
// inline implementations
///////////////////////////////////////////////////////////////////////////////

#ifndef NDEBUG
#define USE_CURRENT
#endif

#ifdef CLIP_PIXELS
#define USE_CURRENT
#endif


inline
Raster::Raster(byte *d, float *zb, int w, int h, int s)
    : buffer(d), zbuffer(zb), width(w), height(h), stride(s),
      red(0), green(0), blue(0) {
}


inline
void Raster::GotoPoint(int x, int y) {
  #ifdef USE_CURRENT
  current_x = x;
  current_y = y;
  #endif
  current_index = y*stride + 3*x;
  current_indexz = y*width + x;
}


inline
void Raster::WriteZ(float z) {
  #ifdef CLIP_PIXELS
  if (current_x < 0 || width <= current_x
      || current_y < 0 || height <= current_y)
    return;
  #endif
  assert(0 <= current_x && current_x < width
         && 0 <= current_y && current_y < height);
  zbuffer[current_indexz] = z;
}


inline
float Raster::GetZ(void) {
  #ifdef CLIP_PIXELS
  if (current_x < 0 || width <= current_x
      || current_y < 0 || height <= current_y)
    return -1;
  #endif
  assert(0 <= current_x && current_x < width
         && 0 <= current_y && current_y < height);
  return zbuffer[current_indexz];
}


inline
void Raster::SetColor(byte r, byte g, byte b) {
  red = r;
  green = g;
  blue = b;
}


inline
void Raster::WritePixel(void) {
  #ifdef CLIP_PIXELS
  if (current_x < 0 || width <= current_x
      || current_y < 0 || height <= current_y)
    return;
  #endif
  assert(0 <= current_x && current_x < width
         && 0 <= current_y && current_y < height);
  int index = current_index;
  buffer[index] = red;
  buffer[++index] = green;
  buffer[++index] = blue;
}


inline
void Raster::IncrementX(void) {
  #ifdef USE_CURRENT
  ++current_x;
  #endif
  current_index += 3;
  ++current_indexz;
}


inline
void Raster::DecrementX(void) {
  #ifdef USE_CURRENT
  --current_x;
  #endif
  current_index -= 3;
  --current_indexz;
}


inline
void Raster::IncrementY(void) {
  #ifdef USE_CURRENT
  ++current_y;
  #endif
  current_index += stride;
  current_indexz += width;
}


inline
void Raster::DecrementY(void) {
  #ifdef USE_CURRENT
  --current_y;
  #endif
  current_index -= stride;
  current_indexz -= width;
}


inline
int Raster::Width(void) const {
  return width;
}


inline
int Raster::Height(void) const {
  return height;
}


inline
int Raster::Stride(void) const {
  return stride;
}


#endif

