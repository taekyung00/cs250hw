// TriangleTest.cpp
// -- test harness for triangle filling algorithm (y-axis up).  Does
//    not test for z-buffer correctness.
// cs250 10/17
//
// To compile using Visual Studio command prompt:
//     cl /EHsc TriangleTest.cpp Affine.lib CubeMesh.lib Camera.lib Projection.lib
//        RasterUtilites.cpp opengl32.lib SDL2.lib SDL2main.lib /link /subsystem:console
// To compile using Linux command line:
//     g++ TriangleTest.cpp Affine.cpp CubeMesh.cpp Camera.cpp Projection.cpp
//         RasterUtilites.cpp -lGL -lSDL2

#include <iostream>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "Affine.h"
#include "Raster.h"
#include "RasterUtilities.h"
using namespace std;


class Client {
  public:
    Client(void);
    ~Client(void);
    void draw(void);
    void resize(int W, int H);
    void mousedown(int x, int y);
    void mousemove(int x, int y);
  private:
  int width, height;
  int cell_count_y,
      cell_count_x,
      xmin, ymin, xmax, ymax;
  int vertices[6];
  int vertex_count;
  Raster::byte *grid;
  float *gridz;
  bool reset;
};


Client::Client(void) {
  width = 600;
  height = 600;
  cell_count_y = 30;
  for (int i=0; i < 6; ++i)
    vertices[i] = 0;
  vertex_count = 0;
  reset = true;

  float ratio = float(width)/float(height);
  cell_count_x = int(ratio*cell_count_y);
  xmin = int(0.5f*width/(cell_count_x+1));
  xmax = int((cell_count_x+0.5f)*width/(cell_count_x+1));
  ymin = int(0.5f*height/(cell_count_y+1));
  ymax = int((cell_count_y+0.5f)*height/(cell_count_y+1));

  SDL_Rect rect;
  SDL_GetDisplayBounds(0,&rect);
  grid = new Raster::byte[3*rect.w*rect.h];
  gridz = new float[rect.w*rect.h];
}


Client::~Client(void) {
  delete[] gridz;
  delete[] grid;
}


void Client::draw(void) {
  // clear the screen
  glClearColor(1,1,1,0);
  glClear(GL_COLOR_BUFFER_BIT);

  // screen to NDC transform
  Point ScreenCenter(0.5f*width,0.5f*height,0),
        GridCenter(0.5f*cell_count_x,0.5f*cell_count_y,0),
        O(0,0,0);
  Affine N = scale(2.0f/width,-2.0f/height,1) * translate(O-ScreenCenter);

  // screen to grid transform
  Affine G = translate(GridCenter-O)
             * scale(float(cell_count_x+1)/float(width),
                     -float(cell_count_y+1)/float(height),1)
             * translate(O-ScreenCenter),
         iG = inverse(G);

  // grid to NDC transform
  Affine NG = N * iG;

  // draw grid
  glColor3d(0,0,0);
  for (int i=0; i <= cell_count_y; ++i) {
    glBegin(GL_LINES);
    Point P = O + float(i)*Vector(0,1,0),
          Q = P + float(cell_count_x)*Vector(1,0,0);
    P = NG*P;
    Q = NG*Q;
    glVertex2f(P.x,P.y);
    glVertex2f(Q.x,Q.y);
    glEnd();
  }
  for (int j=0; j <= cell_count_x; ++j) {
    glBegin(GL_LINES);
    Point P = O + float(j)*Vector(1,0,0),
          Q = P + float(cell_count_y)*Vector(0,1,0);
    P = NG*P;
    Q = NG*Q;
    glVertex2f(P.x,P.y);
    glVertex2f(Q.x,Q.y);
    glEnd();
  }

  // draw actual edge
  glColor3d(0,0,1);
  if (vertex_count == 1) {
    Point A[2], P[2];
    for (int i=0; i < 2; ++i) {
      A[i] = G * Point(float(vertices[2*i]),float(vertices[2*i+1]),0);
      P[i] = NG*A[i];
    }
    glBegin(GL_LINES);
    glVertex2f(P[0].x,P[0].y);
    glVertex2f(P[1].x,P[1].y);
    glEnd();
  }

  else if (!reset) {
    Point A[3], P[3];
    for (int i=0; i < 3; ++i) {
      A[i] = G * Point(float(vertices[2*i]),float(vertices[2*i+1]),0);
      P[i] = NG*A[i];
    }
    glBegin(GL_LINE_LOOP);
    for (int i=0; i < 3; ++i)
      glVertex2f(P[i].x,P[i].y);
    glEnd();

    // draw pixels on grid
    Vector u(0.4f,0,0),
           v(0,0.4f,0);
    int grid_width = cell_count_x + 1,
        grid_height = cell_count_y + 1,
        stride = 3*grid_width;
    Raster raster(grid,gridz,grid_width,grid_height,stride);
    ClearBuffers(raster);
    raster.SetColor(1,1,1);
    FillTriangle(raster,A[0],A[1],A[2]);
    for (int i=0; i < grid_height; ++i)
      for (int j=0; j < grid_width; ++j)
        if (grid[stride*i+3*j] != 0) {
          Point V1(j-0.2f,i-0.2f,0),
                V2 = V1 + u,
                V3 = V2 + v,
                V4 = V1 + v;
          V1 = NG*V1;
          V2 = NG*V2;
          V3 = NG*V3;
          V4 = NG*V4;
          glColor3d(1,0,0);
          glBegin(GL_POLYGON);
          glVertex2f(V1.x,V1.y);
          glVertex2f(V2.x,V2.y);
          glVertex2f(V3.x,V3.y);
          glVertex2f(V4.x,V4.y);
          glEnd();
          glColor3d(0,0,0);
          glBegin(GL_LINE_LOOP);
          glVertex2f(V1.x,V1.y);
          glVertex2f(V2.x,V2.y);
          glVertex2f(V3.x,V3.y);
          glVertex2f(V4.x,V4.y);
          glEnd();
        }
  }
}


void Client::resize(int W, int H) {
  glViewport(0,0,W,H);
  width = W;
  height = H;
  float ratio = float(width)/float(height);
  cell_count_x = int(ratio*cell_count_y);
  // grid bounds in screen space
  xmin = int(0.5f*width/(cell_count_x+1));
  xmax = int((cell_count_x+0.5f)*width/(cell_count_x+1));
  ymin = int(0.5f*height/(cell_count_y+1));
  ymax = int((cell_count_y+0.5f)*height/(cell_count_y+1));
  reset = true;
}


void Client::mousedown(int x, int y) {
  if (xmin <= x && x <= xmax && ymin <= y && y <= ymax) {
    reset = false;
    vertices[2*vertex_count] = x;
    vertices[2*vertex_count+1] = y;
    ++vertex_count;
    if (vertex_count < 3) {
      vertices[2*vertex_count] = x;
      vertices[2*vertex_count+1] = y; }
    else
      vertex_count = 0;
  }
}


void Client::mousemove(int x, int y) {
  if (xmin <= x && x <= xmax && ymin <= y && y <= ymax) {
    if (vertex_count != 0) {
      vertices[2*vertex_count] = x;
      vertices[2*vertex_count+1] = y;
    }
  }
}


/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

int main(int /*argc*/, char * /*argv*/[]) {

  // SDL: initialize and create a window
  SDL_Init(SDL_INIT_VIDEO);
  const char *title = "CS 250: Triangle Pixel Test";
  int width = 600,
      height = 600;
  SDL_Window *window = SDL_CreateWindow(title,SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,width,height,
                                        SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
  SDL_GLContext context = SDL_GL_CreateContext(window);

  // animation loop
  bool done = false;
  Client *client = new Client();
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
        case SDL_MOUSEBUTTONDOWN:
          if (event.button.button == SDL_BUTTON_LEFT)
            client->mousedown(event.button.x,event.button.y);
          break;
        case SDL_MOUSEMOTION:
          client->mousemove(event.button.x,event.button.y);
          break;
      }
    }
    client->draw();
    SDL_GL_SwapWindow(window);
  }

  // clean up
  delete client;
  SDL_GL_DeleteContext(context);
  SDL_Quit();
  return 0;
}

