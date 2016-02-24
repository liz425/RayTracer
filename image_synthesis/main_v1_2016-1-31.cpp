//
//  main.cpp
//  image_synthesis
//
//  Created by 李镇 on 1/22/16.
//  Copyright © 2016 zl. All rights reserved.


#include <iostream>
#include <GLUT/GLUT.h>
#include <cmath>
#include <vector>
#include <cfloat>

using namespace std;

int width, height;
unsigned char *pixmap;

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
class Vector3D{
public:
  double x, y, z;
  
  //Vector3D constructor
  Vector3D(double x1, double y1, double z1): x(x1), y(y1), z(z1){}
  
  Vector3D(){
    x = 0; y = 0; z = 0;
  }
  
  void setV(double x_in, double y_in, double z_in){
    x = x_in; y = y_in; z = z_in;
  }
  
  //operator overloading, support + - * /
  Vector3D operator +(const Vector3D& v) const{
    return (Vector3D(x + v.x, y + v.y, z + v.z));
  }
  
  //vector subtraction: AX - AY = YX
  Vector3D operator -(const Vector3D& v) const{
    return (Vector3D(x - v.x, y - v.y, z - v.z));
  }
  
  Vector3D operator /(const double& v) const{
    return (Vector3D(x / v, y / v, z / v));
  }
  
  Vector3D operator *(const double& v) const{
    return (Vector3D(x * v, y * v, z * v));
  }
  
  double length(){
    return sqrt(x * x + y * y + z * z);
  }
  
  //member function normalize will change Vector3D object itself
  Vector3D normalize(){
    double l = this->length();
    if (l != 0) {
      x /= l;
      y /= l;
      z /= l;
    }
    return *this;
  }
  
  //define dot product and cross_product of vectors
  double dot(const Vector3D& v) const{
    return (x * v.x + y * v.y + z * v.z);
  }
  
  Vector3D cross(const Vector3D& v) const{
    return Vector3D(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
  }
  
};

//function Normalize will NOT change input Vector3D v itself
Vector3D Normalize(Vector3D v){
  double l = v.length();
  //cout << v.x << " " << v.y << " " << v.z << " : " << l << endl;
  //cout << l << endl;
  if (l != 0) {
    return (v / l);
  }else{
    return v;
  }
}

double DotProduct(Vector3D u, Vector3D v){
  return (u.x * v.x + u.y * v.y + u.z * v.z);
}

Vector3D CrossProduct(Vector3D u, Vector3D v){
  return Vector3D(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
class Point3D{
public:
  double x, y, z;

  Point3D(double x_in, double y_in, double z_in){
    x = x_in; y = y_in; z = z_in;
  }
  
  Point3D(){
    x = 0; y = 0; z = 0;
  }
  
  //subtraction of Point3Ds generates Vector3D
  Vector3D operator -(const Point3D& p) const{
    return Vector3D(x - p.x, y - p.y, z - p.z);
  }
  
  //PointA + VectorAB = PointB
  Point3D operator +(const Vector3D& v) const{
    return Point3D(x + v.x, y + v.y, z + v.z);
  }
  
  //PointB - VectorAB = PointA
  Point3D operator -(const Vector3D& v) const{
    return Point3D(x - v.x, y - v.y, z - v.z);
  }
  
};


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
class View{
public:
  Point3D pe;  //eye position
  Vector3D npe; //direction of view, npe MUST BE unit vector
  
  View(){
    pe = Point3D(0, 0, 0);
    npe = Vector3D(0, 1, 0);
  }
  
  View(Point3D pe_in, Vector3D npe_in):pe(pe_in), npe(Normalize(npe_in)){};
  
  View(double pe_x, double pe_y, double pe_z, double npe_x, double npe_y, double npe_z){
    pe = Point3D(pe_x, pe_y, pe_z);
    npe = Normalize(Vector3D(npe_x, npe_y, npe_z));
  }
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
class Color{
public:
  unsigned char r;
  unsigned char g;
  unsigned char b;
  Color(){
    r = 0; g = 0; b = 0;
  }
  
  Color(char r_in, char g_in, char b_in): r(r_in), g(g_in), b(b_in){}
  
  Color(unsigned int clr){
    r = (clr & 0xff0000) >> 16;
    g = (clr & 0xff00) >> 8;
    b = (clr & 0xff);
  }
};


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
class AnyObject{
public:
  Color clr;
  virtual const pair<Color, double> CalcIntersect(View eye) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
class Plane : public AnyObject{
public:
  Point3D pi;
  Vector3D ni;
  Color clr;
  
  Plane(){
    pi = Point3D(0, 0, 0);
    ni = Vector3D(0, 0, 0);
    clr = Color(0, 0, 0);
  }
  
  Plane(Point3D pi_in, Vector3D ni_in, Color clr_in): pi(pi_in), ni(ni_in), clr(clr_in){}
  
  Plane(double pi_x, double pi_y, double pi_z, double ni_x, double ni_y, double ni_z, char r, char g, char b){
    pi = Point3D(pi_x, pi_y, pi_z);
    ni = Vector3D(ni_x, ni_y, ni_z);
    clr = Color(r, g, b);
  }
  
  const pair<Color, double> CalcIntersect(View eye){
    //check if eye position behind the plane
    if (DotProduct(this->ni, eye.pe - this->pi) <= 0) {
      cout << "Wrong eye position: Behind the plane!" << endl;
      return make_pair(Color(0, 0, 0), -1);
    }
    //When no intersection, return t = -1
    if (DotProduct(this->ni, eye.npe) >= 0) {
      return make_pair(Color(0, 0, 0), -1);
    }
    double t = -DotProduct(this->ni, eye.pe - this->pi)/DotProduct(this->ni, eye.npe);
    return make_pair(this->clr, t);
  }

};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
class Sphere : public AnyObject{
  //(p - pi) dot_product (p - pi) - r^2 = 0
public:
  Point3D pi;
  double radius;  //radius of sphere
  Color clr;
  
  Sphere(){
    pi = Point3D(0, 0, 0);
    radius = 1;
    clr = Color(0, 0, 0);
  }
  
  Sphere(Point3D pi_in, double radius_in, Color clr_in): pi(pi_in), radius(radius_in), clr(clr_in){}
  
  Sphere(double x_in, double y_in, double z_in, double radius_in, char r_in, char g_in, char b_in){
    pi = Point3D(x_in, y_in, z_in);
    radius = radius_in;
    clr = Color(r_in, g_in, b_in);
  }
  
  
  const pair<Color, double> CalcIntersect(View eye){
    //cout << eye.npe.length() << endl;
    //Check if eye position inside the sphere, return -1 when true
    if (DotProduct(eye.pe - this->pi, eye.pe - this->pi) - pow(this->radius, 2) <= 0){
      cout << "Wrong eye position: Inside of the sphere!" << endl;
      return make_pair(Color(0, 0, 0), -1);
    }
    
    double B = DotProduct(eye.npe, (this->pi - eye.pe));
    double C = DotProduct(eye.pe - this->pi, eye.pe - this->pi) - pow(this->radius, 2);
    double delta = pow(B, 2) - C;
    if (delta < 0) {   //when no intesection, return t = -1
      return make_pair(Color(0, 0, 0), -1);
    }else{
      double t = B - sqrt(delta);
      return make_pair(this->clr, t);
    }
  }
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
class Cone : public AnyObject{
  //x^2/A^2 + y^2/B^2 = z^2/C^2
public:
  Color clr;
  double a;
  double b;
  double c;
  
  Cone(){
    a = 1;
    b = 1;
    c = 1;
    clr = Color(0, 0, 0);
  }
  
  Cone(double a_in, double b_in, double c_in, Color clr_in): a(a_in), b(b_in), c(c_in), clr(clr_in){}
  
  Cone(double a_in, double b_in, double c_in, char clr_r, char clr_g, char clr_b){
    a = a_in; b = b_in; c = c_in;
    clr = Color(clr_r, clr_g, clr_b);
  }
  
  const pair<Color, double> CalcIntersect(View eye){
    ////////////////
    // INCOMPLETE //
    ////////////////
    double t = 1;
    return make_pair(this->clr, t);
  }
};

/*
class Quadric{
public:
  int a02, a12, a22, a21, a00;
  Point3D pc;
  
  Quadric(int a02_in, int a12_in, int a22_in, int a21_in, int a00_in){
    a02 = a02_in;
    a12 = a12_in;
    a22 = a22_in;
    a21 = a21_in;
    a00 = a00_in;
  }
};
*/

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
Color CalcSubPixel(View eye, vector<AnyObject*> objs){
  //for all objects, find the closest object
  int obj_number = (int)objs.size();
  double t_min = FLT_MAX;
  Color clr = Color(0, 0, 0);
  for(int i = 0; i < obj_number; i++){
    pair<Color, double> intsec = objs[i]->CalcIntersect(eye);
    double t_tmp = intsec.second;
    Color clr_tmp = intsec.first;
    if (t_tmp >= 0 && t_tmp < t_min) {
//      /* test code */
//      if (t_min - t_tmp <= 2) {
//        cout << t_tmp << endl;
//      }
//      /* test end */
      
      t_min = t_tmp;
      clr = clr_tmp;
    }
  }
  //cout << t_min << endl;
  return clr;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
class Scene{
public:
  Vector3D v_up;
  Vector3D v_view;
  Point3D p_eye;
  double dist;
  double s_x;
  double s_y;
  vector<AnyObject*> objs;
  
  Scene(){
    v_up = Vector3D(0, 0, 1);
    v_view = Vector3D(0, 1, 0);
    p_eye = Point3D(0, -10, 0);
    dist = 5;
    s_x = 10;
    s_y = 10;
  }
  
  int obj_number(){
    return (int)objs.size();
  }
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
View CalcView(Scene sce, double x_per, double y_per){
  //Vector start with 'n' are UNIT VECTORS
  Vector3D n_view = Normalize(sce.v_view);
  Point3D p_center = sce.p_eye + n_view * sce.dist;
  Vector3D v0 = CrossProduct(sce.v_view, sce.v_up);
  Vector3D n0 = Normalize(v0);
  Vector3D n1 = CrossProduct(n0, n_view);
  Point3D p00 = p_center - n0 * (sce.s_x / 2) - n1 * (sce.s_y / 2);
  Point3D pp = p00 + n0 * (x_per * sce.s_x) + n1 * (y_per * sce.s_y);
  Vector3D v_pe = pp - sce.p_eye;
  Vector3D npe = Normalize(v_pe);
  //cout << npe.x << " " << npe.y << " " << npe.z << endl;
  //cout << npe.length() << endl;
  return View(sce.p_eye, npe);
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
Color CalcPixel(Scene sce, int x, int y, int alias){
  double red = 0;
  double green = 0;
  double blue = 0;
  Color clr;
  for (int q = 0; q < alias; q++){
    for (int p = 0; p < alias; p++){
      double pointX = ((double)rand() / RAND_MAX + p) / alias + x;
      double pointY = ((double)rand() / RAND_MAX + q) / alias + y;
      
      double x_per = pointX / width;
      double y_per = pointY / height;
      
      View eye = CalcView(sce, x_per, y_per);
      Color clr_sub = CalcSubPixel(eye, sce.objs);
      
      red += clr_sub.r;
      green += clr_sub.g;
      blue += clr_sub.b;
    }
  }
  
  if (alias > 0){
    red = red / (alias * alias);
    green = green / (alias * alias);
    blue = blue / (alias * alias);
    clr = Color(red, green, blue);
  }else{
    double x_per = (double)x / width;
    double y_per = (double)y / height;
    View eye = CalcView(sce, x_per, y_per);
    clr = CalcSubPixel(eye, sce.objs);
  }
  return clr;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
void setPixels(Scene sce, int alias)
{
  Color clr;
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      
      int i = (y * width + x) * 3;
      clr = CalcPixel(sce, x, y, alias);
      pixmap[i++] = clr.r;
      pixmap[i++] = clr.g;
      pixmap[i]   = clr.b;
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
  int alias = 3;
  pixmap = new unsigned char[width * height * 3];
  vector<AnyObject*> objs;
  
  AnyObject* plane1 = (AnyObject*)new Plane(Point3D(0, 20, 0), Vector3D(0, -1, -1), Color(0x00ff00));
  objs.push_back(plane1);
  AnyObject* plane2 = (AnyObject*)new Plane(Point3D(0, 20, 0), Vector3D(1, -1, 1), Color(0xff0000));
  objs.push_back(plane2);
  AnyObject* sphere1 = (AnyObject*)new Sphere(Point3D(2, 3, 4), 5, Color(0x0000ff));
  objs.push_back(sphere1);
  
  Scene sce;
  sce.objs = objs;
  setPixels(sce, alias);
  glutInit(&argc, argv);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(width, height);
  glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
  glutCreateWindow("Project 1");
  init();
  glutReshapeFunc(windowResize);
  glutDisplayFunc(windowDisplay);
  glutMouseFunc(processMouse);
  glutMainLoop();
  
  return 0;
}