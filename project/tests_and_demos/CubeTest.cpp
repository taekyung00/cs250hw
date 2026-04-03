// CubeTest.cpp
// -- display a rotating cube (uses CubeMesh)
// cs200 5/17
//
// To compile/link using Visual Studio Command Prompt:
//     cl /EHsc CubeTest.cpp Affine.lib CubeMesh.cpp opengl32.lib
//     glew32.lib SDL2.lib SDL2main.lib /link /subsystem:console
// To compile under Linux:
//     g++ CubeTest.cpp Affine.cpp CubeMesh.cpp -lGL -lGLEW -lSDL2


#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "CubeMesh.h"
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
    CubeMesh cube;
    Point cube_center;
    float cube_scale,
          cube_rot_rate;
    Vector cube_rot_axis;
    float time;
    bool draw_solid;
    Affine ObjToWorld;
    Matrix PersProj;
    GLint program[2],
          aposition[2],
          anormal,
          utransform[2],
          uprojection[2];
    GLuint buffers[4];
};


Client::Client(void) {
  // compile and link wire frame shader
  const char *fragment_shader_text_wire =
    "#version 130\n\
     out vec4 frag_color;\
     void main(void) {\
       frag_color = vec4(0.5,0,0.5,1);\
     }";
  GLuint fshader_wire = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fshader_wire,1,&fragment_shader_text_wire,0);
  glCompileShader(fshader_wire);

  const char *vertex_shader_text_wire =
    "#version 130\n\
     attribute vec4 position;\
     uniform mat4 transform;\
     uniform mat4 projection;\
     void main(void) {\
       gl_Position = projection * transform * position;\
     }";
  GLuint vshader_wire = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vshader_wire,1,&vertex_shader_text_wire,0);
  glCompileShader(vshader_wire);

  program[0] = glCreateProgram();
  glAttachShader(program[0],fshader_wire);
  glAttachShader(program[0],vshader_wire);
  glLinkProgram(program[0]);

  // compile and link solid shader program
  const char *fragment_shader_text_solid =
    "#version 130\n\
     in vec4 vcolor;\
     out vec4 frag_color;\
     void main(void) {\
       frag_color = vcolor;\
     }";
  GLuint fshader_solid = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fshader_solid,1,&fragment_shader_text_solid,0);
  glCompileShader(fshader_solid);

  const char *vertex_shader_text_solid =
    "#version 130\n\
     attribute vec4 position;\
     attribute vec4 normal;\
     uniform mat4 transform;\
     uniform mat4 projection;\
     out vec4 vcolor;\
     void main() {\
       vec4 P = transform * position;\
       gl_Position = projection * P;\
       vec4 m = normalize(transform * normal);\
       float value = max(0.0,m.z);\
       vcolor = vec4(value,0,value,1);\
     }";
  GLuint vshader_solid = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vshader_solid,1,&vertex_shader_text_solid,0);
  glCompileShader(vshader_solid);

  program[1] = glCreateProgram();
  glAttachShader(program[1],fshader_solid);
  glAttachShader(program[1],vshader_solid);
  glLinkProgram(program[1]);

  // shader parameter locations
  for (int i=0; i < 2; ++i) {
    aposition[i] = glGetAttribLocation(program[i],"position");
    utransform[i] = glGetUniformLocation(program[i],"transform");
    uprojection[i] = glGetUniformLocation(program[i],"projection");
  }
  anormal = glGetAttribLocation(program[1],"normal");

  glEnable(GL_DEPTH_TEST);

  glGenBuffers(4,buffers);

  // vertex buffer for wire frame
  int count = cube.VertexCount();
  Hcoord *htemp = new Hcoord[count];
  for (int i=0; i < cube.VertexCount(); ++i)
    htemp[i] = cube.GetVertex(i);
  glBindBuffer(GL_ARRAY_BUFFER,buffers[0]);
  glBufferData(GL_ARRAY_BUFFER,count*sizeof(Hcoord),htemp,GL_STATIC_DRAW);
  delete[] htemp;

  // edge buffer for wire frame
  count = cube.EdgeCount();
  Mesh::Edge *etemp = new Mesh::Edge[count];
  for (int i=0; i < cube.EdgeCount(); ++i)
    etemp[i] = cube.GetEdge(i);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,buffers[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,count*sizeof(Mesh::Edge),etemp,GL_STATIC_DRAW);
  delete[] etemp;

  // vertex buffer for solid
  // -- flat shading, so need do this on a per face basis
  count = 3 * cube.FaceCount();
  htemp = new Hcoord[count];
  for (int i=0; i < cube.FaceCount(); ++i) {
    const Mesh::Face &f = cube.GetFace(i);
    htemp[3*i+0] = cube.GetVertex(f.index1);
    htemp[3*i+1] = cube.GetVertex(f.index2);
    htemp[3*i+2] = cube.GetVertex(f.index3);
  }
  glBindBuffer(GL_ARRAY_BUFFER,buffers[2]);
  glBufferData(GL_ARRAY_BUFFER,count*sizeof(Hcoord),htemp,GL_STATIC_DRAW);

  // normal buffer for solid
  for (int i=0; i < cube.FaceCount(); ++i) {
    const Mesh::Face &f = cube.GetFace(i);
    const Point &P = cube.GetVertex(f.index1),
                &Q = cube.GetVertex(f.index2),
                &R = cube.GetVertex(f.index3);
    Vector m = cross(Q-P,R-P);
    for (int j=0; j < 3; ++j)
      htemp[3*i+j] = m;
  }
  glBindBuffer(GL_ARRAY_BUFFER,buffers[3]);
  glBufferData(GL_ARRAY_BUFFER,count*sizeof(Hcoord),htemp,GL_STATIC_DRAW);
  delete[] htemp;

  // cube parameters
  cube_center = Point(0,0,-2.5f);
  cube_scale = 1.4f;
  cube_rot_rate = 2*PI/10.0f;
  cube_rot_axis = normalize(Vector(frand(-1,1),frand(-1,1),frand(-1,1)));
  draw_solid = false;
  time = 0;

  // perspective projection
  PersProj.row[0] = Hcoord(1.4f,0,0,0);
  PersProj.row[1] = Hcoord(0,1.4f,0,0);
  PersProj.row[2] = Hcoord(0,0,-10.1f/9.9f,-2/9.9f);
  PersProj.row[3] = Hcoord(0,0,-1,0);
}


Client::~Client(void) {
  glDeleteBuffers(4,buffers);
  glUseProgram(0);
  for (int i=0; i < 2; ++i) {
    GLuint shaders[2];
    GLsizei count;
    glGetAttachedShaders(program[i],2,&count,shaders);
    for (int j=0; j < count; ++j)
      glDeleteShader(shaders[j]);
    glDeleteProgram(program[i]);
  }
}


void Client::draw(float dt) {
  // clear frame buffer and z-buffer
  glClearColor(0.95f,0.95f,0.95f,1);
  glClearDepth(1);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  // modeling transform update
  ObjToWorld = translate(cube_center-O)
                      * rotate(acos(dot(EZ,cube_rot_axis)),
                               cross(EZ,cube_rot_axis))
                      * rotate(cube_rot_rate*time,EZ)
                      * scale(0.5f*cube_scale);

  int index = draw_solid ? 1 : 0;
  glUseProgram(program[index]);
  glEnableVertexAttribArray(aposition[index]);
  glUniformMatrix4fv(utransform[index],1,true,(float*)&ObjToWorld);
  glUniformMatrix4fv(uprojection[index],1,true,(float*)&PersProj);

  if (draw_solid) {
    glBindBuffer(GL_ARRAY_BUFFER,buffers[2]);
    glVertexAttribPointer(aposition[1],4,GL_FLOAT,false,0,0);
    glEnableVertexAttribArray(anormal);
    glBindBuffer(GL_ARRAY_BUFFER,buffers[3]);
    glVertexAttribPointer(anormal,4,GL_FLOAT,false,0,0);
    glDrawArrays(GL_TRIANGLES,0,3*cube.FaceCount());
  }

  else {
    glLineWidth(2);
    glBindBuffer(GL_ARRAY_BUFFER,buffers[0]);
    glVertexAttribPointer(aposition[0],4,GL_FLOAT,false,0,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,buffers[1]);
    glDrawElements(GL_LINES,2*cube.EdgeCount(),GL_UNSIGNED_INT,0);
  }

  time += dt;
}


void Client::keypress(SDL_Keycode kc) {
  if (kc == SDLK_SPACE)
    draw_solid = !draw_solid;
  else if (kc == SDLK_r)
    cube_rot_axis = normalize(Vector(frand(-1,1),frand(-1,1),frand(-1,1)));
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
  const char *title = "CS 250: CubeMesh Test";
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

