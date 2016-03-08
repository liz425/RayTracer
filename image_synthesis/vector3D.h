//
//  vector3D.cpp
//  image_synthesis
//
//  Created by 李镇 on 3/7/16.
//  Copyright © 2016 zl. All rights reserved.
//

#pragma once

#include <stdio.h>
#include <cmath>
using namespace std;



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

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
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

