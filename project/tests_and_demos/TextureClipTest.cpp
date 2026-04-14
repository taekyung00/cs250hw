// TextureClipTest.cpp
// -- simple test of TextureClip class.
// cs250 4/17

#include <iostream>
#include <iomanip>
#include "Interpolate.h"
using namespace std;


ostream& operator<<(ostream &s, const TexturedCoord &P) {
  s << '(' << P.x << ',' << P.y << ',' << P.z << ',' << P.w
    << ';' << P.u << ',' << P.v << ')';
  return s;
}


int main(void) {
  cout << boolalpha;
    cout << setprecision(3);

  { // clip a textured triangle against a single half-space
    Hcoord P(-1,0,6,8),
           Q(11,21,15,14),
           R(-9,12,42,12);
    TexturedCoord Ptex(P,-0.125f,0),
                  Qtex(Q,0.786f,1.5f),
                  Rtex(R,-0.75f,1);
    Polyhedron poly;
    poly.half_spaces.push_back(HalfSpace(24,-24,3,11));
    TextureClip tclip(poly);
    vector<TexturedCoord> verts;
    verts.push_back(Ptex);
    verts.push_back(Qtex);
    verts.push_back(Rtex);
    bool result = tclip(verts);
    cout << result << " : ";
    for (unsigned i=0; i < verts.size(); ++i)
       cout << verts[i] << ' ';
    cout << endl;
  }

  { // clip a textured triangle against the standard cube
    Matrix Pi;
    Pi.row[0] = Hcoord(4,0,0,0);
    Pi.row[1] = Hcoord(0,8,0,0);
    Pi.row[2] = Hcoord(0,0,-2,-3);
    Pi.row[3] = Hcoord(0,0,-1,0);
    Point Pcam(-0.5f,0.1f,-0.5f),
          Qcam(0.5f,0.2f,-1.3f),
          Rcam(0.2f,0.3f,-4);
    TexturedCoord Ptex(Pi*Pcam,-1,0.2f),
                  Qtex(Pi*Qcam,1,0.4f),
                  Rtex(Pi*Rcam,0.4f,0.6f);
    Polyhedron poly(true);
    TextureClip tclip(poly);
    vector<TexturedCoord> verts;
    verts.push_back(Ptex);
    verts.push_back(Qtex);
    verts.push_back(Rtex);
    bool result = tclip(verts);
    cout << result << " : ";
    for (unsigned i=0; i < verts.size(); ++i)
       cout << verts[i] << ' ';
    cout << endl;
  }

  return 0;
}

