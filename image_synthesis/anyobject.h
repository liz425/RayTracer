//
//  anyobject.cpp
//  image_synthesis
//
//  Created by 李镇 on 3/8/16.
//  Copyright © 2016 zl. All rights reserved.
//


#pragma once
#include <stdio.h>


class AnyObject{
public:
    Color clr;
    virtual const tuple<Color, double, Point3D, Vector3D> CalcIntersect(View eye) = 0;
    virtual const string GetObjectType() = 0;
    //at least 2 texture maps: first <-> the normal map; second <-> darker map
    vector<Texture*> textures;
    //    virtual void SetN(Vector3D n0, Vector3D n1, Vector3D n2) = 0;
    //    virtual void SetS(float s0, float s1, float s2) = 0;
    void setTexture(string texture_file_name){
        Texture* new_texture = new Texture(texture_file_name);
        textures.push_back(new_texture);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
class Plane : public AnyObject{
public:
    Point3D pi;
    Vector3D n0; // n0
    Vector3D n1; // n1
    Vector3D ni; // ni is normal vector of the plane
    Color clr;
    
    Plane(){
        pi = Point3D(0, 0, 0);
        ni = Vector3D(0, 0, 0);
        clr = Color(0, 0, 0);
    }
    
    Plane(Point3D pi_in, Vector3D ni_in, Vector3D n0_in, Color clr_in): pi(pi_in), ni(ni_in), n0(n0_in), clr(clr_in){
        n1 = CrossProduct(n0, ni);
    }
    
    Plane(double pi_x, double pi_y, double pi_z, double ni_x, double ni_y, double ni_z, double n0_x, double n0_y, double n0_z, char r, char g, char b){
        pi = Point3D(pi_x, pi_y, pi_z);
        ni = Vector3D(ni_x, ni_y, ni_z);
        n0 = Vector3D(n0_x, n0_y, n0_z);
        n1 = CrossProduct(n0, ni);
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
    Point3D pCenter;
    double radius;  //radius of sphere
    Color clr;
    Vector3D n0 = Vector3D(0, 1, 0);
    Vector3D n1 = Vector3D(-1, 0, 0);
    Vector3D n2 = Vector3D(0, 0, 1);
    
    Sphere(){
        pCenter = Point3D(0, 0, 0);
        radius = 1;
        clr = Color(0, 0, 0);
    }
    
    Sphere(Point3D pCenter_in, double radius_in, Color clr_in): pCenter(pCenter_in), radius(radius_in), clr(clr_in){}
    
    Sphere(double x_in, double y_in, double z_in, double radius_in, char r_in, char g_in, char b_in){
        pCenter = Point3D(x_in, y_in, z_in);
        radius = radius_in;
        clr = Color(r_in, g_in, b_in);
    }
    
    
    const tuple<Color, double, Point3D, Vector3D> CalcIntersect(View eye){
        ////cout << eye.npe.length() << endl;
        //Check if eye position inside the sphere, return -1 when true
        if (DotProduct(eye.pe - this->pCenter, eye.pe - this->pCenter) - pow(this->radius, 2) <= 0){
            //cout << "Wrong eye position: Inside of the sphere!" << endl;
            return make_tuple(Color(0, 0, 0), -1, Point3D(0, 0, 0), Vector3D(0, 0, 0));
        }
        
        double B = DotProduct(eye.npe, (this->pCenter - eye.pe));
        double C = DotProduct(eye.pe - this->pCenter, eye.pe - this->pCenter) - pow(this->radius, 2);
        double delta = pow(B, 2) - C;
        if (delta < 0) {   //when no intesection, return t = -1
            return make_tuple(Color(0, 0, 0), -1, Point3D(0, 0, 0), Vector3D(0, 0, 0));
        }else{
            double t = B - sqrt(delta);
            //ph is the hit point
            Point3D ph = eye.pe + eye.npe * t;
            return make_tuple(this->clr, t, ph, Normalize(ph - pCenter));
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
