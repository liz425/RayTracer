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

//specular type, 0 -> soft, 1 -> sharp, 2 -> other shape
#define SpecularType 0
#define _hard_spot_light_


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
  ////cout << v.x << " " << v.y << " " << v.z << " : " << l << endl;
  ////cout << l << endl;
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
  
  Color operator *(const double& v) const{
    return (Color(r * v, g * v, b * v));
  }
  
  Color operator *(const Vector3D& v) const{
    return (Color(r * v.x, g * v.y, b * v.z));
  }
  
  Color operator /(const double& v) const{
    return (Color(r / v, g / v, b / v));
  }
  
  Color operator +(const Color& v) const{
    return (Color(r + v.r, g + v.g, b + v.b));
  }
  
  Color operator -(const Color& v) const{
    return (Color(r - v.r, g - v.g, b - v.b));
  }

};


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
class AnyObject{
public:
  Color clr;
  virtual const tuple<Color, double, Point3D, Vector3D> CalcIntersect(View eye) = 0;
  virtual const string GetObjectType() = 0;
//  virtual void SetN(Vector3D n0, Vector3D n1, Vector3D n2) = 0;
//  virtual void SetS(float s0, float s1, float s2) = 0;
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
  
  const tuple<Color, double, Point3D, Vector3D> CalcIntersect(View eye){
    //check if eye position behind the plane
    if (DotProduct(this->ni, eye.pe - this->pi) <= 0) {
      //cout << "Wrong eye position: Behind the plane!" << endl;
      return make_tuple(Color(0, 0, 0), -1, Point3D(0, 0, 0), Normalize(ni));
    }
    //When no intersection, return t = -1
    if (DotProduct(this->ni, eye.npe) >= 0) {
      return make_tuple(Color(0, 0, 0), -1, Point3D(0, 0, 0), Normalize(ni));
    }
    double t = -DotProduct(this->ni, eye.pe - this->pi)/DotProduct(this->ni, eye.npe);
    //ph: hit point
    Point3D ph = eye.pe + eye.npe * t;
    return make_tuple(this->clr, t, ph, ni);
  }
  
  const string GetObjectType(){
    return "Plane";
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
  
  
  const tuple<Color, double, Point3D, Vector3D> CalcIntersect(View eye){
    ////cout << eye.npe.length() << endl;
    //Check if eye position inside the sphere, return -1 when true
    if (DotProduct(eye.pe - this->pi, eye.pe - this->pi) - pow(this->radius, 2) <= 0){
      //cout << "Wrong eye position: Inside of the sphere!" << endl;
      return make_tuple(Color(0, 0, 0), -1, Point3D(0, 0, 0), Vector3D(0, 0, 0));
    }
    
    double B = DotProduct(eye.npe, (this->pi - eye.pe));
    double C = DotProduct(eye.pe - this->pi, eye.pe - this->pi) - pow(this->radius, 2);
    double delta = pow(B, 2) - C;
    if (delta < 0) {   //when no intesection, return t = -1
      return make_tuple(Color(0, 0, 0), -1, Point3D(0, 0, 0), Vector3D(0, 0, 0));
    }else{
      double t = B - sqrt(delta);
      //ph is the hit point
      Point3D ph = eye.pe + eye.npe * t;
      return make_tuple(this->clr, t, ph, Normalize(ph - pi));
    }
  }
  
  const string GetObjectType(){
    return "Sphere";
  }
};


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
class Quadric : public AnyObject{
public:
  Point3D pCenter;
  //a02, a12, a22, a21, a00;
  int aCoor[5];
  //n0, n1, n2
  Vector3D nCoor[3];
  //s0, s1, s2
  double sCoor[3];
  Color clr;
  
  Quadric(){
    //default Quadric is a plane
    aCoor[0] = 0;
    aCoor[1] = 0;
    aCoor[2] = 0;
    aCoor[3] = 1;
    aCoor[4] = 0;
  }
  
  Quadric(int a0, int a1, int a2, int a3, int a4){
    aCoor[0] = a0;
    aCoor[1] = a1;
    aCoor[2] = a2;
    aCoor[3] = a3;
    aCoor[4] = a4;
  }
  
  void SetN(Vector3D n0, Vector3D n1, Vector3D n2) {
    nCoor[0] = n0;
    nCoor[1] = n1;
    nCoor[2] = n2;
  }
  
  void SetS(float s0, float s1, float s2) {
    sCoor[0] = s0;
    sCoor[1] = s1;
    sCoor[2] = s2;
  }
  
  void SetPCenter(Point3D pc_in){
    pCenter = pc_in;
  }
  
  void SetColor(Color clr_in){
    clr = clr_in;
  }
  
  const tuple<Color, double, Point3D, Vector3D> CalcIntersect(View eye){
    //check inside/outside of the object
    double Fp = aCoor[3] / sCoor[2] * DotProduct(nCoor[2], (eye.pe - pCenter)) + aCoor[4];
    for (int i = 0; i < 3; i++) {
      Fp += aCoor[i] * pow(DotProduct(nCoor[i], eye.pe - pCenter) / sCoor[i], 2);
    }
    if (Fp < 0) {
      ////cout << "Wrong eye position: Inside of the Quadric!" << endl;
      return make_tuple(Color(0, 0, 0), -1, Point3D(0, 0, 0), Vector3D(0, 0, 0));
    }
    
    //finish checking, then calculate intersection: t
    double A = 0;
    double B = aCoor[3] / sCoor[2] * DotProduct(nCoor[2], eye.npe);
    double C = aCoor[3] / sCoor[2] * DotProduct(nCoor[2], (eye.pe - pCenter)) + aCoor[4];
    for (int i = 0; i < 3; i++) {
      A += aCoor[i] * pow(DotProduct(nCoor[i], eye.npe) / sCoor[i], 2);
      B += 2 * aCoor[i] / pow(sCoor[i], 2) * DotProduct(nCoor[i], eye.npe) * DotProduct(nCoor[i], eye.pe - pCenter);
      C += aCoor[i] * pow(DotProduct(nCoor[i], eye.pe - pCenter) / sCoor[i], 2);
    }
    
    double t = 0;
    Point3D ph;
    Vector3D nh;
    if (A == 0) {
      if (B != 0) {
        t = -(C / B);
        ph = eye.pe + eye.npe * t;
        nh = nCoor[2] * (aCoor[3] / sCoor[2]);
        for (int i; i < 3; i++) {
          nh = nh + nCoor[i] * (2 * aCoor[i] /pow(sCoor[i], 2) * DotProduct(nCoor[i], ph - pCenter));
        }
        nh = Normalize(nh);
        return make_tuple(clr, t, ph, nh);
      }else{
        return make_tuple(Color(0, 0, 0), -1, Point3D(0, 0, 0), Vector3D(0, 0, 0));
      }
    }else{
      double delta = B * B - 4 * A * C;
      if (delta >= 0) {
        t = (-B - sqrt(delta)) / (2 * A);
        ph = eye.pe + eye.npe * t;
//        //cout << "t: " << t << endl;
//        //cout << "ph: " << ph.x << " : " << ph.y << " : " << ph.z << endl;
        nh = nCoor[2] * (aCoor[3] / sCoor[2]);
//        //cout << "nh_init: " << nh.x << " : " << nh.y << " : " << nh.z << endl;
        for (int i = 0; i < 3; i++) {
          nh = nh + nCoor[i] * (2 * aCoor[i] /pow(sCoor[i], 2) * DotProduct(nCoor[i], ph - pCenter));
//          //cout << "nh[" << i << "]: " << nh.x << " : " << nh.y << " : " << nh.z << endl;
        }
        nh = Normalize(nh);
        
        return make_tuple(clr, t, ph, nh);
      }else{
        return make_tuple(Color(0, 0, 0), -1, Point3D(0, 0, 0), Vector3D(0, 0, 0));
      }
    }
    //end of calculation
  }
  
  const string GetObjectType(){
    return "Quadric";
  }
};


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
class Cone : public Quadric{
public:
  Cone(){
    //default Quadric is a plane
    aCoor[0] = 1;
    aCoor[1] = 1;
    aCoor[2] = -1;
    aCoor[3] = 0;
    aCoor[4] = 0;
  }
  
  const string GetObjectType(){
    return "Cone";
  }
};


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
class Ellipsoid : public Quadric{
public:
  Ellipsoid(){
    //default Quadric is a plane
    aCoor[0] = 1;
    aCoor[1] = 1;
    aCoor[2] = 1;
    aCoor[3] = 0;
    aCoor[4] = -1;
  }
  
  const string GetObjectType(){
    return "Ellipsoid";
  }
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
class Cylinder : public Quadric{
public:
  Cylinder(){
    //default Quadric is a plane
    aCoor[0] = 1;
    aCoor[1] = 1;
    aCoor[2] = 0;
    aCoor[3] = 0;
    aCoor[4] = -1;
  }
  
  const string GetObjectType(){
    return "Cylinder";
  }
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
class EllipticParaboloid : public Quadric{
public:
  EllipticParaboloid(){
    //default Quadric is a plane
    aCoor[0] = 1;
    aCoor[1] = 1;
    aCoor[2] = 0;
    aCoor[3] = -1;
    aCoor[4] = 0;
  }
  
  const string GetObjectType(){
    return "EllipticParaboloid";
  }
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
class HyperbolicParaboloid : public Quadric{
public:
  HyperbolicParaboloid(){
    //default Quadric is a plane
    aCoor[0] = 1;
    aCoor[1] = -1;
    aCoor[2] = 0;
    aCoor[3] = -1;
    aCoor[4] = 0;
  }
  
  const string GetObjectType(){
    return "HyperbolicParaboloid";
  }
};

class light_src{
public:
  // light type, 0 -> directional, 1 -> point, 2 -> spot, 3 -> area light
  int type;
  Point3D position;
  Vector3D direction;
  Vector3D color;
  Vector3D n0 = Vector3D(1, 0, 0);
  Vector3D n1 = Vector3D(0, 1, 0);
  Vector3D n2 = Vector3D(0, 0, 1);
  double sx = 1;
  double sy = 1;
  double spot_angle = 20;
  
  //type 0 / 1, directional or point light
  light_src(int type_in, Point3D position_in, Vector3D direction_in, Vector3D color_in){
    type = type_in;
    position = position_in;
    direction = Normalize(direction_in);
    color = color_in;
  }
  
  //type 2, spot light
  light_src(int type_in, Point3D position_in, Vector3D direction_in, Vector3D color_in, double spot_angle_in){
    type = type_in;
    position = position_in;
    direction = Normalize(direction_in);
    color = color_in;
    spot_angle = spot_angle_in;
  }
  
  //type 3, area light
  light_src(int type_in, Point3D position_in, Vector3D direction_in, Vector3D color_in, Vector3D n0_in, Vector3D n1_in, Vector3D n2_in, double sx_in, double sy_in){
    type = type_in;
    position = position_in;
    direction = Normalize(direction_in);
    color = color_in;
    n0 = n0_in;
    n1 = n1_in;
    n2 = n2_in;
    sx = sx_in;
    sy = sy_in;
  }
  
};

class Shader{
  vector<light_src*> light;
  Color ambient_color;
  Color specular_color;
  Color border_color;
public:
  Shader(vector<light_src*> light_in, Color ambient_color_in, Color specular_color_in, Color border_color_in){
    light = light_in;
    ambient_color = ambient_color_in;
    specular_color = specular_color_in;
    border_color = border_color_in;
  };
  
  Color shading(Color cm0, Point3D ph, Vector3D nh, View eye, bool no_border, vector<AnyObject*> objs){
    int light_num = (int)light.size();
    Color clr_sum = Color(0, 0, 0); // add all shading of different light source
    
    for (int i = 0; i < light_num; i++) {
      Vector3D nhl;  //normal vector from light position to hitpoint
      if (light[i]->type == 0) {
        nhl = light[i]->direction;
      }else if(light[i]->type == 1){
        nhl = Normalize(ph - light[i]->position);
      }else{
        nhl = Normalize(ph - light[i]->position);
      }
      
      Color clr = cm0;
      //clr = Color(0, 0, 0);
      clr = CalcDiffuse(cm0, ph, nh, eye, nhl, i);
      clr = CalcSpecular(clr, ph, nh, eye, nhl, i);
      if (!no_border) {
        clr = CalcBorder(clr, ph, nh, eye, nhl);
      }
      clr = CalcShadow(clr, ph, nh, nhl, i, objs);
      //Since 'Color' class is 'char' typt, never make it larger than 255.
      clr_sum = clr_sum / (i + 1.0) * i + clr / (i + 1.0);
    }
    return clr_sum;
  }
  
  Color CalcDiffuse(Color cm0, Point3D ph, Vector3D nh, View eye, Vector3D nhl, int i){
    double t = (DotProduct(nh, nhl * (-1)) + 1) / 2.0;
    if (light[i]->type == 2) {
#ifdef _hard_spot_light_
      if (DotProduct(light[i]->direction, nhl) < cos(light[i]->spot_angle / 180 * 3.141592653)) {
        t = 0.2;
      }
#else
      t = t * pow((DotProduct(light[i]->direction, nhl) + 1) / 2.0, 1);
#endif

    }
    Color clr = cm0 * (light[i]->color * t) + ambient_color * (Vector3D(1, 1, 1) - light[i]->color * t);
    return clr;
  }
  
  Color CalcSpecular(Color clr, Point3D ph, Vector3D nh, View eye, Vector3D nhl, int i){
    Vector3D reh = eye.npe - nh * (2 * DotProduct(nh, eye.npe));
    reh.normalize();
    double s = DotProduct(nhl * (-1), reh);
    if (SpecularType == 0) {
      s = (s > 0)? s : 0;
      s = pow(s, 25);
    }else if(SpecularType == 1){
      double ks = 0.97;
      s = (s > ks)? s : 0;
    }else{
      if (DotProduct(reh, light[i]->n2) > 0) {
        double x = DotProduct(nhl, light[i]->n0 - light[i]->n2 * DotProduct(reh, light[i]->n0) / DotProduct(reh, light[i]->n2));
        double y = DotProduct(nhl, light[i]->n1 - light[i]->n2 * DotProduct(reh, light[i]->n1) / DotProduct(reh, light[i]->n2));

        s = (max(abs(x)/light[i]->sx, abs(y)/light[i]->sy) < 1)? 0.8 : 0;
        //s = 1 - max(abs(x)/light[i]->sx, abs(y)/light[i]->sy);
        //s = (s > 0)? s : 0;
      }else{
        s = 0;
      }
    }
    
    // If spot light, tune specular color according to angle.
#ifdef _hard_spot_light_
    if (DotProduct(light[i]->direction, nhl) < cos(light[i]->spot_angle / 180 * 3.141592653)) {
      s = s * 0.2;
    }
#else
    if (light[i]->type == 2) {
       s = s * pow((DotProduct(light[i]->direction, nhl) + 1) / 2.0, 1);
    }
#endif
    
    clr = clr * (1 - s) + specular_color * s;
    return clr;
  }
  
  Color CalcBorder(Color clr, Point3D ph, Vector3D nh, View eye, Vector3D nhl){
    double b = 1 - DotProduct(Normalize(eye.pe - ph), nh);
    double min = 0.8;
    double max = 0.9;
    //b = pow(b, 6);
    b = (b - min) / (max - min);
    b = (b > 1)? 1 : ((b < 0)? 0 : b);
    clr = clr * (1 - b) + border_color * b;
    return clr;
  }
  
  
  
  Color CalcShadow(Color clr, Point3D ph, Vector3D nh, Vector3D nhl, int i, vector<AnyObject*> objs){
    double sh = 1;
    Vector3D nlh = nhl * (-1);
    if (DotProduct(nh, nlh) <= 0) {
      //in shadow
      sh = 0.5;
    }else{
      int obj_number = (int)objs.size();
      double t_min = FLT_MAX;
      int obstacle_cnt = 0;
      for(int i = 0; i < obj_number; i++){
        tuple<Color, double, Point3D, Vector3D> intsec = objs[i]->CalcIntersect(View(ph, nlh));
        double t_tmp = get<1>(intsec);
        if (t_tmp >= 0 && t_tmp < t_min) {
          t_min = t_tmp;
          obstacle_cnt ++;
        }
      }
      
      double L = (light[i]->position - ph).length();
      if(t_min > 0 && t_min < L){
        sh = t_min / L;
        ////cout << sh << endl;
      }
    }
    
    //cout << sh << endl;
    if (light[i]->type == 2) {
#ifdef _hard_spot_light_
      if (DotProduct(light[i]->direction, nhl) < cos(light[i]->spot_angle / 180 * 3.141592653)) {
        sh = 0.2;
      }
#else
      sh = sh * pow((DotProduct(light[i]->direction, nhl) + 1) / 2.0, 1);
#endif
    }
    
    //cout << sh << endl;
    clr = clr * sh + ambient_color * (1 - sh);
    return clr;
  }
  
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
Color CalcSubPixel(View eye, vector<AnyObject*> objs, Shader shd){
  //for all objects, find the closest object
  int obj_number = (int)objs.size();
  double t_min = FLT_MAX;
  Color clr = Color(0, 0, 0);
  Point3D ph = Point3D(0, 0, 0);
  Vector3D nh = Vector3D(0, 0, 0);
  Color cm0 = Color(0, 0, 0); //material color
  bool no_border = true;
  for(int i = 0; i < obj_number; i++){
    tuple<Color, double, Point3D, Vector3D> intsec = objs[i]->CalcIntersect(eye);
    double t_tmp = get<1>(intsec);
    if (t_tmp >= 0 && t_tmp < t_min) {
//      /* test code */
//      if (t_min - t_tmp <= 2) {
//        //cout << t_tmp << endl;
//      }
//      /* test end */
      t_min = t_tmp;
      cm0 = get<0>(intsec);
      ph = get<2>(intsec);
      nh = get<3>(intsec);
      no_border = (objs[i]->GetObjectType() == "Plane");
    }
  }
  ////cout << t_min << endl;
  if(t_min != FLT_MAX){
    clr = shd.shading(cm0, ph, nh, eye, no_border, objs);
  }
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
  Vector3D n_view;
  Point3D p_center;
  Vector3D v0;
  Vector3D n0;
  Vector3D n1;
  Point3D p00;
  vector<AnyObject*> objs;
  
  Scene(){
    v_up = Vector3D(0, 0, 1);
    v_view = Vector3D(0, 1, 0);
    p_eye = Point3D(0, -50, 0);
    dist = 5;
    s_x = 10;
    s_y = 10;
    SetCamera();
  }
  
  Scene(Vector3D v_up_in, Vector3D v_view_in, Point3D p_eye_in, double dist_in, double s_x_in, double s_y_in){
    v_up = v_up_in;
    v_view = v_view_in;
    p_eye = p_eye_in;
    dist = dist_in;
    s_x = s_x_in;
    s_y = s_y_in;
    SetCamera();
  }
  
  int obj_number(){
    return (int)objs.size();
  }
  
  void SetCamera(){
    n_view = Normalize(v_view);
    p_center = p_eye + n_view * dist;
    v0 = CrossProduct(v_view, v_up);
    n0 = Normalize(v0);
    n1 = CrossProduct(n0, n_view);
    p00 = p_center - n0 * (s_x / 2) - n1 * (s_y / 2);
  }

  
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
View CalcView(Scene sce, double x_per, double y_per){
  Point3D pp = sce.p00 + sce.n0 * (x_per * sce.s_x) + sce.n1 * (y_per * sce.s_y);
  Vector3D v_pe = pp - sce.p_eye;
  Vector3D npe = Normalize(v_pe);
  ////cout << npe.x << " " << npe.y << " " << npe.z << endl;
  ////cout << npe.length() << endl;
  return View(sce.p_eye, npe);
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
Color CalcPixel(Scene sce, Shader shd, int x, int y, int alias){
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
      Color clr_sub = CalcSubPixel(eye, sce.objs, shd);
      
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
    clr = CalcSubPixel(eye, sce.objs, shd);
  }
  return clr;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
void setPixels(Scene sce, Shader shd, int alias)
{
  Color clr;
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      
      int i = (y * width + x) * 3;
      clr = CalcPixel(sce, shd, x, y, alias);
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
static void windowDisplay()
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
  height = 640;
  int alias = 3;
  Scene sce;
//  sce.p_eye = Point3D(0, -50, 0);
//  sce.v_view = Vector3D(0, 1, 0);
  sce.p_eye = Point3D(0, -50, 0);
  sce.v_view = Vector3D(0, 1, 0);
  sce.v_up = Vector3D(0, 0, 1);
  sce.dist = 8;
  sce.SetCamera();
  
  
  pixmap = new unsigned char[width * height * 3];
  vector<AnyObject*> objs;
  
  AnyObject* plane1 = (AnyObject*)new Plane(Point3D(0, 50, 0), Vector3D(0, -1, 0), Color(100, 92, 72));
  objs.push_back(plane1);
  
  AnyObject* plane2 = (AnyObject*)new Plane(Point3D(0, 0, -20), Vector3D(0, 0, 1), Color(68, 57, 43));
  objs.push_back(plane2);
  
  AnyObject* plane3 = (AnyObject*)new Plane(Point3D(35, 0, 0), Vector3D(-1, 0, 0), Color(117, 52, 43));
  objs.push_back(plane3);
  
  Ellipsoid* ellipsoid1 = new Ellipsoid();
  ellipsoid1->SetN(Vector3D(1, 0, 0), Vector3D(0, 1, 0), Vector3D(0, 0, 1));
  ellipsoid1->SetS(8, 8, 13);
  ellipsoid1->SetPCenter(Point3D(15, 10, -2));
  ellipsoid1->SetColor(Color(53, 71, 14));
  objs.push_back((AnyObject*) ellipsoid1);
  
  Ellipsoid* ellipsoid5 = new Ellipsoid();
  ellipsoid5->SetN(Vector3D(1, 0, 0), Vector3D(0, 1, 0), Vector3D(0, 0, 1));
  ellipsoid5->SetS(3, 3, 5);
  ellipsoid5->SetPCenter(Point3D(15, 10, 14));
  ellipsoid5->SetColor(Color(171, 133, 112));
  objs.push_back((AnyObject*) ellipsoid5);
  
  Ellipsoid* ellipsoid6 = new Ellipsoid();
  ellipsoid6->SetN(Vector3D(1, -0.5, 0), Vector3D(0.5, 1, 0), Vector3D(0, 0, 1));
  ellipsoid6->SetS(8, 2, 8);
  ellipsoid6->SetPCenter(Point3D(17, 9, 14));
  ellipsoid6->SetColor(Color(180, 180, 160));
  objs.push_back((AnyObject*) ellipsoid6);
  
  
  Ellipsoid* ellipsoid3 = new Ellipsoid();
  ellipsoid3->SetN(Vector3D(1, 0, 0), Vector3D(0, 1, 0), Vector3D(0, 0, 1));
  ellipsoid3->SetS(3, 3, 5);
  ellipsoid3->SetPCenter(Point3D(-15, 10, 18));
  ellipsoid3->SetColor(Color(164, 140, 114));
  objs.push_back((AnyObject*) ellipsoid3);
  
  Ellipsoid* ellipsoid4 = new Ellipsoid();
  ellipsoid4->SetN(Vector3D(1, 0, 0), Vector3D(0, 1, 0), Vector3D(0, 0, 1));
  ellipsoid4->SetS(8, 5, 5);
  ellipsoid4->SetPCenter(Point3D(-16, 15, 22));
  ellipsoid4->SetColor(Color(49, 50, 44));
  objs.push_back((AnyObject*) ellipsoid4);
  
  Ellipsoid* ellipsoid2 = new Ellipsoid();
  ellipsoid2->SetN(Vector3D(1, 0, 0), Vector3D(0, 1, 0), Vector3D(0, 0, 1));
  ellipsoid2->SetS(8, 8, 15);
  ellipsoid2->SetPCenter(Point3D(-15, 10, 0));
  ellipsoid2->SetColor(Color(43, 33, 25));
  objs.push_back((AnyObject*) ellipsoid2);

  
  sce.objs = objs;
  
  vector<light_src*> light;
  light_src* light_src1 = new light_src(0, Point3D(0, 0, 0), Vector3D(-1, 1, -1), Vector3D(1, 1, 1));
  light.push_back(light_src1);

  light_src* light_src2 = new light_src(2, Point3D(0, -30, 20), Vector3D(0.2, 1.4, -1), Vector3D( 1, 1, 1), 30);
  //light.push_back(light_src2);
  
  light_src* light_src3 = new light_src(1, Point3D(-20, 0, 50), Vector3D(0, 0, -1), Vector3D( 1, 1, 1), Vector3D(1, 0, 0), Vector3D(0, 1, 0), Vector3D(0, 0, 1), 0.25, 0.25);
  light.push_back(light_src3);
  

  Shader shd = Shader(light, Color(20, 23, 30), Color(220, 200, 250), Color(240, 240, 240));
  
  setPixels(sce, shd, alias);
  glutInit(&argc, argv);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(width, height);
  glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
  glutCreateWindow("Project 3");
  init();
  glutReshapeFunc(windowResize);
  glutDisplayFunc(windowDisplay);
  glutMouseFunc(processMouse);
  glutMainLoop();
  
  return 0;
}