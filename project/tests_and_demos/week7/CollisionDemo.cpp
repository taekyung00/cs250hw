// CollisionDemo.cpp
// -- collision detection using point + polyhedron test
// cs250 7/17
//
// To compile/link from the Visual Studio command prompt:
//     cl /EHsc CollisionDemo.cpp Affine.lib Camera.lib Projection.lib
//        CubeMesh.lib SnubDodecMesh.cpp Render.lib HalfSpace.cpp opengl32.lib
//        glew32.lib SDL2.lib SDL2main.lib /link /subsystem:console
// To compile/link from the Linux command line:
//     g++ CollisionDemo.cpp Affine.cpp Camera.cpp Projection.cpp
//         CubeMesh.cpp SnubDodecMesh.cpp Render.cpp HalfSpace.cpp
//         -lGL -lGLEW -lSDL2

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "CubeMesh.h"
#include "SnubDodecMesh.h"
#include "Projection.h"
#include "HalfSpace.h"
#include "Render.h"
using namespace std;


const float PI = 4.0f*atan(1.0f);
const Point O(0,0,0);
const Vector EX(1,0,0),
             EY(0,1,0),
             EZ(0,0,1);
const Vector WHITE(1,1,1),
             PURPLE(1,0,1),
             BLUE(0,0,1),
             GREEN(0,1,0),
             RED(1,0,0);


namespace {
  struct Object {
    Mesh* mesh_ptr;
    Affine to_world;
    Vector color;
  };


  float frand(float a=0, float b=1) {
    return a + (b-a)*float(rand())/float(RAND_MAX);
  }
}


class Client {
  public:
    Client(void);
    ~Client(void);
    void draw(float dt);
  private:
    Camera camera;
    CubeMesh cube;
    Vector cube_rot_axis;
    float cube_rot_rate,
          cube_size;
    SnubDodecMesh snub;
    Vector snub_move_rate;
    float snub_rot_rate;
    vector<Object> objects;
    vector<Point> temp_world_verts,
                  temp_cam_verts;
    vector<Hcoord> temp_clip_verts;
    float cube_radius;
    bool point_test;
    Render render;
    float time;
};


Client::Client(void) {
  camera = Camera(O+2*EZ,-EZ,EY,0.5f*PI,1,0.1f,10);
  cube_rot_axis = EY;
  cube_rot_rate = 2*PI/10.0f;
  cube_size = 0.4f;
  snub_move_rate = 2*PI*Vector(1/frand(5,15),1/frand(5,15),1/frand(5,15));
  snub_rot_rate = 2*PI/frand(5,15);
  cube_radius = 0.5f*cube_size*sqrt(3.0f);
  point_test = true;
  time = 1;
}


Client::~Client(void) { }


void Client::draw(float dt) {
  render.ClearBuffers(WHITE);
  objects.clear();

  // moving object
  Object object;
    {
      Point P = O + sin(snub_move_rate.x*time)*EX
                  + 0.5f*sin(snub_move_rate.y*time)*EY
                  + sin(snub_move_rate.z*time)*EZ;
      Vector vel = snub_move_rate.x*cos(snub_move_rate.x*time)*EX
                   + 0.5f*snub_move_rate.y*cos(snub_move_rate.y*time)*EY
                   + snub_move_rate.z*cos(snub_move_rate.z*time)*EZ,
             axis = normalize(vel);
      object.mesh_ptr = &snub;
      object.to_world = translate(P-O)
                        * rotate(acos(axis.y),cross(EY,axis))
                        * scale(1.5f*0.5f/snub.Dimensions().x, 1.5f * 2 / snub.Dimensions().y, 1.5f * 0.8f / snub.Dimensions().z)
                        * translate(O-snub.Center());
  }
  // rotating cube
  Affine Mcube = rotate(cube_rot_rate*time,cube_rot_axis)
                 * scale(0.1f*cube_size);

  // intersection test
  temp_world_verts.clear();
  for (int i=0; i < snub.VertexCount(); ++i)
    temp_world_verts.push_back(object.to_world * snub.GetVertex(i));
  Point C = object.to_world * snub.Center();
  Polyhedron poly;
  for (int i=0; i < snub.FaceCount(); ++i) {
    const Mesh::Face& face = snub.GetFace(i);
    const Point &P = temp_world_verts[face.index1],
                &Q = temp_world_verts[face.index2],
                &R = temp_world_verts[face.index3];
    poly.half_spaces.push_back(HalfSpace(P,Q,R,C));
  }
  bool inside = contains(poly,O);
  object.color = inside ? PURPLE : BLUE;
  objects.push_back(object);

  // stationary rotating cube
  if (!inside) {
    object.mesh_ptr = &cube;
    object.color = GREEN;
    object.to_world = Mcube;
  }
  else {
    object.mesh_ptr = &snub;
    object.color = RED;
    object.to_world = rotate(cube_rot_rate*time,cube_rot_axis)
                      * scale(0.5f*cube_size/snub.Dimensions().x)
                      * translate(O-snub.Center());
  }
  objects.push_back(object);

  // render objects
  Affine world_to_cam = WorldToCamera(camera);
  Matrix cam_to_clip = CameraToNDC(camera);
  for (unsigned i=0; i < objects.size(); ++i) {
    Mesh& mesh = *objects[i].mesh_ptr;
    Affine obj_to_cam = world_to_cam * objects[i].to_world;
    const Vector& color = objects[i].color;
    temp_cam_verts.clear();
    temp_clip_verts.clear();
    for (int j=0; j < mesh.VertexCount(); ++j) {
      temp_cam_verts.push_back(obj_to_cam * mesh.GetVertex(j));
      temp_clip_verts.push_back(cam_to_clip * temp_cam_verts[j]);
    }
    for (int j=0; j < mesh.FaceCount(); ++j) {
      const Mesh::Face& face = mesh.GetFace(j);
      const Point &Pcam = temp_cam_verts[face.index1],
                  &Qcam = temp_cam_verts[face.index2],
                  &Rcam = temp_cam_verts[face.index3];
      const Hcoord &Pclip = temp_clip_verts[face.index1],
                   &Qclip = temp_clip_verts[face.index2],
                   &Rclip = temp_clip_verts[face.index3];
      Vector normal = normalize(cross(Qcam-Pcam,Rcam-Pcam));
      float mu = normal.z;
      render.SetColor(mu*color);
      render.FillTriangle(Pclip,Qclip,Rclip);
    }
  }

  time += dt;
}


/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

int main(int /*argc*/, char * /*argv*/[]) {
  srand(unsigned(time(nullptr)));

  // SDL: initialize and create a window
  SDL_Init(SDL_INIT_VIDEO);
  const char *title = "CS 250: Collision Demo";
  int width = 600,
      height = 600;
  SDL_Window *window = SDL_CreateWindow(title,SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,width,height,
                                        SDL_WINDOW_OPENGL);
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

