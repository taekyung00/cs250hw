// ClipTest.cpp
// -- simple test of clipping routines
// cs250 7/17

#include <iostream>
#include <iomanip>
#include "Clip.h"
using namespace std;


ostream& operator<<(ostream& s, const Hcoord& P) {
  s << '(' << P.x << ',' << P.y << ',' << P.z << ',' << P.w << ')';
  return s;
}


int main(void) {
  cout << boolalpha << fixed << setprecision(3);

  { // clip a triangle to the near plane
    vector<Hcoord> verts;
    verts.push_back(Hcoord(40,-3,15,9));
    verts.push_back(Hcoord(-32,24,-21,-9));
    verts.push_back(Hcoord(-20,8,7,31));
    Polyhedron poly;
    poly.half_spaces.push_back(HalfSpace(0,0,-1,-1));
    Clip clip(poly);
    bool is_intersection = clip(verts);
    cout << is_intersection << " : ";
    for (unsigned i=0; i < verts.size(); ++i)
      cout << verts[i] << ' ';
    cout << endl;
  }

  { // clip a triangle to the unit cube
    vector<Hcoord> verts;
    verts.push_back(Hcoord(2,1,-4,-1));
    verts.push_back(Hcoord(1,-1,8,7));
    verts.push_back(Hcoord(-3,0,2,3));
    Polyhedron poly(true);
    Clip clip(poly);
    bool is_intersection = clip(verts);
    cout << is_intersection << " : ";
    for (unsigned i=0; i < verts.size(); ++i)
      cout << verts[i] << ' ';
    cout << endl;
  }

  return 0;
}

