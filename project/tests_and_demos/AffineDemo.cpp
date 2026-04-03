// AffineDemo.cpp
// -- demo program for the 'Affine' package
// cs250 9/17
//
// To compile under windows (from Visual Studio 2017 command prompt):
//     cl /EHsc AffineDemo.cpp Affine.cpp opengl32.lib SDL2.lib\
//        SDL2main.lib glew32.lib /link /subsystem:console
//     [you can substitute 'Affine.cpp' with 'Affine.lib']
// To compile under Linux:
//     g++ AffineDemo.cpp Affine.cpp -lGL -lGLEW -lSDL2

#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "Affine.h"
using namespace std;


const float PI = 4.0f*atan(1.0f);
const Point O(0,0,0);
const Vector EZ(0,0,1);

namespace {
  float frand(float a=0, float b=1) {
    return a + (b-a)*float(rand())/float(RAND_MAX);
  }
}


class Client {
  public:
    Client(void);
    ~Client(void);
    void draw(float dt);
    void keypress(SDL_Keycode kc);
    void resize(int W, int H);
  private:
    float time;
    GLint ucolor,
          utransform;
    GLuint vertex_buffer,
           edge_buffer;
    int cube_count;
    Point center;
    float rsize, rot_scale, rot_rate;
    Vector rot_axis;
    Matrix Pers;
};


Client::Client(void)
    : time(0) {

  // compile fragment shader
  const char *fragment_shader_text =
    "#version 130\n\
     uniform vec3 color;\
     out vec4 frag_color;\
     void main(void) {\
       frag_color = vec4(color,1);\
     }";
  GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fshader,1,&fragment_shader_text,0);
  glCompileShader(fshader);

  // compile vertex shader
  const char *vertex_shader_text =
    "#version 130\n\
     attribute vec4 position;\
     uniform mat4 transform;\
     void main() {\
       gl_Position = transform * position;\
     }";
  GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vshader,1,&vertex_shader_text,0);
  glCompileShader(vshader);

  // link shader program
  GLuint program = glCreateProgram();
  glAttachShader(program,fshader);
  glAttachShader(program,vshader);
  glLinkProgram(program);
  glUseProgram(program);

  // shader uniform variable locations
  utransform = glGetUniformLocation(program,"transform");
  ucolor = glGetUniformLocation(program,"color");

  // upload cube vertices to GPU
  glGenBuffers(1,&vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER,vertex_buffer);
  Point cube_verts[8] = { Point( 0.5f, 0.5f,0.5f), Point( 0.5f, 0.5f,-0.5f),
                          Point( 0.5f,-0.5f,0.5f), Point( 0.5f,-0.5f,-0.5f),
                          Point(-0.5f, 0.5f,0.5f), Point(-0.5f, 0.5f,-0.5f),
                          Point(-0.5f,-0.5f,0.5f), Point(-0.5f,-0.5f,-0.5f) };
  glBufferData(GL_ARRAY_BUFFER,sizeof(cube_verts),cube_verts,GL_STATIC_DRAW);

  // bind vertex array to 'position' shader attribute
  GLint aposition;
  aposition = glGetAttribLocation(program,"position");
  glVertexAttribPointer(aposition,4,GL_FLOAT,false,sizeof(Point),0);
  glEnableVertexAttribArray(aposition);

  // upload cube edge list to GPU
  glGenBuffers(1,&edge_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,edge_buffer);
  unsigned cube_edges[12][2] = { {0,1}, {0,2}, {0,4}, {1,3},
                                 {1,5}, {2,3}, {2,6}, {3,7},
                                 {4,5}, {4,6}, {5,7}, {6,7} };
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(cube_edges),cube_edges,GL_STATIC_DRAW);

  // elementary perspective projection
  Pers.row[0] = Hcoord(1, 0, 0, 0);
  Pers.row[1] = Hcoord(0, 1, 0, 0);
  Pers.row[2] = Hcoord(0, 0, 1, 0);
  Pers.row[3] = Hcoord(0, 0,-1, 0);

  // rotating figure parameters
  center = Point(0,0,-1.25f);
  cube_count = 3;
  rsize = 1.0f/cube_count;
  rot_scale = 0.75f;
  rot_rate = 2.0f*PI/10.0f;
  rot_axis = normalize(Vector(frand(),frand(),frand()));
}


Client::~Client(void) {
  glDeleteBuffers(1,&edge_buffer);
  glDeleteBuffers(1,&vertex_buffer);
  GLint program;
  glGetIntegerv(GL_CURRENT_PROGRAM,&program);
  glUseProgram(0);
  GLuint shaders[2];
  GLsizei count;
  glGetAttachedShaders(program,2,&count,shaders);
  for (int i=0; i < count; ++i)
    glDeleteShader(shaders[i]);
  glDeleteProgram(program);
}


void Client::draw(float dt) {

  // clear frame buffer
  glClearColor(1,1,1,1);
  glClear(GL_COLOR_BUFFER_BIT);

  // draw a rotating array of cubes
  Affine R = rotate(acos(dot(EZ,rot_axis)),cross(EZ,rot_axis))
             * rotate(rot_rate*time,EZ);
  for (int i=0; i < cube_count; ++i) {
    for (int j=0; j < cube_count; ++j) {
      for (int k=0; k < cube_count; ++k) {
        // modeling transformation
        Vector offset = rsize * (Vector(float(i),float(j),float(k))
                                 - 0.5f*(cube_count-1)*Vector(1,1,1));
        Affine obj2world = translate(center-O) * R * translate(offset)
                           * scale(rot_scale*rsize);
        // perspective projection
        Matrix obj2dev = Pers * obj2world;
        // render
        glLineWidth(2);
        glUniformMatrix4fv(utransform,1,true,(float*)&obj2dev);
        glUniform3f(ucolor,i*rsize,j*rsize,k*rsize);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,edge_buffer);
        glDrawElements(GL_LINES,2*12,GL_UNSIGNED_INT,0);
      }
    }
  }

  time += dt;
}


void Client::keypress(SDL_Keycode kc) {
  if (kc == SDLK_SPACE) {
    cube_count = 1 + cube_count % 10;
    rsize = 1.0f / cube_count;
    if (cube_count == 1)
    {
        rot_axis = normalize(Vector(frand(), frand(), frand()));
    }
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
  srand(unsigned(time(0)));

  // SDL: initialize and create a window
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = SDL_CreateWindow("CS250: Affine Demo",SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,600,600,
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

  // clean up
  delete client;
  SDL_GL_DeleteContext(context);
  SDL_Quit();
  return 0;
}

