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

#include "vector3D.h"
#include "color.h"
#include "texture.h"
#include "shader.h"
#include "anyobject.h"

#ifndef PI
#define PI 3.14159265
#endif

#define REFLECTION_TIMES_THRES 0
#define REFLECTION_KTOTAL_THRES 0.05


using namespace std;

int width, height;
unsigned char *pixmap;



//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
Color CalcSubPixel(View eye, vector<AnyObject*>& objs, Shader shd, int reflection_times, double reflection_ktotal){
    //for all objects, find the closest object
    int obj_number = (int)objs.size();
    int object_index = 0;
    double t_min = FLT_MAX;
    Color clr = Color(0, 0, 0);
    Point3D ph = Point3D(0, 0, 0);
    Vector3D nh = Vector3D(0, 0, 0);
    Color cm0 = Color(0, 0, 0); //material color
    bool no_border = true;
    Vector2D UV = Vector2D(0, 0);
    
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
    ////cout << t_min << endl;
    if(t_min != FLT_MAX){
        //If we using normal map, then normal vector 'nh' may change due to normal map
        //Thus shading function should also update 'nh' by using pass by reference: Vector3D& nh
        clr = shd.shading(cm0, shd.ambient_color, ph, nh, eye, no_border, objs, object_index, UV);
    }
    
    //calculate reflection
    double ks = objs[object_index]->ks;
    
//    if(objs[object_index]->GetObjectType() == "Sphere"){
//        cout << ks << endl;
//    }
    
    reflection_ktotal *= ks;
    reflection_times++;
    if(reflection_times <= REFLECTION_TIMES_THRES && reflection_ktotal >= REFLECTION_KTOTAL_THRES){
        Vector3D reflect_npe = eye.npe - nh * 2 * DotProduct(nh, eye.npe);
        View reflect_view = View(ph, reflect_npe);
        return clr * (1 - ks) + CalcSubPixel(reflect_view, objs, shd, reflection_times, reflection_ktotal) * ks;
    }else{
        return clr;
    }
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
      Color clr_sub = CalcSubPixel(eye, sce.objs, shd, 0, 1.0);
      
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
    clr = CalcSubPixel(eye, sce.objs, shd, 0, 1.0);
  }
  return clr;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
void setPixels(Scene sce, Shader shd, int alias){
    Color clr;
    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            int i = (y * width + x) * 3;
            clr = CalcPixel(sce, shd, x, y, alias);
            pixmap[i++] = clr.r;
            pixmap[i++] = clr.g;
            pixmap[i]   = clr.b;
        }
        
        if(y % (height / 100) == 0){
            cout << (y * 100 / height) << "%\n" << endl;
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



int main(int argc, char *argv[])
{
    width = 600;
    height = 600;
    int alias = 3;
    Scene sce;
    //  sce.p_eye = Point3D(0, -50, 0);
    //  sce.v_view = Vector3D(0, 1, 0);
    sce.p_eye = Point3D(-50, -50, 20);
    sce.v_view = Vector3D(1, 1, 0);
    sce.v_up = Vector3D(0, 0, 1);
//    sce.v_view = Vector3D(0, 0, -1);
//    sce.v_up = Vector3D(1, 0, 0);
    
    sce.dist =5;
    sce.s_x = 10;
    sce.s_y = 10;
    sce.SetCamera();
  
  
    pixmap = new unsigned char[width * height * 3];
    vector<AnyObject*> objs;
  
    AnyObject* plane1 = (AnyObject*)new Plane(Point3D(-80, 50, -80), Vector3D(0, -1, 0), Vector3D(-1, 0, 0), Color(214, 147, 44), 0);
    plane1->AddTexture("fall.bmp", 200, 200);
    plane1->AddTexture("fall.bmp", 200, 200);
    plane1->texture_type = 1;
    //objs.push_back(plane1);
  
    
    AnyObject* plane2 = (AnyObject*)new Plane(Point3D(0, 0, -20), Vector3D(0, 0, 1), Vector3D(1, 0, 0), Color(157, 139, 187), 1);
    plane2->AddTexture("texture0.bmp", 50, 50);
    plane2->AddTexture("texture1.bmp", 50, 50);
    plane2->texture_type = 1;
    //objs.push_back(plane2);
    

  
    AnyObject* sphere1 = (AnyObject*)new Sphere(Point3D(-20, -30, 0), 12, Color(73, 179, 248), Vector3D(1, 0, 0), Vector3D(0, 1, 0), Vector3D(0, 0, 1), 1);
    sphere1->AddTexture("wall0.bmp", 1, 1);
    sphere1->AddTexture("wall0.bmp", 1, 1);
    //sphere1->AddTexture("normal.bmp", 1, 1);
    sphere1->texture_type = 1;
    objs.push_back(sphere1);
  
    
    
    AnyObject* environment = (AnyObject*)new Environment(Point3D(0, -50, 20), 1000, Color(255, 255, 0), Vector3D(0, 1, 0), Vector3D(-1, 0, 0), Vector3D(0, 0, 1), 0);
    environment->AddTexture("360.bmp", 1, 1);
    environment->AddTexture("360.bmp", 1, 1);
    environment->texture_type = 1;
    objs.push_back(environment);
    
    
    
    

//    AnyObject* mesh1 = (AnyObject*)new Mesh("tetrahedron.obj", Point3D(0, -20, 20), 8);
    AnyObject* mesh1 = (AnyObject*)new Mesh("cube.obj", Point3D(-20, -50, 0), 5, 1);
    mesh1->AddTexture("star0.bmp", 0.1, 0.1);
    mesh1->AddTexture("star1.bmp", 0.1, 0.1);
    //mesh1->AddTexture("normal.bmp");
    mesh1->texture_type = 1;
    objs.push_back(mesh1);
    
    AnyObject* mesh2 = (AnyObject*)new Mesh("dodecahandle.obj", Point3D(-20, -30, 20), 3, 1);
    mesh2->AddTexture("star0.bmp", 1, 1);
    mesh2->AddTexture("star0.bmp", 1, 1);
    mesh2->texture_type = 1;
    //objs.push_back(mesh2);

  
  sce.objs = objs;
  
  vector<light_src*> light;
  light_src* light_src1 = new light_src(1, Point3D(-100, -30, 100), Vector3D(-1, 1, -1), Vector3D(1, 1, 1));
  light.push_back(light_src1);

  

//  Shader shd = Shader(light, Color(20, 23, 30), Color(220, 200, 250), Color(240, 240, 240));
    Shader shd = Shader(light, Color(0x000000), Color(0xdddddd), Color(0x000000));
  
  setPixels(sce, shd, alias);
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