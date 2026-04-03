// CubeRenderDemo.cpp
// -- multiple cubes, multiple renderers
// cs250 5/17

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "CubeMesh.h"
#include "Render.h"
using namespace std;


const float PI = 4.0f * atan(1.0f);
const Point O(0,0,0);
const Vector EZ(0,0,1);
const Vector GRAY(0.95f,0.95f,0.95f),
             RED(1,0,0),
             BLUE(0,0,1);


class Client {
  public:
    Client(void);
    ~Client(void);
    void draw(float dt);
    void resize(int W, int H);
  private:
    CubeMesh cube;
    float cube1_rot_rate,
          cube2_rot_rate;
    Vector cube1_rot_axis,
           cube2_rot_axis;
    Point cube1_center,
          cube2_center;
    Render *render1,
           *render2;
    Matrix PersProj;
    float time;
    vector<Point> world_verts;
    vector<Hcoord> clip_verts;
};


Client::Client(void) {
  render1 = new Render();
  render2 = new Render();

  PersProj.row[0] = Hcoord(1.2f,0,0,0);
  PersProj.row[1] = Hcoord(0,1.2f,0,0);
  PersProj.row[2] = Hcoord(0,0,-10.1f/9.9f,-2/9.9f);
  PersProj.row[3] = Hcoord(0,0,-1,0);

  cube1_rot_rate = 2*PI/4.1f;
  cube1_rot_axis = normalize(Vector(1,1,1));
  cube1_center = Point(0.5f,0,-2);
  cube2_rot_rate = 2*PI/9.3f;
  cube2_rot_axis = normalize(Vector(2,-1,-1));
  cube2_center = Point(-0.5f,0,-2);

  time = 0;
}


Client::~Client(void) {
  delete render2;
  delete render1;
}


void Client::draw(float dt) {
  render1->ClearBuffers(GRAY);

  Affine M1 =  translate(cube1_center-O)
               * rotate(acos(cube1_rot_axis.z),cross(EZ,cube1_rot_axis))
               * rotate(cube1_rot_rate*time,EZ)
               * scale(0.8f,0.8f,0.05f),
         M2 = translate(cube2_center-O)
              * rotate(acos(cube2_rot_axis.z),cross(EZ,cube2_rot_axis))
               * rotate(cube2_rot_rate*time,EZ)
               * scale(0.8f,0.8f,0.05f);

  Affine M[2] = { M1, M2 };
  Render *render[2] = { render1, render2 };
  Vector color[2] = { RED, BLUE };
  for (int n=0; n < 2; ++n) {
    world_verts.clear();
    clip_verts.clear();
    for (int i=0; i < cube.VertexCount(); ++i) {
      world_verts.push_back(M[n]*cube.GetVertex(i));
      clip_verts.push_back(PersProj*world_verts[i]);
    }

    for (int i=0; i < cube.FaceCount(); ++i) {
      const Mesh::Face &f = cube.GetFace(i);
      const Point &P = world_verts[f.index1],
                  &Q = world_verts[f.index2],
                  &R = world_verts[f.index3];
      Vector m = cross(Q-P,R-P);
      if (dot(m,O-P) > 0) {
        const Vector L(-0.5f,0,2);
        float mu = abs(dot(m,L))/(abs(m)*abs(L));
        render[n]->SetColor(mu*color[n]);
        render[n]->FillTriangle(clip_verts[f.index1],
                                clip_verts[f.index2],
                                clip_verts[f.index3]);
      }
    }
  }

  time += dt;
}


void Client::resize(int W, int H) {
  int D = min(W,H);
  glViewport(0,0,D,D);
}


/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

int main(int /*argc*/, char * /*argv*/[]) {

  // SDL: initialize and create a window
  SDL_Init(SDL_INIT_VIDEO);
  const char *title = "CS 250: Cube+Render Demo";
  int width = 600,
      height = 600;
  SDL_Window *window = SDL_CreateWindow(title,SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,width,height,
                                        SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
  SDL_GLContext context = SDL_GL_CreateContext(window);

  // GLEW: get function bindings (if possible)
  GLenum value = glewInit();
  if (value != GLEW_OK) {
    cout << glewGetErrorString(value) << endl;
    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return -1;
  }

  // animation loop
  try {
    Client *client = new Client();
    Uint32 ticks_last = SDL_GetTicks();
    bool done = false;
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
          case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
              client->resize(event.window.data1,event.window.data2);
            break;
        }
      }
      Uint32 ticks = SDL_GetTicks();
      float dt = 0.001f*(ticks - ticks_last);
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

