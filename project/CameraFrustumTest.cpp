// CameraFrustumTest.cpp
// -- display the camera (uses Camera and FrustumMesh)
// cs250 5/17
//
// To compile/link using Visual Studio command prompt:
//     cl /EHsc CameraFrustumTest.cpp Affine.lib Render.lib SimpleRender.lib
//        Camera.cpp FrustumMesh.cpp opengl32.lib glew32.lib SDL2.lib
//        SDL2main.lib /link /subsystem:console
// Under Linux:
//     g++ CameraFrustumTest.cpp Affine.cpp Render.cpp SimpleRender.cpp\
//         Camera.cpp FrustumMesh.cpp -lGL -lGLEW -lSDL2

#include <iostream>
#include <algorithm>
#include <cmath>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "Render.h"
#include "SimpleRender.h"
#include "Camera.h"
#include "FrustumMesh.h"
using namespace std;


const float PI = 4.0f*atan(1.0f);
const Point O(0,0,0);
const Vector EX(1,0,0),
             EY(0,1,0),
             EZ(0,0,1);
const Vector BLUE(0,0,1),
             GREEN(0,1,0),
             GRAY(0.8f,0.8f,0.8f);


class Client {
  public:
    Client(void);
    ~Client(void);
    void draw(float dt);
    void keypress(SDL_Keycode kc);
    void resize(int W, int H);
  private:
    Camera camera;
    FrustumMesh frustum;
    bool draw_solid;
    Render *render;
    SimpleRender *srender;
};


Client::Client(void)
    :  frustum(0.4f*PI,1.5f,1,3) {
  render = new Render();
  srender = new SimpleRender(*render);
  camera = Camera(O-10*EZ,-EZ,EY,0.4f*PI,1.5f,1,3);
  draw_solid = false;
}


Client::~Client(void) {
  delete srender;
  delete render;
}


void Client::draw(float /*dt*/) {
  render->ClearBuffers(GRAY);

  // Draw view frustum
  Affine camera_to_world(camera.Right(),camera.Up(),camera.Back(),camera.Eye());
  if (draw_solid)
    srender->DisplayFaces(frustum,camera_to_world,GREEN);
  else
    srender->DisplayEdges(frustum,camera_to_world,BLUE);
}


void Client::keypress(SDL_Keycode kc) {
  const float angle_increment = PI/180.0f,
              dist_increment = 0.1f,
              zoom_increment = 0.95f;
  switch (kc) {
    case SDLK_SPACE:
      draw_solid = !draw_solid;
      break;
    case SDLK_w:
      camera.Pitch(angle_increment);
      break;
    case SDLK_x:
      camera.Pitch(-angle_increment);
      break;
    case SDLK_a:
      camera.Yaw(angle_increment);
      break;
    case SDLK_d:
      camera.Yaw(-angle_increment);
      break;
    case SDLK_s:
      camera.Roll(angle_increment);
      break;
    case SDLK_j:
      camera.Roll(-angle_increment);
      break;
    case SDLK_u:
      camera.Forward(dist_increment);
      break;
    case SDLK_m:
      camera.Forward(-dist_increment);
      break;
    case SDLK_1:
      camera.Zoom(zoom_increment);
      break;
    case SDLK_2:
      camera.Zoom(1.0f/zoom_increment);
      break;
  }
  if (kc == SDLK_1 || kc == SDLK_2) {
    float fov = 2*atan(0.5f*camera.ViewportGeometry().x
                       /camera.ViewportGeometry().z),
          aspect = camera.ViewportGeometry().x/camera.ViewportGeometry().y;
    frustum = FrustumMesh(fov,aspect,1,3);
  }
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
  const char *title = "CS 250: Camera+Frustum Test";
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

