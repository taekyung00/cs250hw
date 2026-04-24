// ClipDemo.cpp
// -- graphical test of clipping routine
// cs250 10/17
//
// From the Visual Studio command prompt:
//     cl /EHsc ClipDemo.cpp Affine.cpp Camera.cpp Projection.cpp\
//        CubeMesh.cpp HalfSpace.cpp Clip.cpp

#include <iostream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "Affine.h"
#include "CubeMesh.h"
#include "SnubDodecMesh.h"
#include "Camera.h"
#include "Projection.h"
#include "Clip.h"
using namespace std;


const float PI = 4.0f*atan(1.0f);
const Point O(0,0,0);
const Vector LIGHT(0,0,1),
             EX(1,0,0),
             EY(0,1,0),
             EZ(0,0,1),
             ZERO(0,0,0);


class Client {
  public:
    Client(SDL_Window *window);
    ~Client(void);
    void draw(double dt);
    void keypress(SDL_Keycode kc);
    void resize(int W, int H);
    void DisplayEdges(Mesh &m, const Affine &M, const Vector &c);
    void DisplayClippedFaces(Mesh &m, const Affine &M,
                             Clip &clip, const Vector &c);
  private:
    CubeMesh cube;
    SnubDodecMesh other;
    int mode;
    Vector offset1, offset2;
    Point cube_center;
    Vector cube1_axis, cube2_axis;
    float cube1_rate, cube2_rate;
    Camera cam;
    Matrix world_to_ndc;
    vector<Point> world_verts;
    vector<Hcoord> clip_verts;
    double time;
    SDL_Window *window;
};


Client::Client(SDL_Window *w) {
  window = w;
  time = 0;
  cube_center = Point(0,0,-2);
  cube1_axis = normalize(Vector(1,1,1));
  cube2_axis = normalize(Vector(-0.5f,1,2));
  cube1_rate = 2*PI/20.0f;
  cube2_rate = 2*PI/9.0f;
  cam = Camera(O,-EZ,EY,0.4f*PI,1,0.1f,10);
  world_to_ndc = CameraToNDC(cam) * WorldToCamera(cam);
  mode = 0;
  offset1 =  0.2f*EX + 0.1f*EY;
  offset2 = -0.1f*EX - 0.2f*EY;
  glEnable(GL_DEPTH_TEST);
}


void Client::DisplayEdges(Mesh& mesh, const Affine& obj_to_world,
                          const Vector& clr) {
  Matrix obj_to_ndc = world_to_ndc * obj_to_world;
  clip_verts.clear();
  for (int i=0; i < mesh.VertexCount(); ++i)
    clip_verts.push_back(obj_to_ndc * mesh.GetVertex(i));
  glColor3f(clr.x,clr.y,clr.z);
  for (int i=0; i < mesh.EdgeCount(); ++i) {
    Mesh::Edge e = mesh.GetEdge(i);
    Hcoord &P = clip_verts[e.index1],
           &Q = clip_verts[e.index2];
    glBegin(GL_LINES);
      glVertex4f(P.x,P.y,P.z,P.w);
      glVertex4f(Q.x,Q.y,Q.z,Q.w);
    glEnd();
  }
}


void Client::DisplayClippedFaces(Mesh& mesh, const Affine& obj_to_world,
                                 Clip& clip, const Vector& clr) {
  world_verts.clear();
  for (int i=0; i < mesh.VertexCount(); ++i)
    world_verts.push_back(obj_to_world*mesh.GetVertex(i));
  for (int i=0; i < mesh.FaceCount(); ++i) {
    Mesh::Face f = mesh.GetFace(i);
    const Point& wP = world_verts[f.index1],
                 wQ = world_verts[f.index2],
                 wR = world_verts[f.index3];
    clip_verts.clear();
    clip_verts.push_back(wP);
    clip_verts.push_back(wQ);
    clip_verts.push_back(wR);
    if (clip(clip_verts)) {
      Vector n = cross(wQ-wP,wR-wP);
      float mu = abs(dot(n,LIGHT))/abs(n);
      glColor3f(mu*clr[0],mu*clr[1],mu*clr[2]);
      for (unsigned j=0; j < clip_verts.size(); ++j)
        clip_verts[j] = world_to_ndc * clip_verts[j];
      glBegin(GL_TRIANGLES);
      for (unsigned j=2; j < clip_verts.size(); ++j) {
        const Hcoord &cP = clip_verts[0],
                     &cQ = clip_verts[j-1],
                     &cR = clip_verts[j];
        glVertex4f(cP.x,cP.y,cP.z,cP.w);
        glVertex4f(cQ.x,cQ.y,cQ.z,cQ.w);
        glVertex4f(cR.x,cR.y,cR.z,cR.w);
      }
      glEnd();
    }
  }
}


Client::~Client(void) { }


void Client::draw(double dt) {
  glClearColor(1,1,1,0);
  glClear(GL_COLOR_BUFFER_BIT);
  glClearDepth(1.0f);
  glClear(GL_DEPTH_BUFFER_BIT);

  Mesh *mesh1_ptr = (mode&0x1) == 0 ? static_cast<Mesh*>(&cube)
                                    : static_cast<Mesh*>(&other),
       *mesh2_ptr = (mode&0x2) == 0 ? static_cast<Mesh*>(&cube)
                                    : static_cast<Mesh*>(&other);
  Mesh &mesh1 = *mesh1_ptr,
       &mesh2 = *mesh2_ptr;

  Vector cube1_offset = (mode&0x4) == 0 ? ZERO : offset1,
         cube2_offset = (mode&0x4) == 0 ? ZERO : offset2;

  // cube #1 object-to-world transformation
  float rscale = 0.35f*(sin(0.83f*cube1_rate*float(time)) + 2)
                 /abs(mesh1.GetVertex(0)-mesh1.Center());
  Affine cube1_to_world = translate(cube_center-O+cube1_offset)
                          * rotate(acos(cube1_axis.y),cross(EY,cube1_axis))
                          * rotate(cube1_rate*float(time),EY)
                          * scale(rscale)
                          * translate(O-mesh1.Center());

  // draw wire frame for cube #1
  DisplayEdges(mesh1,cube1_to_world,Vector(1,0,0));

  // set up clipping to cube #1
  world_verts.clear();
  for (int i=0; i < mesh1.VertexCount(); ++i)
    world_verts.push_back(cube1_to_world * mesh1.GetVertex(i));
  Point C = cube1_to_world * mesh1.Center();
  Polyhedron poly;
  for (int i=0; i < mesh1.FaceCount(); ++i) {
    const Mesh::Face &f = mesh1.GetFace(i);
    const Point &P = world_verts[f.index1],
                &Q = world_verts[f.index2],
                &R = world_verts[f.index3];
    poly.half_spaces.push_back(HalfSpace(P,Q,R,C));
  }
  Clip clip(poly);

  // cube #2 object-to-world transformation
  rscale = 0.35f*(sin(0.61f*cube2_rate*float(time)) + 2)
                 /abs(mesh2.GetVertex(0)-mesh2.Center());
  Affine cube2_to_world = translate(cube_center-O+cube2_offset)
                          * rotate(acos(cube2_axis.y),cross(EY,cube2_axis))
                          * rotate(cube2_rate*float(time),EY)
                          * scale(rscale)
                          * translate(O-mesh2.Center());

  // draw wire frame for cube #2
  DisplayEdges(mesh2,cube2_to_world,Vector(0,0,1));

  // draw faces of cube #2 clipped to cube #1
  DisplayClippedFaces(mesh2,cube2_to_world,clip,Vector(0.5f,0.5f,1));

  // set up clipping to cube #2
  world_verts.clear();
  for (int i=0; i < mesh2.VertexCount(); ++i)
    world_verts.push_back(cube2_to_world * mesh2.GetVertex(i));
  C = cube2_to_world * mesh2.Center();
  poly.half_spaces.clear();
  for (int i=0; i < mesh2.FaceCount(); ++i) {
    const Mesh::Face &f = mesh2.GetFace(i);
    const Point &P = world_verts[f.index1],
                &Q = world_verts[f.index2],
                &R = world_verts[f.index3];
    poly.half_spaces.push_back(HalfSpace(P,Q,R,C));
  }
  clip = Clip(poly);

  // draw faces of cube #1 clipped to cube #2
  DisplayClippedFaces(mesh1,cube1_to_world,clip,Vector(1,0.5f,0.5f));

  time += dt;
}


void Client::keypress(SDL_Keycode kc) {
  // respond to keyboard input
  //   kc: SDL keycode (e.g., SDLK_SPACE, SDLK_a, SDLK_s)
  if (kc == SDLK_SPACE) {
    stringstream ss;
    mode = (mode + 1) % 8;
    ss << "CS250 - Clip Demo [mode " << (mode+1) << "/8]";
    SDL_SetWindowTitle(window,ss.str().c_str());
  }
}


void Client::resize(int W, int H) {
  glViewport(0,0,W,H);
  cam = Camera(O,-EZ,EY,0.4f*PI,float(W)/float(H),0.1f,10);
  world_to_ndc = CameraToNDC(cam) * WorldToCamera(cam);
}


/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

int main(int /*argc*/, char * /*argv*/[]) {

  // SDL: initialize and create a window
  SDL_Init(SDL_INIT_VIDEO);
  const char *title = "CS 250: Clip Demo [mode 0/8]";
  int width = 600,
      height = 600;
  SDL_Window *window = SDL_CreateWindow(title,SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,width,height,
                                        SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
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

  // clean up
  delete client;
  SDL_GL_DeleteContext(context);
  SDL_Quit();
  return 0;
}

