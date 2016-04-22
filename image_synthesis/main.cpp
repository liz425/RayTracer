//
//  main.cpp
//  image_synthesis
//
//  Created by 李镇 on 1/22/16.
//  Copyright © 2016 zl. All rights reserved.


#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <GLUT/GLUT.h>
#include <cmath>
#include <vector>
#include <cfloat>

#include "vector3D.h"
#include "color.h"
#include "texture.h"
#include "shader.h"
#include "anyobject.h"

#ifndef PI
#define PI 3.14159265
#endif

#define REFLECTION_TIMES_THRES 3
#define REFLECTION_KTOTAL_THRES 0.05


using namespace std;

int width, height;
unsigned char *pixmap;




//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
Color CalcSubPixel(View eye, vector<AnyObject*>& objs, Shader shd, int reflection_times, double reflection_ktotal, int outside, int object_index = 0, double m = 0, double n = 0){
    //for all objects, find the closest object
    int obj_number = (int)objs.size();
    double t_min = FLT_MAX;
    Color clr = Color(0, 0, 0);
    Point3D ph = Point3D(0, 0, 0);
    Vector3D nh = Vector3D(0, 0, 0);
    Color cm0 = Color(0, 0, 0); //material color
    bool no_border = true;
    Vector2D UV = Vector2D(0, 0);
    
    if(outside <= -1 && object_index >= 0){
        //cout << object_index << " " << outside << endl;
        tuple<Color, double, Point3D, Vector3D, int> intsec = objs[object_index]->CalcIntersect(eye, outside);
        t_min = get<1>(intsec);
        cm0 = get<0>(intsec);
        ph = get<2>(intsec);
        nh = get<3>(intsec);
        no_border = (objs[object_index]->GetObjectType() == "Plane" || objs[object_index]->GetObjectType() == "Mesh");
        if(objs[object_index]->GetObjectType() == "Mesh"){
            UV = ((Mesh*)objs[object_index])->GetUV();
        }
    }else{
        for(int i = 0; i < obj_number; i++){
            tuple<Color, double, Point3D, Vector3D, int> intsec = objs[i]->CalcIntersect(eye);
            double t_tmp = get<1>(intsec);
            if (t_tmp >= 0 && t_tmp < t_min) {
                t_min = t_tmp;
                object_index = i;
                cm0 = get<0>(intsec);
                ph = get<2>(intsec);
                nh = get<3>(intsec);
                no_border = (objs[i]->GetObjectType() == "Plane" || objs[i]->GetObjectType() == "Mesh");
                if(objs[i]->GetObjectType() == "Mesh"){
                    UV = ((Mesh*)objs[i])->GetUV();
                }
            }
        }
    }
    
    ////cout << t_min << endl;
    if(t_min != FLT_MAX){
        //If we using normal map, then normal vector 'nh' may change due to normal map
        //Thus shading function should also update 'nh' by using pass by reference: Vector3D& nh
        clr = shd.shading(cm0, shd.ambient_color, ph, nh, eye, no_border, objs, object_index, UV, m, n);
    }
    
    //calculate reflection
    double ks = objs[object_index]->ks;
    
    
    reflection_ktotal *= ks;
    if(outside >= 1){
        reflection_times++;
    }
    Color clr_reflection;
    Color clr_refraction;
    if(outside >= 1 && reflection_times <= REFLECTION_TIMES_THRES && reflection_ktotal >= REFLECTION_KTOTAL_THRES){
        Vector3D reflect_npe = eye.npe - nh * 2 * DotProduct(nh, eye.npe);
        // Glossy
        //reflect_npe = reflect_npe + Vector3D((double)rand() / RAND_MAX - 0.5, (double)rand() / RAND_MAX - 0.5, (double)rand() / RAND_MAX - 0.5) * 0.1;
        //reflect_npe = reflect_npe.normalize();
        View reflect_view = View(ph, reflect_npe);
        clr_reflection = clr * (1 - ks) + CalcSubPixel(reflect_view, objs, shd, reflection_times, reflection_ktotal, 1) * ks;
    }else{
        clr_reflection = clr;
    }
    
    //claculate refraction
    
    double fresnel = objs[object_index]->fresnel;
    if(fresnel < 1){
        if(outside >= 1){
            Vector3D V = eye.npe * (-1);
            double c = DotProduct(nh, V);
            double IOR = 1 / objs[object_index]->IOR; //relative n = n1/n2, n1 is IOR of air, = 1
//            double IOR = 1 / (1.1 + clr.r / 2000.0);
            Vector3D T = V * (-IOR) + nh * (IOR * c - sqrt(pow(IOR * c, 2) - IOR * IOR + 1));
            T = T.normalize();
            View refract_view = View(ph, T);
            clr_refraction = CalcSubPixel(refract_view, objs, shd, reflection_times, reflection_ktotal, -1, object_index);
            clr = clr_reflection * fresnel + clr_refraction * (1 - fresnel);
        }else{
            Vector3D V = eye.npe * (-1);
            double c = DotProduct(nh, V);
            double IOR = objs[object_index]->IOR;
//            double IOR = 1.1 + clr.r / 2000.0;
            double delta = (pow(IOR * c, 2) - IOR * IOR + 1);
            if(delta > 0){
                Vector3D T = V * (-IOR) + nh * (IOR * c - sqrt(delta));
                T = T.normalize();
                View refract_view = View(ph, T);
                clr_refraction = CalcSubPixel(refract_view, objs, shd, reflection_times, reflection_ktotal, 1, object_index);
            }else{
                //full reflection
                Vector3D reflect_npe;
                reflect_npe = eye.npe - nh * 2 * DotProduct(nh, eye.npe);
                View reflect_view = View(ph, reflect_npe);
                if(outside-- <= -5){
                    outside = 1;
                }
                //full reflection
                clr_refraction = CalcSubPixel(reflect_view, objs, shd, reflection_times, reflection_ktotal, outside, object_index);
            }
            clr = clr_refraction;
        }
    }else{
        clr = clr_reflection;
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
    int k0 = (double)rand() / RAND_MAX * 100;
    int k1 = (double)rand() / RAND_MAX * 100;
    double r = (double)rand() / RAND_MAX;
  for (int q = 0; q < alias; q++){
      
      //motion blur
      //Sphere* sphere = (Sphere*) sce.objs[0];
      //sphere->pCenter = Point3D(0, -20, 10) + Vector3D(q * 3.0 / alias, 0, 0);
   
      
      for (int p = 0; p < alias; p++){
          double pointX = ((double)rand() / RAND_MAX + p) / alias + x;
          double pointY = ((double)rand() / RAND_MAX + q) / alias + y;
          
          double x_per = pointX / width;
          double y_per = pointY / height;
          
          //Lost focus
          //sce.p_eye = sce.p_eye + Vector3D((double)rand() / RAND_MAX - 0.5, (double)rand() / RAND_MAX - 0.5, 0) * 0.5;
        
          
          
          View eye = CalcView(sce, x_per, y_per);
          
          
          //parameters for soft shadow
          double m = ((p + k0) % alias + r) * 1.0 / alias;
          double n = ((q + k1) % alias + r) * 1.0 / alias;
          
          
          
          Color clr_sub = CalcSubPixel(eye, sce.objs, shd, 0, 1.0, 1, m, n);
          
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
    clr = CalcSubPixel(eye, sce.objs, shd, 0, 1.0, 1);
  }
  return clr;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
void setPixels(Scene sce, Shader shd, int alias, int index){
    Color clr;
    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            int i = (y * width + x) * 3;
            clr = CalcPixel(sce, shd, x, y, alias);
            pixmap[i++] = clr.r;
            pixmap[i++] = clr.g;
            pixmap[i]   = clr.b;
        }
        
        if(y % (height / 10) == 0){
            cout << "Pic " << index << ": " << (y * 100 / height) << "%\n" << endl;
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
  //if (state == GLUT_UP)
  //  exit(0);               // Exit on mouse click.
}

static void init(void)
{
  glClearColor(1, 1, 1, 1); // Set background color.
}


// The SaveImage function came from Adam Arron
void SaveImage(const char *file, int width, int height, int dpi, unsigned char *pixmap) {
    FILE *f;
    int totalPixels = width * height;
    int stuff = 4 * totalPixels;
    
    int fileSize = 54 + stuff;
    
    float factor = 39.375;
    int m = static_cast<int>(factor);
    
    int ppm = dpi * m;
    unsigned char bmpfileheader[14] = { 'B', 'M',  0, 0, 0, 0,  0, 0, 0, 0,  54, 0, 0, 0 };
    unsigned char bmpinfoheader[40] = { 40, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 24, 0 };
    
    bmpfileheader[2] = (unsigned char)(fileSize);
    bmpfileheader[3] = (unsigned char)(fileSize>>8);
    bmpfileheader[4] = (unsigned char)(fileSize>>16);
    bmpfileheader[5] = (unsigned char)(fileSize>>24);
    
    bmpinfoheader[4] = (unsigned char)(width);
    bmpinfoheader[5] = (unsigned char)(width>>8);
    bmpinfoheader[6] = (unsigned char)(width>>16);
    bmpinfoheader[7] = (unsigned char)(width>>24);
    
    bmpinfoheader[8] = (unsigned char)(height);
    bmpinfoheader[9] = (unsigned char)(height >> 8);
    bmpinfoheader[10] = (unsigned char)(height >> 16);
    bmpinfoheader[11] = (unsigned char)(height >> 24);
    
    bmpinfoheader[21] = (unsigned char)(stuff);
    bmpinfoheader[22] = (unsigned char)(stuff >> 8);
    bmpinfoheader[23] = (unsigned char)(stuff >> 16);
    bmpinfoheader[24] = (unsigned char)(stuff >> 24);
    
    bmpinfoheader[25] = (unsigned char)(ppm);
    bmpinfoheader[26] = (unsigned char)(ppm >> 8);
    bmpinfoheader[27] = (unsigned char)(ppm >> 16);
    bmpinfoheader[28] = (unsigned char)(ppm >> 24);
    
    bmpinfoheader[29] = (unsigned char)(ppm);
    bmpinfoheader[30] = (unsigned char)(ppm >> 8);
    bmpinfoheader[31] = (unsigned char)(ppm >> 16);
    bmpinfoheader[32] = (unsigned char)(ppm >> 24);
    
    f = fopen(file, "wb");
    
    fwrite(bmpfileheader, 1, 14, f);
    fwrite(bmpinfoheader, 1, 40, f);
    for (int i = 0; i < totalPixels; i++)
    {
        
        unsigned char red = (pixmap[i * 3]);
        unsigned char green = (pixmap[i * 3 + 1]);
        unsigned char blue = (pixmap[i * 3 + 2]);
        
        unsigned char color[3] = {blue, green, red};
        
        fwrite(color, 1, 3, f);
    }
    fclose(f);
    //sleep(1);
}




int main(int argc, char *argv[])
{

    
    width = 1920;
    height = 1080;
    int alias = 2;
    Scene sce;
    //  sce.p_eye = Point3D(0, -50, 0);
    //  sce.v_view = Vector3D(0, 1, 0);
    sce.p_eye = Point3D(0, -50, 20);
    sce.v_view = Vector3D(0, 1, 0);
    sce.v_up = Vector3D(0, 0, 1);
//    sce.v_view = Vector3D(0, 0, -1);
//    sce.v_up = Vector3D(1, 0, 0);
    
    sce.dist =25;
    sce.s_x = sce.dist * 4.0 * width / height;
    sce.s_y = sce.dist * 4;
    sce.SetCamera();
  
  
    pixmap = new unsigned char[width * height * 3];
    vector<AnyObject*> objs;
  
    AnyObject* plane1 = (AnyObject*)new Plane(Point3D(-80, 100, -80), Vector3D(0, -1, 0), Vector3D(-1, 0, 0), Color(214, 147, 44), 0);
    plane1->AddTexture("fall.bmp", 200, 200);
    plane1->AddTexture("fall.bmp", 200, 200);
    plane1->texture_type = 0;
    //objs.push_back(plane1);
  
    
    AnyObject* plane2 = (AnyObject*)new Plane(Point3D(0, 0, 0), Vector3D(0, 0, 1), Vector3D(1, 0, 0), Color(246,246,246), 0);
    plane2->AddTexture("fall.bmp", 10, 10);
    plane2->AddTexture("fall.bmp", 10, 10);
    plane2->texture_type = 0;
    //objs.push_back(plane2);
    

  
    AnyObject* sphere1 = (AnyObject*)new Sphere(Point3D(0, -20, 10), 10, Color(73, 179, 248), Vector3D(1, 0, 0), Vector3D(0, 1, 0), Vector3D(0, 0, 1), 1);
    sphere1->fresnel = 1;
    sphere1->IOR = 1.1;
    sphere1->AddTexture("wall0.bmp", 0.5, 0.5);
    sphere1->AddTexture("wall0.bmp", 0.5, 0.5);
    //sphere1->AddTexture("normal.bmp", 1, 1);
    sphere1->texture_type = 0;
    objs.push_back(sphere1);
  
    
    AnyObject* sphere2 = (AnyObject*)new Sphere(Point3D(0, 0, 20), 10, Color(73, 179, 248), Vector3D(1, 0, 0), Vector3D(0, 1, 0), Vector3D(0, 0, 1), 1);
    sphere2->AddTexture("wall0.bmp", 0.5, 0.5);
    sphere2->AddTexture("wall0.bmp", 0.5, 0.5);
    sphere2->texture_type = 3;
    //objs.push_back(sphere2);
    
    AnyObject* sphere3 = (AnyObject*)new Sphere(Point3D(-20, -20, 20), 10, Color(73, 179, 248), Vector3D(1, 0, 0), Vector3D(0, 1, 0), Vector3D(0, 0, 1), 1);
    sphere3->AddTexture("wall0.bmp", 0.5, 0.5);
    sphere3->AddTexture("wall0.bmp", 0.5, 0.5);
    sphere3->texture_type = 3;
    //objs.push_back(sphere3);
    
    
    
    AnyObject* environment = (AnyObject*)new Environment(Point3D(0, -50, 20), 1000, Color(255, 255, 0), Vector3D(-1, 1, 0), Vector3D(-1, -1, 0), Vector3D(0, 0, 1), 0);
    environment->AddTexture("360.bmp", 1, 1);
    environment->AddTexture("360.bmp", 1, 1);
    environment->texture_type = 1;
    objs.push_back(environment);
    
    
    
    

//    AnyObject* mesh1 = (AnyObject*)new Mesh("tetrahedron.obj", Point3D(0, -20, 20), 8);
    AnyObject* mesh1 = (AnyObject*)new Mesh("cube.obj", Point3D(-20, -90, 20), 5, 1);
    mesh1->fresnel = 0;
    mesh1->IOR = 1.1;
    mesh1->AddTexture("star0.bmp", 0.1, 0.1);
    mesh1->AddTexture("star1.bmp", 0.1, 0.1);
    mesh1->AddTexture("normal.bmp");
    mesh1->texture_type = 1;
    //objs.push_back(mesh1);
    
    AnyObject* mesh2 = (AnyObject*)new Mesh("dodecahandle.obj", Point3D(-20, -30, 20), 3, 1);
    mesh2->AddTexture("star0.bmp", 1, 1);
    mesh2->AddTexture("star0.bmp", 1, 1);
    mesh2->texture_type = 1;
    //objs.push_back(mesh2);

  
    sce.objs = objs;

    vector<light_src*> light;
    light_src* light_src1 = new light_src(0, Point3D(-100, -30, 100), Vector3D(-1, 1, -1), Vector3D(1, 1, 1));
    light_src* light_src2 = new light_src(3, Point3D(30, -30, 50), Vector3D(0, 0, -1), Vector3D(1, 1, 1), Vector3D(1,0,0), Vector3D(0,1,0), Vector3D(0,0,-1), 30, 30);
    light.push_back(light_src2);

  

//  Shader shd = Shader(light, Color(20, 23, 30), Color(220, 200, 250), Color(240, 240, 240));
    Shader shd = Shader(light, Color(0x000000), Color(0xdddddd), Color(0x000000));
  
    for (int i = 0; i < 50; i++) {
        Sphere* tmp = (Sphere*) sphere1;
        double theta = 2 * PI / 50.0 * i;
        tmp->pCenter = tmp->pCenter + Vector3D(sin(theta), cos(theta), 0);
        setPixels(sce, shd, alias, i + 1);
        string num = to_string(i);
        num = "00000" + num;
        num = num.substr((int)num.size() - 5, 5);
        string file = "video/" + num + ".bmp";
//        string file = to_string(i);
        SaveImage(file.c_str(), width, height, 72, pixmap);
    }
    
    
  glutInit(&argc, argv);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(width, height);
  glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
  glutCreateWindow("Project 8");
  init();
  glutReshapeFunc(windowResize);
  glutDisplayFunc(windowDisplay);
  glutMouseFunc(processMouse);
  glutMainLoop();
  
  return 0;
}