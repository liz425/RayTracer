//
//  anyobject.cpp
//  image_synthesis
//
//  Created by 李镇 on 3/8/16.
//  Copyright © 2016 zl. All rights reserved.
//


#pragma once
#include <iostream>     // std::cout
#include <fstream>      // std::fstream
#include <sstream>      // std::istringstream
#include <string>       // std::string


class AnyObject{
public:
    Color clr;
    virtual const tuple<Color, double, Point3D, Vector3D> CalcIntersect(View eye) = 0;
    virtual const string GetObjectType() = 0;
    //at least 2 texture maps: first <-> the normal map; second <-> darker map
    vector<Texture*> textures;
    //type: 0 No texture mapping    type: 1  texture mapping     type: 2 solid texture(image projecting)
    //type: 3 3D Julia Set
    int texture_type = 0;
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
    Color clr;
    Point3D pi;
    Vector3D n0; // n0
    Vector3D n1; // n1
    Vector3D ni; // ni is normal vector of the plane
    
    
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
    Color clr;
    Point3D pCenter;
    double radius;  //radius of sphere
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

class Triangle : public AnyObject{
public:
    Color clr = Color(150, 200, 110);
    Point3D p_center;
    Point3D p0, p1, p2;
    Vector3D normal, normal_p0, normal_p1, normal_p2;
    
    
    Triangle(Point3D p0_in = Point3D(0, 0, 0), Point3D p1_in = Point3D(1, 0, 0), Point3D p2_in = Point3D(0, 1, 0), Vector3D n0 = Vector3D(1, 0, 0), Vector3D n1 = Vector3D(1, 0, 0), Vector3D n2 = Vector3D(1, 0, 0)):p0(p0_in), p1(p1_in), p2(p2_in), normal_p0(n0), normal_p1(n1), normal_p2(n2){
        normal_p0.normalize();
        normal_p1.normalize();
        normal_p2.normalize();
        normal = CrossProduct(p1 - p0, p2 - p0);
        normal.normalize();
        p_center = Point3D((p0.x + p1.x + p2.x) / 3.0, (p0.y + p1.y + p2.y) / 3.0, (p0.z + p1.z + p2.z) / 3.0);
    }
    
    const tuple<Color, double, Point3D, Vector3D> CalcIntersect(View eye){
        //check if eye position behind the plane
        if (DotProduct(this->normal, eye.pe - this->p_center) <= 0) {
            //cout << "Wrong eye position: Behind the plane!" << endl;
            return make_tuple(Color(0, 0, 0), -1, Point3D(0, 0, 0), Normalize(normal));
        }
        
        //When no intersection with the plane, return t = -1
        if (DotProduct(this->normal, eye.npe) >= 0) {
            return make_tuple(Color(0, 0, 0), -1, Point3D(0, 0, 0), Normalize(normal));
        }
        
        //Has intersection with the plane, then judge whether inside the triangle
        double t = -DotProduct(this->normal, eye.pe - this->p_center)/DotProduct(this->normal, eye.npe);
        
        //ph: hit point
        Point3D ph = eye.pe + eye.npe * t;
        
        //judge whether ph is inside triangle
        Vector3D AA = CrossProduct(p1 - p0, p2 - p0);
        //Vector3D A0 = CrossProduct(p1 - ph, p2 - ph);
        Vector3D A1 = CrossProduct(p2 - ph, p0 - ph);
        Vector3D A2 = CrossProduct(p0 - ph, p1 - ph);
        
        double u = (AA.x != 0)? A1.x / AA.x : ((AA.y != 0)? A1.y / AA.y : A1.z / AA.z);
        double v = (AA.x != 0)? A2.x / AA.x : ((AA.y != 0)? A2.y / AA.y : A2.z / AA.z);
        
        if(u >= 0 && v >= 0 && (u + v) <= 1){
            //Inside triangle
            return make_tuple(this->clr, t, ph, normal);
        }else{
            //When outside the triangle, return t = -1
            return make_tuple(Color(0, 0, 0), -1, Point3D(0, 0, 0), Normalize(normal));
        }
    }
    
    const string GetObjectType(){
        return "Triangle";
    }

};

class Mesh : public AnyObject{
public:
    Color clr = Color(150, 200, 110);
    vector<Point3D> vertices;
    vector<Vector3D> normals;
    vector<Triangle> triangles;
    
    //Open file and set triangles
    Mesh(string filePath, Point3D mesh_origin = Point3D(0, 0, 0), double scale = 1){
        const char* file_name = filePath.c_str();
        ifstream ifs;
        ifs.open(file_name);
        if(ifs.fail()){
            cout << "Can't open OBJ file: " << filePath << endl;
            return;
        }
        
        //Start reading file
        cout << "File reading: " << filePath << endl;
        string line;
        
        //different OBJ file format has different face parameters
        //set default as 1
        
        int face_type = 1;
        bool has_normal = false;
        
        while(ifs.peek() != EOF){
            getline(ifs, line);
            
            if(line.substr(0,2) == "v "){
                //check v for vertices
                istringstream v(line.substr(2));
                double x, y, z;
                v >> x; v >> y; v >> z;
                Point3D vertex = mesh_origin + (Point3D(x, y, z) - Point3D(0, 0, 0))* scale;
                vertices.push_back(vertex);
            }else if(line.substr(0,2) == "vt"){
                //check vt for texture co-ordinate
                continue;
            }else if(line.substr(0,2) == "vn"){
                has_normal = true;
                //check vn for vertex normal
                istringstream v(line.substr(2));
                double x, y, z;
                v >> x; v >> y; v >> z;
                Vector3D normal = Vector3D(x, y, z);
                normals.push_back(normal);
            }else if(line.substr(0,2) == "f "){
                //check f for faces
                const char* chh=line.c_str();
                int a = 1, b = 1, c = 1;    //to store vertex index, start from 1
                int p = 1, q = 1, r = 1;    //to store texture index, start from 1
                int l = 1, m = 1, n = 1;    //to store normal index, start from 1
                
                int para_cnt = 0;
                if(face_type == 1){
                    para_cnt = sscanf(chh = line.c_str(), "f %i %i %i", &a, &b, &c);
                    if(para_cnt != 3){
                        face_type = 2;
                    }
                }
                
                if(face_type == 2){
                    para_cnt = sscanf(chh = line.c_str(), "f %i/%i %i/%i %i/%i", &a, &p, &b, &q, &c, &r);
                    if(para_cnt != 6){
                        face_type = 3;
                    }
                }
                
                if(face_type == 3){
                    para_cnt = sscanf(chh = line.c_str(), "f %i/%i/%i %i/%i/%i %i/%i/%i", &a, &p, &l, &b, &q, &m, &c, &r, &n);
                    if(para_cnt != 9){
                        cout << "Can't read OBJ file correnctly: " << filePath << endl;
                        return;
                    }
                }
                
                //index start form 1, should convert to start from 0
                Triangle tri;
                
                if(has_normal){
                    tri = Triangle(vertices[a - 1], vertices[b - 1], vertices[c - 1], normals[l - 1], normals[m - 1], normals[n - 1]);
                }else{
                    tri = Triangle(vertices[a - 1], vertices[b - 1], vertices[c - 1]);
                }
                
                tri.clr = this->clr;
                triangles.push_back(tri);
            }else{
                //start with '#', comments, skip
                continue;
            }
        }
        cout << "OBJ file reading finish." << endl;
        
    }
    
    const tuple<Color, double, Point3D, Vector3D> CalcIntersect(View eye){
        double t_min = FLT_MAX;
        tuple<Color, double, Point3D, Vector3D> intsec_min;
        for(Triangle tri : triangles){
            tuple<Color, double, Point3D, Vector3D> intsec = tri.CalcIntersect(eye);
            double t_tmp = get<1>(intsec);
            if (t_tmp >= 0 && t_tmp < t_min) {
                t_min = t_tmp;
                intsec_min = intsec;
            }
        }
        
        if(t_min > 0 && t_min != FLT_MAX){
            return intsec_min;
        }else{
            return make_tuple(Color(0, 0, 0), -1, Point3D(0, 0, 0), Normalize(Vector3D(0, 0, 0)));
        }
    }
    
    const string GetObjectType(){
        return "Mesh";
    }
};
