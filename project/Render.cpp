// Render.cpp
// jsh 1/17

#include <stdexcept>
#include <GL/glew.h>
#include "Render.h"
using namespace std;


Render::Render(void) {
  GLint value;
  GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
  const char *fragment_shader_text =
    "#version 130\n\
     uniform vec3 color;\
     out vec4 frag_color;\
     void main(void) {\
       frag_color = vec4(color,1);\
     }";
  glShaderSource(fshader,1,&fragment_shader_text,0);
  glCompileShader(fshader);
  glGetShaderiv(fshader,GL_COMPILE_STATUS,&value);
  if (value == 0)
    throw runtime_error("fragment shader failed to compile");

  GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
  const char *vertex_shader_text =
    "#version 130\n\
     attribute vec4 position;\
     void main() {\
       gl_Position = position;\
     }";
  glShaderSource(vshader,1,&vertex_shader_text,0);
  glCompileShader(vshader);
  glGetShaderiv(vshader,GL_COMPILE_STATUS,&value);
  if (value == 0)
    throw runtime_error("vertex shader failed to compile");

  program = glCreateProgram();
  glAttachShader(program,fshader);
  glAttachShader(program,vshader);
  glLinkProgram(program);
  glGetProgramiv(program,GL_LINK_STATUS,&value);
  if (value == 0)
    throw runtime_error("shader program failed to link");

  ucolor = glGetUniformLocation(program,"color");
  aposition = glGetAttribLocation(program,"position");
  glEnableVertexAttribArray(aposition);

  glEnable(GL_DEPTH_TEST);
}


Render::~Render(void) {
  glUseProgram(0);
  GLuint shaders[2];
  GLsizei count;
  glGetAttachedShaders(program,2,&count,shaders);
  for (int i=0; i < count; ++i)
    glDeleteShader(shaders[i]);
  glDeleteProgram(program);
}


void Render::ClearBuffers(const Vector& c) {
  glClearColor(c[0],c[1],c[2],1);
  glClear(GL_COLOR_BUFFER_BIT);
  glClearDepth(1);
  glClear(GL_DEPTH_BUFFER_BIT);
}


void Render::SetColor(const Vector& c) {
  glUseProgram(program);
  glUniform3f(ucolor,c[0],c[1],c[2]);
}
  

void Render::DrawLine(const Hcoord& P, const Hcoord& Q) {
  glUseProgram(program);
  GLuint vertex_buffer;
  glGenBuffers(1,&vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER,vertex_buffer);
  float vertices[8] = { P.x,P.y,P.z,P.w, Q.x,Q.y,Q.z,Q.w };
  glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);
  glVertexAttribPointer(aposition,4,GL_FLOAT,false,0,0);
  glDrawArrays(GL_LINES,0,2);
  glDeleteBuffers(1,&vertex_buffer);
}


void Render::FillTriangle(const Hcoord& P, const Hcoord& Q, const Hcoord& R) {
  glUseProgram(program);
  GLuint vertex_buffer;
  glGenBuffers(1,&vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER,vertex_buffer);
  float vertices[12] = { P.x,P.y,P.z,P.w, Q.x,Q.y,Q.z,Q.w, R.x,R.y,R.z,R.w };
  glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);
  glVertexAttribPointer(aposition,4,GL_FLOAT,false,0,0);
  glDrawArrays(GL_TRIANGLES,0,3);
  glDeleteBuffers(1,&vertex_buffer);
}

