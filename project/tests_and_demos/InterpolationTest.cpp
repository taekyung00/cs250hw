// InterpolationTest.cpp
// -- test of clipping and interpolation of textures
// cs250 11/17
//
// packages used:
//   Affine, Camera, Projection, RasterUtilities, HalfSpace,
//   Texture, Interpolate
//
// note:
//   The bitmap image file 'texture.bmp' is used (if present)

#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "Projection.h"
#include "Interpolate.h"
#include "Texture.h"
#include "RasterUtilities.h"
using namespace std;


const float PI = 4.0f*atan(1.0f);
const Point O(0,0,0);
const Vector EX(1,0,0),
             EY(0,1,0),
             EZ(0,0,1);

float frand(float a=0, float b=1) {
  return a + (b-a)*float(rand()/float(RAND_MAX));
}


/////////////////////////////////////////////////////////////////
// texture-compatible cube mesh
/////////////////////////////////////////////////////////////////
Point vertices[] = { Point( 1, 1, 1),  Point( 1, 1,-1),
                     Point( 1,-1, 1),  Point( 1,-1,-1),
                     Point(-1, 1, 1),  Point(-1, 1,-1),
                     Point(-1,-1, 1),  Point(-1,-1,-1) };

struct TexturedFace {
  int index1, index2, index3;
  float u1, v1, u2, v2, u3, v3;
};

TexturedFace faces[] = {
  { 0,3,1, 0,1, 1,0, 1,1 }, { 0,2,3, 0,1, 0,0, 1,0 },
  { 4,5,7, 1,1, 0,1, 0,0 }, { 4,7,6, 1,1, 0,0, 1,0 },
  { 1,3,7, 0,1, 0,0, 1,0 }, { 1,7,5, 0,1, 1,0, 1,1 },
  { 0,1,5, 1,0, 1,1, 0,1 }, { 0,5,4, 1,0, 0,1, 0,0 },
  { 2,6,7, 1,1, 0,1, 0,0 }, { 2,7,3, 1,1, 0,0, 1,0 },
  { 0,4,6, 1,1, 0,1, 0,0 }, { 0,6,2, 1,1, 0,0, 1,0 } };


/////////////////////////////////////////////////////////////////
class Client {
  public:
    Client(SDL_Window *w);
    ~Client(void);
    void draw(float dt);
    void keypress(SDL_Keycode kc);
  private:
    SDL_Window *window;
    Raster *raster;
    Raster::byte *frame_buffer;
    float *zbuffer;

    Point cube1_center, cube2_center;
    Vector cube1_axis, cube2_axis;
    Hcoord cube1_rate, cube2_rate;

    Camera camera;
    Affine ndc_to_dev;
    Matrix world_to_ndc;

    TextureClip tclip;
    vector<Affine> obj_to_world;
    vector<Point> world_verts;
    vector<TexturedCoord> clipped_verts;

    Texture* texture[2];
    float tex_scale[2];
    bool intersect;
    int mode;

    float current_time,
           translate_time;
};


Client::Client(SDL_Window *w)
      : window(w) {
  int width, height;
  SDL_GetWindowSize(window,&width,&height);
  frame_buffer = new Raster::byte[3*width*height];
  zbuffer = new float[width*height];
  raster = new Raster(frame_buffer,zbuffer,width,height,3*width);

  cube1_axis = normalize(Vector(1,1,1));
  cube2_axis = normalize(Vector(-0.5f,1,2));
  cube1_rate = 2*PI*Hcoord(1/frand(10,20),1/frand(10,20),1/frand(10,20),1/frand(10,20));
  cube2_rate = 2*PI*Hcoord(1/frand(10,20),1/frand(10,20),1/frand(10,20),1/frand(10,20));

  float aspect = float(width)/float(height);
  camera = Camera(O+10*EZ,-EZ,EY,0.4f*PI,aspect,1,30);
  ndc_to_dev = translate(Vector(-0.5f,-0.5f,0))
               * scale(0.5f*width,0.5f*height,1)
               * translate(Vector(1,1,0));
  world_to_ndc = CameraToNDC(camera) * WorldToCamera(camera);

  tclip = TextureClip(Polyhedron(true));
  texture[0] = new Texture(512,512);
  texture[1] = texture[0];
  try { texture[1] = new Texture("texture.bmp"); }
  catch (exception &e) { cout << e.what() << endl; }
  TexturedCoord::SetTexture(texture[0]);
  tex_scale[0] = 1.0;
  tex_scale[1] = 1.0;
  intersect = false;
  mode = 0;

  current_time = 0;
  translate_time = 0;
}


Client::~Client(void) {
  if (texture[1] != texture[0])
    delete texture[1];
  delete texture[0];
  delete raster;
  delete[] zbuffer;
  delete[] frame_buffer;
}


void Client::draw(float dt) {
  raster->SetColor(230,230,230);
  ClearBuffers(*raster,1);

  Point cube_center[2];
  cube_center[0] = O + 5*sin(cube1_rate.x*translate_time)*EX
                     + 5*sin(cube1_rate.y*translate_time)*EY
                     + 5*sin(cube1_rate.z*translate_time)*EZ;
  obj_to_world.clear();
  Affine Mo = translate(cube_center[0]-O)
              * rotate(acos(cube1_axis.z),cross(EZ,cube1_axis))
              * rotate(cube1_rate.w*current_time,EZ)
              * scale(2);
  obj_to_world.push_back(Mo);
  cube_center[1] = O + 5*sin(cube2_rate.x*translate_time)*EX
                     + 5*sin(cube2_rate.y*translate_time)*EY
                     + 5*sin(cube2_rate.z*translate_time)*EZ;
  Mo = translate(cube_center[1]-O)
       * rotate(acos(cube2_axis.z),cross(EZ,cube2_axis))
       * rotate(cube2_rate.w*current_time,EZ)
       * scale(2.1f);
  obj_to_world.push_back(Mo);

  TextureClip cube_clip[2];
  if (intersect) {
    for (int n=0; n < 2; ++n) {
      world_verts.clear();
      for (int i=0; i < 8; ++i)
        world_verts.push_back(obj_to_world[n] * vertices[i]);
      Polyhedron poly;
      for (int i=0; i < 12; ++i) {
        const TexturedFace &f = faces[i];
        const Point &P = world_verts[f.index1],
                    &Q = world_verts[f.index2],
                    &R = world_verts[f.index3];
        poly.half_spaces.push_back(HalfSpace(P,Q,R,cube_center[n]));
      }
      cube_clip[n] = TextureClip(poly);
    }
  }

  for (int n=0; n < 2; ++n) {
    TexturedCoord::SetTexture(texture[n]);
    world_verts.clear();
    for (int i=0; i < 8; ++i)
      world_verts.push_back(obj_to_world[n] * vertices[i]);
  
    for (int i=0; i < 12; ++i) {
      const TexturedFace &f = faces[i];
      const Point &Pworld = world_verts[f.index1],
                  &Qworld = world_verts[f.index2],
                  &Rworld = world_verts[f.index3];
      TexturedCoord Ptex(Pworld,tex_scale[n]*f.u1,tex_scale[n]*f.v1),
                    Qtex(Qworld,tex_scale[n]*f.u2,tex_scale[n]*f.v2),
                    Rtex(Rworld,tex_scale[n]*f.u3,tex_scale[n]*f.v3);
      clipped_verts.clear();
      clipped_verts.push_back(Ptex);
      clipped_verts.push_back(Qtex);
      clipped_verts.push_back(Rtex);
      if (cube_clip[(n+1)%2](clipped_verts)) {
        for (unsigned j=0; j < clipped_verts.size(); ++j) {
          const TexturedCoord &Pworldclipped = clipped_verts[j];
          Hcoord Pndc = world_to_ndc * Pworldclipped;
          clipped_verts[j] = TexturedCoord(Pndc,Pworldclipped.u,Pworldclipped.v);
        }
        if (tclip(clipped_verts)) {
          Vector m = cross(Qworld-Pworld,Rworld-Pworld);
          float mu = min(abs(dot(m,camera.Back()))/abs(m)+0.1f,1.0f);
          TexturedCoord::SetColorScale(mu);
          for (unsigned k=0; k < clipped_verts.size(); ++k) {
            const TexturedCoord &Pndc = clipped_verts[k];
            Hcoord Pdev = ndc_to_dev * Pndc;
            clipped_verts[k] = TexturedCoord(Pdev,Pndc.u,Pndc.v);
          }
          for (unsigned k=2; k < clipped_verts.size(); ++k)
            FillTriangle(*raster,clipped_verts[0],clipped_verts[k-1],clipped_verts[k]);
        }
      }
    }
  }

  int width, height;
  SDL_GetWindowSize(window,&width,&height);
  glDrawPixels(width,height,GL_RGB,GL_UNSIGNED_BYTE,frame_buffer);

  current_time += dt;
  if (!intersect)
    translate_time += dt;
}


void Client::keypress(SDL_Keycode kc) {
  if (kc == SDLK_SPACE) {
    mode = (mode + 1)%8;
    tex_scale[0] = (mode%2 == 0) ? 1.0f : 4.0f;
    tex_scale[1] = ((mode&0x2) == 0) ? 1.0f : 4.0f;
    intersect = ((mode&0x4) != 0);
    stringstream ss;
    ss << "CS250 - Interpolation Demo [mode " << mode << "]";
    SDL_SetWindowTitle(window,ss.str().c_str());
  }
}


/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

int main(int /*argc*/, char * /*argv*/[]) {

  // SDL: initialize and create a window
  SDL_Init(SDL_INIT_VIDEO);
  const char *title = "CS250 - Interpolation Demo [press spacebar]";
  int width = 700,
      height = 700;
  SDL_Window *window = SDL_CreateWindow(title,SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,width,height,
                                        SDL_WINDOW_OPENGL);
  SDL_GLContext context = SDL_GL_CreateContext(window);

  // animation loop
  bool done = false;
  Client *client = new Client(window);
  Uint32 ticks_last = SDL_GetTicks();
  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          done = true;
          break;
        case SDL_KEYDOWN:
          if (event.key.keysym.sym == SDLK_ESCAPE)
            done = true;
          else
            client->keypress(event.key.keysym.sym);
          break;
      }
    }
    Uint32 ticks = SDL_GetTicks();
    float dt = 0.001f*(ticks - ticks_last);
    ticks_last = ticks;
    client->draw(dt);
    SDL_GL_SwapWindow(window);
  }

  // clean up
  delete client;
  SDL_GL_DeleteContext(context);
  SDL_Quit();
  return 0;
}

