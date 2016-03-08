//
//  Color.cpp
//  image_synthesis
//
//  Created by 李镇 on 3/7/16.
//  Copyright © 2016 zl. All rights reserved.
//

#pragma once

#include <stdio.h>
#include "vector3D.h"
using namespace std;


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
 
