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
Color CalcSubPixel(View eye, vector<AnyObject*> objs, Shader shd){
    //for all objects, find the closest object
    int obj_number = (int)objs.size();
    int object_index = 0;
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
//          /* test code */
//          if (t_min - t_tmp <= 2) {
//          //cout << t_tmp << endl;
//          }
//          /* test end */
            t_min = t_tmp;
            object_index = i;
            cm0 = get<0>(intsec);
            ph = get<2>(intsec);
            nh = get<3>(intsec);
            no_border = (objs[i]->GetObjectType() == "Plane" || objs[i]->GetObjectType() == "Mesh");
        }
    }
  ////cout << t_min << endl;
  if(t_min != FLT_MAX){
    clr = shd.shading(cm0, shd.ambient_color, ph, nh, eye, no_border, objs, object_index);
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
//    Texture tt = Texture("all.bmp");
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      
      int i = (y * width + x) * 3;
      clr = CalcPixel(sce, shd, x, y, alias);
//        clr = tt.GetColorSphere(x / 64 * 2 * PI, y / 48 * PI);
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
  width = 1024;
  height = 1024;
  int alias = 3;
  Scene sce;
    //  sce.p_eye = Point3D(0, -50, 0);
    //  sce.v_view = Vector3D(0, 1, 0);
    sce.p_eye = Point3D(50, -50, 20);
    sce.v_view = Vector3D(-1, 1, 0);
    sce.v_up = Vector3D(0, 0, 1);
    sce.dist = 6;
    sce.s_x = 10;
    sce.s_y = 10;
    sce.SetCamera();
  
  
  pixmap = new unsigned char[width * height * 3];
  vector<AnyObject*> objs;
  
    AnyObject* plane1 = (AnyObject*)new Plane(Point3D(-80, 50, -80), Vector3D(0, -1, 0), Vector3D(-1, 0, 0), Color(214, 147, 44));
    Texture* t0_pln1 = new Texture("fall.bmp", 200, 200);
    Texture* t1_pln1 = new Texture("fall.bmp", 200, 200);
    Texture* t2_pln1 = new Texture("fall.bmp", 200, 200);

    plane1->textures.push_back(t0_pln1);
    plane1->textures.push_back(t1_pln1);
    plane1->textures.push_back(t2_pln1);
    plane1->texture_type = 1;
    objs.push_back(plane1);
  
    
    AnyObject* plane2 = (AnyObject*)new Plane(Point3D(0, 0, -20), Vector3D(0, 0, 1), Vector3D(1, 0, 0), Color(157, 139, 187));
    Texture* t0_pln2 = new Texture("texture0.bmp", 50, 50);
    Texture* t1_pln2 = new Texture("texture1.bmp", 50, 50);
    Texture* t2_pln2 = new Texture("texture0.bmp", 50, 50);

    plane2->textures.push_back(t0_pln2);
    plane2->textures.push_back(t1_pln2);
    plane2->textures.push_back(t2_pln2);
    plane2->texture_type = 1;
    objs.push_back(plane2);
    

  /**
  AnyObject* sphere1 = (AnyObject*)new Sphere(Point3D(0, 0, 20), 12, Color(73, 179, 248));
    Texture* t0_sph1 = new Texture("wall0.bmp", 30, 30, Vector3D(1, 0, 0), Vector3D(0, 0, 1), Vector3D(0, -1, 0), Point3D(0, 0, 20));
    Texture* t1_sph1 = new Texture("wall1.bmp", 30,30, Vector3D(1, 0, 0), Vector3D(0, 0, 1), Vector3D(0, -1, 0), Point3D(0, 0, 20));
    Texture* t2_sph1 = new Texture("wall0.bmp");


    sphere1->textures.push_back(t0_sph1);
    sphere1->textures.push_back(t1_sph1);
    sphere1->textures.push_back(t2_sph1);
    sphere1->texture_type = 3;
    objs.push_back(sphere1);
  */
    

    AnyObject* mesh1 = (AnyObject*)new Mesh("tetrahedron.obj", Point3D(0, -20, 20), 8);
//    AnyObject* mesh1 = (AnyObject*)new Mesh("cube.obj", Point3D(0, -40, 20), 3);
//    AnyObject* mesh1 = (AnyObject*)new Mesh("dodecahandle.obj", Point3D(0, -40, 20), 1);
    
    objs.push_back(mesh1);
    


  
  sce.objs = objs;
  
  vector<light_src*> light;
  light_src* light_src1 = new light_src(1, Point3D(100, -30, 100), Vector3D(-1, 1, -1), Vector3D(1, 1, 1));
  light.push_back(light_src1);

  

  Shader shd = Shader(light, Color(20, 23, 30), Color(220, 200, 250), Color(240, 240, 240));
  
  setPixels(sce, shd, alias);
  glutInit(&argc, argv);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(width, height);
  glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
  glutCreateWindow("Project 5");
  init();
  glutReshapeFunc(windowResize);
  glutDisplayFunc(windowDisplay);
  glutMouseFunc(processMouse);
  glutMainLoop();
  
  return 0;
}