//#include <cstdlib>
#include <iostream>
//#include <OpenGL/gl.h>
//#include <OpenGL/glu.h>
#include <GLUT/GLUT.h>
#include <cmath>
#include <time.h>
#include <algorithm>
#include <fstream>
#include <cassert>
#include <sstream>
#include <string>

using namespace std;

int width, height;
unsigned char *pixmap;
class Vector3D{
public:
  float x, y, z;
  void setV(float x1, float y1, float z1){
    x = x1, y = y1, z = z1;
  }
  Vector3D(float x1, float y1, float z1){
    x = x1, y = y1, z = z1;
  }
  Vector3D(){ x = 0; y = 0; z = 0; }
  Vector3D operator +(const Vector3D& v) const
  {
    return (Vector3D(x + v.x, y + v.y, z + v.z));
  }
  Vector3D operator /(const float& v) const
  {
    return (Vector3D(x / v, y / v, z / v));
  }
  Vector3D operator *(const float& v) const
  {
    return (Vector3D(x * v, y * v, z * v));
  }
};

Vector3D calcPixel(float x, float y)
{
  int z = 69;//current z plane
  int rad = 100;//r of sphere
  int pw = 50;//plane width
  int ph = height;//plane height
  int tr1 = 50;//R of ellipse
  int tr2 = 20;//r of ellipse
  int color = 0;
  Vector3D center = Vector3D(width / 2, height / 2, 50);
  Vector3D pp = Vector3D(2 * width / 3, height / 2, 50);//point on plane
  Vector3D pn = Vector3D(0, 0, 1);//plane normal
  Vector3D tc = Vector3D(width*.58, height*.68, 50);
  float r = 0; float g = 0; float b = 0;
  
  //dark pink b5385a
  Vector3D dPink = Vector3D(0xb5, 0x38, 0x5a);
  //light pink FF759A
  Vector3D pink = Vector3D(0xff, 0x75, 0x9a);
  //yellow FFF570
  Vector3D yellow = Vector3D(0xff, 0xf5, 0x70);
  //dark blue 3DCFFF
  Vector3D dBlue = Vector3D(0x3d, 0xcf, 0xff);
  //light blue 96E5FF
  Vector3D blue = Vector3D(0x96, 0xe5, 0xff);
  
  
  color = 0;
  if (pow((x - center.x), 2) + pow((y - center.y), 2) + pow((z - center.z), 2) < pow(rad, 2))
  {//if it's inside the sphere
    color = 1;
  }
  if (abs(x - pp.x)<pw && abs(y - pp.y)<ph)
  {//if it's inside the bounded plane
    color += 2;
  }
  if (pow(tr1 - sqrt(pow(x - tc.x, 2) + pow(y - tc.y, 2)), 2) + pow(z - tc.z, 2) - pow(tr2, 2)<0)
  {//if it's inside the torus
    color += 4;
  }
  switch (color)
  {
    case 0:
      r = yellow.x;
      g = yellow.y;
      b = yellow.z;
      break;
    case 1:
      r = pink.x;
      g = pink.y;
      b = pink.z;
      break;
    case 2:
      r = blue.x;
      g = blue.y;
      b = blue.z;
      break;
    case 3:
      r = (blue.x + pink.x) / 2;
      g = (blue.y + pink.y) / 2;
      b = (blue.z + pink.z) / 2;
      break;
    case 4:
      r = dPink.x;
      g = dPink.y;
      b = dPink.z;
      break;
    case 5:
      r = (dPink.x + pink.x) / 2;
      g = (dPink.y + pink.y) / 2;
      b = (dPink.z + pink.z) / 2;
      break;
    case 6:
      r = (blue.x + dPink.x) / 2;
      g = (blue.y + dPink.y) / 2;
      b = (blue.z + dPink.z) / 2;
      break;
    case 7:
      r = (dPink.x + blue.x + pink.x) / 3;
      g = (dPink.y + blue.y + pink.y) / 3;
      b = (dPink.z + blue.z + pink.z) / 3;
      break;
  }
  return Vector3D(r, g, b);
}
Vector3D calcAlias(int x, int y, int a)
{
  int alias = a;
  int r = 0, g = 0, b = 0;
  for (int subY = 0; subY < alias; subY++)
  {
    for (int subX = 0; subX < alias; subX++)
    {
      float pointX = ((double)rand() / (RAND_MAX)) + 1;
      float pointY = ((double)rand() / (RAND_MAX)) + 1;
      pointX += x;
      pointY += y;
      Vector3D color = calcPixel(pointX, pointY);
      r += color.x;
      g += color.y;
      b += color.z;
    }
  }
  if (alias > 0)
  {
    r = r / (alias * alias);
    g = g / (alias * alias);
    b = b / (alias * alias);
  }
  else
  {
    Vector3D color = calcPixel(x, y);
    r = color.x;
    g = color.y;
    b = color.z;
  }
  return Vector3D(r, g, b);
}
void setPixels(int a)
{
  Vector3D aliased;
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      
      int i = (y * width + x) * 3;
      aliased = calcAlias(x, y, a);
      pixmap[i++] = aliased.x;
      pixmap[i++] = aliased.y;
      pixmap[i] = aliased.z;
    }
  }
}
static void windowResize(int w, int h)
{
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, (w / 2), 0, (h / 2), 0, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}
static void windowDisplay(void)
{
  glClear(GL_COLOR_BUFFER_BIT);
  glRasterPos2i(0, 0);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, pixmap);
  glFlush();
}
static void processMouse(int button, int state, int x, int y)
{
  if (state == GLUT_UP)
    exit(0);               // Exit on mouse click.
}

static void init(void)
{
  glClearColor(1, 1, 1, 1); // Set background color.
}



int main(int argc, char *argv[])
{
  width = 640;
  height = 480;
  pixmap = new unsigned char[width * height * 3];
  //setPixels(atoi(argv[1]));
  setPixels(3);
  glutInit(&argc, argv);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(width, height);
  glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
  glutCreateWindow("Problem 1");
  init();
  glutReshapeFunc(windowResize);
  glutDisplayFunc(windowDisplay);
  glutMouseFunc(processMouse);
  glutMainLoop();
  
  return 0;
}