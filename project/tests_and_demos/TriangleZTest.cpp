// TriangleZTest.cpp
// -- triangle/z-buffer test
// cs250 10/17
//
// To compile using Visual Studio command prompt:
//     cl /EHsc TriangleZTest.cpp Affine.lib CubeMesh.lib Camera.lib Projection.lib
//        RasterUtilites.cpp opengl32.lib SDL2.lib SDL2main.lib /link /subsystem:console
// To compile using Linux command line:
//     g++ TriangleZTest.cpp Affine.cpp CubeMesh.cpp Camera.cpp Projection.cpp
//         RasterUtilites.cpp -lGL -lSDL2

#include <vector>
#include <cstdlib>
#include <cmath>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "CubeMesh.h"
#include "RasterUtilities.h"
#include "Projection.h"
using namespace std;


const float PI = 4.0f*atan(1.0f);
const Point O(0,0,0);
const Vector EY(0,1,0),
             EZ(0,0,1);


class Client {
  public:
    Client(int W, int H);
    ~Client(void);
    void draw(float dt);
  private:
    int width, height;
    Camera cam;
    Affine ndc2device;
    CubeMesh cube;
    vector<Point> temp_world_verts;
    vector<Hcoord> temp_clip_verts;
    char *block;
    Raster *raster;
    Raster::byte *frame_buffer;
    float time;
};


Client::Client(int W, int H)
    : width(W), height(H) {
  float aspect = float(W)/float(H);
  cam = Camera(Point(0,0,1),Vector(0,0,-1),Vector(0,1,0),0.5f*PI,aspect,0.1f,10);
  ndc2device = translate(Vector(-0.5f,-0.5f,0))
               * scale(0.5f*width,0.5f*height,1)
               * translate(Vector(1,1,0));

  int stride = 3*width + (4-((3*width)%4))%4,
      frame_buffer_sz = stride*height*sizeof(Raster::byte),
      zbuffer_sz = width*height*sizeof(float);
  block = new char[frame_buffer_sz+zbuffer_sz];
  frame_buffer = reinterpret_cast<Raster::byte*>(block);
  raster = new Raster(frame_buffer,
                      reinterpret_cast<float*>(block+frame_buffer_sz),
                      width,height,stride);
  time = 0;
}


Client::~Client(void) {
  delete raster;
  delete[] block;
}


void Client::draw(float dt) {
  // clear screen and z-buffer
  raster->SetColor(255,255,255);
  ClearBuffers(*raster);

  // compute modeling transformations for each cube
  Affine obj2world[3];
  Vector u(0,1,1);
  obj2world[0] = translate(Point(-1,0,-3)-O)
                 * rotate(acos(u.z/abs(u)),cross(EZ,u))
                 * rotate(2*PI*time/10.0f,EZ)
                 * scale(0.5f*2.5,0.5f*2.5,0.5f*0.15f);
  u = Vector(0,1,-1);
  obj2world[1] = translate(Point(1,0,-3)-O)
                 * rotate(acos(u.z/abs(u)),cross(EZ,u))
                 * rotate(2*PI*time/8.0f,EZ)
                 * scale(0.5f*2.5,0.5f*2.5,0.5f*0.15f);
  obj2world[2] = translate(Point(0,0,2*sin(2*PI*time/15.0f)-3)-O)
                 * rotate(2*PI*time/7.1f,EY)
                 * scale(0.5f*2,0.5f*2,0.5f*0.1f);

  // draw cubes
  const Vector obj_color[3]
      = { Vector(255,100,100), Vector(100,100,255), Vector(0,255,0) };
  for (int n=0; n < 3; ++n) {
    Matrix obj2device = ndc2device
                        * CameraToNDC(cam)
                        * WorldToCamera(cam)
                        * obj2world[n];
    temp_world_verts.clear();
    temp_clip_verts.clear();
    for (int i=0; i < cube.VertexCount(); ++i) {
      temp_world_verts.push_back(obj2world[n]*cube.GetVertex(i));
      temp_clip_verts.push_back(obj2device*cube.GetVertex(i));
    }
    for (int i=0; i < cube.FaceCount(); ++i) {
      Mesh::Face f = cube.GetFace(i);
      const Point& Pworld = temp_world_verts[f.index1],
                   Qworld = temp_world_verts[f.index2],
                   Rworld = temp_world_verts[f.index3];
      Vector normal = cross(Qworld-Pworld,Rworld-Pworld),
             light(0,0,1);
      float mu = abs(dot(normal,light))/(abs(normal)*abs(light));
      raster->SetColor(Raster::byte(mu*obj_color[n].x),
                       Raster::byte(mu*obj_color[n].y),
                       Raster::byte(mu*obj_color[n].z));
      const Hcoord& Pclip = temp_clip_verts[f.index1],
                    Qclip = temp_clip_verts[f.index2],
                    Rclip = temp_clip_verts[f.index3];
      FillTriangle(*raster,Pclip,Qclip,Rclip);
    }
  }

  glDrawPixels(width,height,GL_RGB,GL_UNSIGNED_BYTE,frame_buffer);
  time += dt;
}


/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

int main(int argc, char * argv[]) {

  // SDL: initialize and create a window
  SDL_Init(SDL_INIT_VIDEO);
  const char *title = "CS 250: Triangle Z-buffer Test";
  int size = (argc > 1) ? atoi(argv[1]) : 600;
  int width = size,
      height = size;
  SDL_Window *window = SDL_CreateWindow(title,SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,width,height,
                                        SDL_WINDOW_OPENGL);
  SDL_GLContext context = SDL_GL_CreateContext(window);

  // animation loop
  bool done = false;
  Client *client = new Client(width,height);
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

