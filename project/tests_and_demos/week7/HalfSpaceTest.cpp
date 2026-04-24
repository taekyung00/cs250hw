// HalfSpaceTest.cpp
// -- simple test of half-space package
// cs250 3/17

#include <iostream>
#include <iomanip>
#include "HalfSpace.h"
using namespace std;


bool near(float x, float y) {
  return Hcoord::near(x,y);
}


int main(int /*argc*/, char* /*argv*/ [])
{
  cout << boolalpha;

  { // half-space from normal+point
    Vector m(3,-5,2);
    Point C(7,8,-1);
    HalfSpace h(m,C);
    Vector v1(0,2,5),
           v2(5,3,0);
    cout << near(dot(h,C),0) << ' '
         << near(dot(h,C+v1),0) << ' '
         << near(dot(h,C+v2),0) << ' '
         << (dot(h,C+m) > 0) << ' '
         << (dot(h,C-m) < 0) << endl;
  }

  { // half-space from 4 points
    Point V1(1,1,0),
          V2(2,1,1),
          V3(-1,0,2),
          V4(0,-1,-1);
    HalfSpace h(V1,V2,V3,V4);
    cout << near(dot(h,V1),0) << ' '
         << near(dot(h,V2),0) << ' '
         << near(dot(h,V3),0) << ' '
         << (dot(h,V4) < 0) << endl;
  }

  { // standard cube + point containment
    Polyhedron cube(true);
    Hcoord P(3,-4,5,6),
           Q(1,1,1,-2),
           R(2,7,-4,3);
    cout << contains(cube,P) << ' '
         << contains(cube,Q) << ' '
         << contains(cube,R) << endl;
  }

  { // frustum + point containment
    const float PI = 4.0f*atan(1.0f);
    Polyhedron frustum(0.5f*PI,1,2,10);
    Hcoord P(8,-6,-10,2),
           Q(0,0,3,-1),
           R(0,14,-12,2),
           S(0,0,-1,1),
           T(3,6,-33,3);
    cout << contains(frustum,P) << ' '
         << contains(frustum,Q) << ' '
         << contains(frustum,R) << ' '
         << contains(frustum,S) << ' '
         << contains(frustum,T) << endl;
  }

  return 0;
}

