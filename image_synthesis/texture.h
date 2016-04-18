//
//  texture.cpp
//  image_synthesis
//
//  Created by 李镇 on 3/7/16.
//  Copyright © 2016 zl. All rights reserved.
//

#pragma once

#include <cstdio>
#include <cstdlib>
#include "color.h"
#include <iostream>
#include "vector3D.h"

#define PI 3.1415927

//#define _DEBUG_

using namespace std;

class Texture{
    unsigned char *pixmap;
public:
    int width = 0;
    int height = 0;
    double sx = 1;
    double sy = 1;
    //following are parameters used only in solid texture
    //using parallel projection
    Vector3D n0 = Vector3D(1, 0, 0);
    Vector3D n1 = Vector3D(0, 1, 0);
    Vector3D n2 = Vector3D(0, 0, 1);
    Point3D p_center;
    Point3D p_projector;
    Point3D p00;
    double dist = 1;
 
    
    
    Texture(string filename, double sx_in = 1, double sy_in = 1, Vector3D n0_in = Vector3D(1, 0, 0), Vector3D n1_in = Vector3D(0, 1, 0), Vector3D n2_in = Vector3D(0, 0, 1), Point3D p_center_in = Point3D(0, 0, 0)): sx(sx_in), sy(sy_in), n0(n0_in), n1(n1_in), n2(n2_in), p_center(p_center_in){
        //parameter initialization
        p_projector = p_center_in - n2 * dist;
        p00 = p_center - n0 * (sx / 2) - n1 * (sy / 2);
        
        
        const char* file_name = filename.c_str();
        FILE* fp = fopen(file_name, "rb");
        if (fp == NULL) {
            perror("file_name");
            throw "Argument Exception";
        }
        unsigned char info[54];
        unsigned char throw_away[84];
        // read the 54-byte header
        fread(info, sizeof(unsigned char), 54, fp);
        //fread(throw_away, sizeof(unsigned char), 84, fp);
        
        // extract image height and width from header
        width = *(int*)&info[18];
        height = *(int*)&info[22];
//        cout << "width: " << width << endl;
//        cout << "height: " << height << endl;
        
        // allocate 3 bytes per pixel, read the rest of the data at once
        int size = 3 * width * height;
        pixmap = new unsigned char[size];
        
        fread(pixmap, sizeof(unsigned char), size, fp);
        fclose(fp);
        
        for(int i = 0; i < size; i += 3)
        {
            unsigned char tmp = pixmap[i];
            pixmap[i] = pixmap[i+2];
            pixmap[i+2] = tmp;
        }
    }
    
    Color GetColorSphere(double theta, double phi){
        double x = theta /(2 * PI) * width / sx;
        double y = phi / PI * height / sy;
        
#ifdef _DEBUG_
        cout << theta << endl;
        cout << phi << endl;
        cout << x << " " << y << endl;
        cout << "//////////" << endl;
#endif

        double u = x - (int)x;
        double v = y - (int)y;
        int pre00 = (int)y % height * width + (int)x % width;
        int pre01 = ((int)y + 1) % height * width + (int)x % width;
        int pre11 = ((int)y + 1) % height * width + ((int)x + 1) % width;
        int pre10 = (int)y % height * width + ((int)x + 1) % width;
        Color clr00 = Color(pixmap[3 * pre00], pixmap[3 * pre00 + 1], pixmap[3 * pre00 + 2]);
        Color clr01 = Color(pixmap[3 * pre01], pixmap[3 * pre01 + 1], pixmap[3 * pre01 + 2]);
        Color clr11 = Color(pixmap[3 * pre11], pixmap[3 * pre11 + 1], pixmap[3 * pre11 + 2]);
        Color clr10 = Color(pixmap[3 * pre10], pixmap[3 * pre10 + 1], pixmap[3 * pre10 + 2]);
        Color clr = clr00 * (1 - u) * (1 - v) + clr01 * (1 - u) * v + clr11 * u * v + clr10 * u * (1 - v);
        return clr;

    }
    
    Color GetColorPlane(double x, double y){
        x = x * width;
        y = y * height;
        double u = x - (int)x;
        double v = y - (int)y;
        int pre00 = (int)y % height * width + (int)x % width;
        int pre01 = ((int)y + 1) % height * width + (int)x % width;
        int pre11 = ((int)y + 1) % height * width + ((int)x + 1) % width;
        int pre10 = (int)y % height * width + ((int)x + 1) % width;
        Color clr00 = Color(pixmap[3 * pre00], pixmap[3 * pre00 + 1], pixmap[3 * pre00 + 2]);
        Color clr01 = Color(pixmap[3 * pre01], pixmap[3 * pre01 + 1], pixmap[3 * pre01 + 2]);
        Color clr11 = Color(pixmap[3 * pre11], pixmap[3 * pre11 + 1], pixmap[3 * pre11 + 2]);
        Color clr10 = Color(pixmap[3 * pre10], pixmap[3 * pre10 + 1], pixmap[3 * pre10 + 2]);
        Color clr = clr00 * (1 - u) * (1 - v) + clr01 * (1 - u) * v + clr11 * u * v + clr10 * u * (1 - v);
        return clr;
    }
    
    Color GetJuliaSet(double x, double y){
        double u0 = -0.75;
        double v0 = 0;
        double T = 0.5;
        for (int i = 0; i < 5; i ++) {
            double x_swap = x * x - y * y + u0;
            y = 2 * x * y + v0;
            x = x_swap;
        }
//        cout << sqrt(x * x + y * y) << endl;
        if (x * x + y * y - T * T > 0) {
            return Color(0xff0000);
        }else{
            return Color(0x00ff00);
        }
    }
    
    Color Get3DJulia(Point3D ph, Point3D p_center){        
        double a = 0;
        Vector3D v = (ph - p_center) / 12.0;
//        cout << v.length() << endl;
        
        double a0 = 1;
        Vector3D v0 = Vector3D(1, 0, 0);
        double T = 10;
        
        for (int i = 0; i < 5; i ++) {
            double a_swap = a * a - DotProduct(v, v) + a0;
            v = v * 2 * a + v0;
            a = a_swap;
        }
               // cout << sqrt(a + v.length() - T) << endl;
        
        if (a + v.length() - T > 0) {
            return Color(0xff0000);
        }else{
            return Color(0x00ff00);
        }
    }
    
    Color Get3DFunction(Point3D ph, Point3D p_center){
        Vector3D v = ph - p_center;
//        cout << v.length() << endl;
        double f = CrossProduct(Vector3D(v.x, 0, v.z), Vector3D(1, 0, 0)).length() + DotProduct(Vector3D(v.x, 0, v.z), Vector3D(1, 0, 0));
        f = sin(f * PI * 0.3);
        double T = 0;
        
        
        if (f - T > 0) {
            return Color(250, 235, 190);
        }else{
            return Color(90, 190, 250);
        }
    }
};