// CameraRender2DTest.cpp
// -- rendering a scene from a camera's point of view,
//    no hidden surface removal.
// cs200 9/17
//
// Visual Studio command prompt:
//     cl /EHsc CameraRender2DTest.cpp Affine.lib Camera.lib FrustumMesh.lib
//        Projection.cpp CubeMesh.lib SnubDodecMesh.cpp Render.lib
//        CameraRender2D.cpp opengl32.lib glew32.lib SLD2.lib SDL2main.lib
//        /link /subsystem:console
//
// Linux command line:
//     g++ CameraRender2DTest.cpp Affine.cpp Camera.cpp FrustumMesh.cpp
//         CubeMesh.cpp SnubDodecMesh.cpp Render.cpp Projection.cpp
//         CameraRender2D.cpp -lGL -lGLEW -lSDL2

#include <iostream>
#include <algorithm>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "Projection.h"
#include "CameraRender2D.h"
#include "FrustumMesh.h"
#include "CubeMesh.h"
#include "SnubDodecMesh.h"
using namespace std;


const Point O(0,0,0);
const Vector EX(1,0,0),
             EY(0,1,0),
             EZ(0,0,1);
const float PI = 4.0f*atan(1.0f);
const Vector WHITE(1,1,1),
             PURPLE(1,0,1),
             BLACK(0,0,0),
             RED(1,0,0),
             GREEN(0,1,0);


class Client {
  public:
    Client(void);
    ~Client(void);
    void draw(float dt);
    void keypress(SDL_Keycode kc);
    void resize(int W, int H);
  private:
    Camera cam1, cam2,
           cam2_0;
    SnubDodecMesh snub_mesh;
    CubeMesh cube_mesh;
    Affine cube2world[9],
           snub2world;
    float snub_rot_rate;
    Vector snub_rot_axis;
    Point snub_center;
    bool use_cam1,
         draw_solid;
    Render *render;
    CameraRender2D *crender;
};


Client::Client(void) {
  render = new Render();
  crender = new CameraRender2D(*render);

  snub_rot_rate = 2*PI/10.0f;
  snub_rot_axis = EY;
  snub_center = Point(0,0,-2);
  use_cam1 = true;
  draw_solid = false;

  cam1 = Camera(O+EZ,-EZ,EY,0.5f*PI,1,0.01f,10);
  cam2_0 = Camera(O+2*EX-2*EZ,-EX,EY,0.5f*PI,1,0.01f,10);
  cam2 = cam2_0;
  snub2world = translate(snub_center-O);
  for (int i=0; i < 9; ++i)
    cube2world[i] = translate(Vector(float((i%3)-1),-1.1f,float(-(i/3)-1)))
                    * scale(0.375f,0.05f,0.375f);

  glDisable(GL_DEPTH_TEST);
}


Client::~Client(void) {
  delete crender;
  delete render;
}


void Client::draw(float dt) {
  render->ClearBuffers(WHITE);

  Camera& cam = use_cam1 ? cam1 : cam2;
  crender->SetCamera(cam);

  // Draw cubes
  for (int i=0; i < 9; ++i)
    if (draw_solid)
      crender->DisplayFaces(cube_mesh,cube2world[i],PURPLE);
    else
      crender->DisplayEdges(cube_mesh,cube2world[i],PURPLE);

  // Draw snub dodecahedron
  snub2world = translate(snub_center-O)
               * rotate(snub_rot_rate*dt,snub_rot_axis)
               * translate(O-snub_center)
               * snub2world;
  if (draw_solid)
    crender->DisplayFaces(snub_mesh,snub2world,GREEN);
  else
    crender->DisplayEdges(snub_mesh,snub2world,GREEN);

  // Draw (other) camera mesh
  if (use_cam1) {
    const float& D = cam2.ViewportGeometry().z,
                 W = cam2.ViewportGeometry().x,
                 H = cam2.ViewportGeometry().y,
                 N = cam2.NearDistance(),
                 F = 1; // for visual appeal
    float fov = 2*atan(0.5f*W/D);
    FrustumMesh frustum(fov,W/H,N,F);
    crender->DisplayEdges(frustum,CameraToWorld(cam2),BLACK);
  }
  else {
    const float& D = cam1.ViewportGeometry().z,
                 W = cam1.ViewportGeometry().x,
                 H = cam1.ViewportGeometry().y,
                 N = cam1.NearDistance(),
                 F = 1;
    float fov = 2*atan(0.5f*W/D);
    FrustumMesh frustum(fov,W/H,N,F);
    crender->DisplayEdges(frustum,CameraToWorld(cam1),RED);
  }
}


void Client::keypress(SDL_Keycode kc) {
  // respond to keyboard input
  //   kc: SDL keycode (e.g., SDLK_SPACE, SDLK_a, SDLK_s)
  const float angle_increment = PI/180.0f,
              dist_increment = 0.1f,
              zoom_increment = 0.95f;
  switch (kc) {
    case SDLK_w:
      cam2.Pitch(angle_increment);
      break;
    case SDLK_x:
      cam2.Pitch(-angle_increment);
      break;
    case SDLK_a:
      cam2.Yaw(angle_increment);
      break;
    case SDLK_d:
      cam2.Yaw(-angle_increment);
      break;
    case SDLK_s:
      cam2.Roll(angle_increment);
      break;
    case SDLK_j:
      cam2.Roll(-angle_increment);
      break;
    case SDLK_u:
      cam2.Forward(dist_increment);
      break;
    case SDLK_m:
      cam2.Forward(-dist_increment);
      break;
    case SDLK_SPACE:
      draw_solid = !draw_solid;
      break;
    case SDLK_1:
      cam2.Zoom(zoom_increment);
      break;
    case SDLK_2:
      cam2.Zoom(1.0f/zoom_increment);
      break;
    case SDLK_3:
      use_cam1 = !use_cam1;
      break;
    case SDLK_4:
      cam2 = cam2_0;
  }
}


void Client::resize(int W, int H) {
  glViewport(0,0,W,H);
  float aspect = float(W)/float(H);
  cam1 = Camera(cam1.Eye(),-cam1.Back(),EY,0.5f*PI,aspect,0.01f,10);
  cam2_0 = Camera(cam2.Eye(),-cam2.Back(),EY,0.5f*PI,aspect,0.01f,10);
  cam2 = cam2_0;
}


/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

int main(int /*argc*/, char * /*argv*/[]) {

  // SDL: initialize and create a window
  SDL_Init(SDL_INIT_VIDEO);
  const char *title = "CS 250: CameraRender2D Test";
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
    bool done = false;
    Client *client = new Client();
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

