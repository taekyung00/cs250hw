// SimpleRenderTest.cpp
// -- test of simple rendering class
// cs200 5/19
//
// Visual Studio command prompt:
//   cl /EHsc SimpleRenderTest.cpp SimpleRender.cpp SnubDodecMesh.cpp
//      Render.lib Affine.lib opengl32.lib glew32.lib SDL2.lib
//      SDL2main.lib /link /subsystem:console
// Linux command line:
//   g++ SimpleRenderTest.cpp SimpleRender.cpp Render.cpp Affine.cpp\
//       SnubDodecMesh.cpp -lGL -lGLEW -lSDL2

#include <iostream>
#include <algorithm>
#include <cmath>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "Render.h"
#include "SimpleRender.h"
#include "SnubDodecMesh.h"
using namespace std;


const Point O(0,0,0);
const Vector EZ(0,0,1);
const float PI = 4.0f * atan(1.0f);
const Vector WHITE(1,1,1),
             GREEN(0,1,0),
             BLUE(0,0,1);


class Client {
  public:
    Client(void);
    ~Client(void);
    void draw(double dt);
    void keypress(SDL_Keycode kc);
    void resize(int W, int H);
  private:
    SnubDodecMesh mesh;
    Point mesh_center;
    float mesh_scale,
          mesh_rot_rate;
    Vector mesh_rot_axis;
    bool draw_solid;
    Render *render;
    SimpleRender *srender;
    double time;
};


Client::Client(void) {
  render = new Render();
  srender = new SimpleRender(*render);
  mesh_center = Point(0,0,-1);
  mesh_scale = 2.0f;
  mesh_rot_rate = 2*PI/10.0f;
  mesh_rot_axis = normalize(Vector(1,1,1));
  draw_solid = false;
  time = 0;
}


Client::~Client(void) {
  delete render;
  delete srender;
}


void Client::draw(double dt) {
  render->ClearBuffers(WHITE);

  // modeling transformation
  Affine obj2world = translate(mesh_center-O)
                     * rotate(acos(mesh_rot_axis.z),cross(EZ,mesh_rot_axis))
                     * rotate(mesh_rot_rate*static_cast<float>(time),EZ)
                     * scale(mesh_scale/mesh.Dimensions().x)
                     * translate(O-mesh.Center());

  // draw figure
  if (draw_solid)
    srender->DisplayFaces(mesh,obj2world,GREEN);
  else
    srender->DisplayEdges(mesh,obj2world,BLUE);

  time += dt;
}


void Client::resize(int W, int H) {
  int D = min(W,H);
  glViewport(0,0,D,D);
}


void Client::keypress(SDL_Keycode kc) {
  if (kc == SDLK_SPACE)
    draw_solid = !draw_solid;
}


/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

int main(int /*argc*/, char * /*argv*/[]) {

  // SDL: initialize and create a window
  SDL_Init(SDL_INIT_VIDEO);
  const char *title = "CS 250: SimpleRender Test";
  int width = 600,
      height = 600;
  SDL_Window *window = SDL_CreateWindow(title,SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,width,height,
                                        SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
  SDL_GLContext context = SDL_GL_CreateContext(window);

  // GLEW: get function bindings (if possible)
  glewInit();
  if (!GLEW_VERSION_2_0) {
    cout << "needs OpenGL version 2.0 or better" << endl;
    return -1;
  }

  // animation loop
  try {
    Client *client = new Client();
    bool done = false;
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
          case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
              client->resize(event.window.data1,event.window.data2);
            break;
        }
      }
      Uint32 ticks = SDL_GetTicks();
      double dt = 0.001*(ticks - ticks_last);
      ticks_last = ticks;
      client->draw(dt);
      SDL_GL_SwapWindow(window);
    }
    delete client;
  }

  catch (exception &e) {
    cout << e.what() << endl;
  }

  SDL_GL_DeleteContext(context);
  SDL_Quit();
  return 0;
}

