// Texture.cpp
// jsh 11/16

#include <iostream>

#include <fstream>
#include <cmath>
#include <stdexcept>
#include "Texture.h"
using namespace std;


Texture::Texture(unsigned W, unsigned H)
    : data(0),
      width(W),
      height(H),
      stride(3*W) {
  unsigned pad = 4 - (3*W)%4;
  stride += (pad != 4) ? pad : 0;

  data = new unsigned char[stride*height];

  float iW = 1.0f/W,
        iH = 1.0f/H;
  for (int j=0; j < height; ++j) {
    for (int i=0; i < width; ++i) {
      float u = iW*(i+0.5f),
            v = iH*(j+0.5f),
            rr = pow(u-0.5f,2) + pow(v-0.5f,2);
      int index = j*stride + 3*i;
      if (rr > 0.16f) {
        data[index+R] = (unsigned char)(100 + 155*u);
        data[index+G] = 0;
        data[index+B] = (unsigned char)(100 + 155*v);
      }
      else if (rr < 0.09f) {
        data[index+R] = (unsigned char)(100 + 155*v);
        data[index+G] = 0;
        data[index+B] = (unsigned char)(100 + 155*u);
      }
      else {
        data[index+R] = 0;
        data[index+G] = 255;
        data[index+B] = 0;
      }
    }
  }
}


Texture::Texture(const char *bitmap)
    : data(0) {

  // read in raw data
  fstream in(bitmap,ios_base::in|ios_base::binary);
  char header[38];
  in.read(header,38);
  unsigned offset = *reinterpret_cast<unsigned*>(header+10),
           size = *reinterpret_cast<unsigned*>(header+34);
  width = *reinterpret_cast<int*>(header+18);
  height = *reinterpret_cast<int*>(header+22);

  if (size == 0) {
    unsigned span = 3*width;
    span += (span%4 == 0) ? 0 : 4-span%4;
    size = height*span;
  }

  in.seekg(offset,ios_base::beg);
  data = new unsigned char[size];
  in.read(reinterpret_cast<char*>(data),size);
  if (!in) {
    delete[] data;
    throw runtime_error("failed to read bitmap data");
  }

  // cook data, bottom-up case
  if (height > 0) {
    stride = size/height;
    for (int j=0; j < height; ++j) {
      for (int i=0; i < width; ++i) {
        int index = j*stride + 3*i;
        unsigned char temp = data[index+R];
        data[index+R] = data[index+B];
        data[index+B] = temp;
      }
    }
  }

  // top-down case
  else {
    height = -height;
    stride = size/height;
    int half_height = height/2;
    for (int j=0; j < half_height; ++j) {
      for (int i=0; i < width; ++i) {
        int index_src = (height-j-1)*stride + 3*i,
            index_dst = j*stride + 3*i;
        unsigned char temp[3] = { data[index_src+R],
                                  data[index_src+G],
                                  data[index_src+B] };
        data[index_src+R] = data[index_dst+B];
        data[index_src+G] = data[index_dst+G];
        data[index_src+B] = data[index_dst+R];
        data[index_dst+R] = temp[B];
        data[index_dst+G] = temp[G];
        data[index_dst+B] = temp[R];
      }
    }
    if (height%2 == 1) {
      for (int i=0; i < width; ++i) {
        int index = half_height*stride + 3*i;
        unsigned char temp = data[index+R];
        data[index+R] = data[index+B];
        data[index+B] = temp;
      }
    }
  }

}


Texture::~Texture(void) {
  delete[] data;
}


Vector Texture::uvToRGB(float u, float v) {
  u -= floor(u);
  v -= floor(v);
  int x = int(width*u),
      y = int(height*v),
      index = y*stride + 3*x;
  return Vector(data[index+R],data[index+G],data[index+B]);
}

