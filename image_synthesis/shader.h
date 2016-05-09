//
//  shader.cpp
//  image_synthesis
//
//  Created by 李镇 on 3/8/16.
//  Copyright © 2016 zl. All rights reserved.
//

#pragma once
#include <stdio.h>
#include "light.h"
#include "view.h"
#include "anyobject.h"

#ifndef PI
#define PI 3.14159265
#endif

#define normal_map_scale  0.03
//specular type, 0 -> soft, 1 -> sharp, 2 -> other shape
#define SpecularType 0
//#define _hard_spot_light_
//#define _NORMAL_MAP

class Shader{
    vector<light_src*> light;
    
public:
    Color ambient_color;
    Color specular_color;
    Color border_color;
    Shader(vector<light_src*> light_in, Color ambient_color_in, Color specular_color_in, Color border_color_in){
        light = light_in;
        ambient_color = ambient_color_in;
        specular_color = specular_color_in;
        border_color = border_color_in;
    };
    
    
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
    Color shading(Color cm0, Color cm1, Point3D ph, Vector3D& nh, View eye, bool no_border, vector<AnyObject*> objs, int object_index, Vector2D UV = Vector2D(0, 0), double m = 0, double n = 0){
        int light_num = (int)light.size();
        bool normal_map_on = false;
        
#ifdef _NORMAL_MAP
        normal_map_on = true;
#endif
        Color clr_sum = Color(0, 0, 0); // add all shading of different light source
        
        
        Color clr = cm0;
        if (objs[object_index]->texture_type == 1) {
            //type 1: texture mapping
            tuple<Color, Color, Vector3D> texture_mapping_rtn = TextureMapping(ph, nh, objs[object_index], normal_map_on, UV);
            cm0 = get<0>(texture_mapping_rtn);
            cm1 = get<1>(texture_mapping_rtn);
            nh = get<2>(texture_mapping_rtn);
        }else if(objs[object_index]->texture_type == 2){
            //type 2: solid texture
            tuple<Color, Color> texture_mapping_rtn = SolidTexture(ph, nh, objs[object_index], cm0);
            cm0 = get<0>(texture_mapping_rtn);
            cm1 = get<1>(texture_mapping_rtn);
        }else if(objs[object_index]->texture_type == 3){
            //type 3: 3D Function
            cm0 = objs[object_index]->textures[0]->Get3DFunction(ph, Point3D(0, 0, 20));
            cm1 = ambient_color;
        }
        
        if(objs[object_index]->GetObjectType() == "Environment"){
            return cm0;
        }
            
        for (int light_index = 0; light_index < light_num; light_index++) {
            Vector3D nhl;  //normal vector from light position to hitpoint
            if (light[light_index]->type == 0) {
                nhl = light[light_index]->direction;
            }else if(light[light_index]->type == 1){
                nhl = Normalize(ph - light[light_index]->position);
            }else{
                nhl = Normalize(ph - light[light_index]->position);
            }
            
            
            clr = CalcDiffuse(cm0, cm1, ph, nh, eye, nhl, light_index);
            clr = CalcSpecular(clr, ph, nh, eye, nhl, light_index);
            if (!no_border) {
                clr = CalcBorder(clr, ph, nh, eye, nhl);
            }
            clr = CalcShadow(clr, ph, nh, nhl, light_index, objs);
            //clr = CalcSoftShadow(clr, ph, nh, nhl, light_index, objs, m, n);
            //Since 'Color' class is 'char' typt, never make it larger than 255.
            clr_sum = clr_sum / (light_index + 1.0) * light_index + clr / (light_index + 1.0);
        }
        return clr_sum;
    }

    ///////////////////////////////////////
    //ambient occlusion
    Color shading2(Color cm0, Color cm1, Point3D ph, Vector3D nh, vector<AnyObject*> objs, vector<Vector3D> geo){
    
        int len = (int)geo.size();
        int cnt = 0;
        int cnt2 = len;
        //randomly rotate geodesic
        double random1 = (double)rand() / RAND_MAX;
        double random2 = (double)rand() / RAND_MAX;
        double theta1 = random1 * 2 * PI;
        double theta2 = random2 * 2 * PI;
        
        for(int i = 0; i < len; i++){
            double x = geo[i].x;
            double y = geo[i].y;
            double z = geo[i].z;
            Vector3D ray(cos(theta1) * x - sin(theta1) * y, sin(theta1) * x + cos(theta1) * y, z);
            x = ray.x;
            y = ray.y;
            z = ray.z;
            ray = Vector3D(x, cos(theta2) * y - sin(theta2) * z, sin(theta2) * y + cos(theta2) * z);
            
            int obj_nmb = (int)objs.size();
            if(DotProduct(ray, nh) <= 0){
                cnt2 -= 1;
                continue;
            }
            for(int j = 0; j < obj_nmb; j++){
                View vw(ph, ray);
                tuple<Color, double, Point3D, Vector3D, int> result = objs[j]->CalcIntersect(vw, 0);
                double t = get<1>(result);
                if(t > 0){
                    cnt += 2;
                    break;
                }
            }
        }
        //cout << cnt2 << endl;
        Color clr = cm0 * (1 - cnt * 1.0 / len) + cm1 * (cnt * 1.0 / len);

        return clr;
    }

    //color bleeding
    Color shading3(Color cm0, Color cm1, Point3D ph, Vector3D nh, vector<AnyObject*> objs, vector<Vector3D> geo){
        
        int len = (int)geo.size();
        int cnt = 0;
        int cnt2 = len;
        //randomly rotate geodesic
        double random1 = (double)rand() / RAND_MAX;
        double random2 = (double)rand() / RAND_MAX;
        double theta1 = random1 * 2 * PI;
        double theta2 = random2 * 2 * PI;
        Color clr_tmp(0,0,0);
        Vector3D clr_sum(0,0,0);
        
        for(int i = 0; i < len; i++){
            double x = geo[i].x;
            double y = geo[i].y;
            double z = geo[i].z;
            Vector3D ray(cos(theta1) * x - sin(theta1) * y, sin(theta1) * x + cos(theta1) * y, z);
            x = ray.x;
            y = ray.y;
            z = ray.z;
            ray = Vector3D(x, cos(theta2) * y - sin(theta2) * z, sin(theta2) * y + cos(theta2) * z);
            
            int obj_nmb = (int)objs.size();
            if(DotProduct(ray, nh) <= 0){
                cnt2 -= 1;
                continue;
            }
            double t_min = FLT_MAX;
            for(int j = 0; j < obj_nmb; j++){
                View vw(ph, ray);
                tuple<Color, double, Point3D, Vector3D, int> result = objs[j]->CalcIntersect(vw, 0);
                double t = get<1>(result);
                if(t > 0 && t < t_min){
                    t_min = t;
                    clr_tmp = get<0>(result);
                }
            }
            if(t_min < 100){
                clr_sum = Vector3D(clr_sum.x + clr_tmp.r, clr_sum.y + clr_tmp.g, clr_sum.z + clr_tmp.b);
                cnt += 2;
            }
        }
        
        clr_tmp = Color(clr_sum.x / cnt, clr_sum.y / cnt, clr_sum.z / cnt);
        Color clr = cm0 * (1 - cnt * 1.0 / len) + clr_tmp * (cnt * 1.0 / len);
        
        return clr;
    }

    
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
    tuple<Color, Color, Vector3D> TextureMapping(Point3D ph, Vector3D nh, AnyObject* obj, bool normal_map_on, Vector2D UV = Vector2D(0, 0)){
        Color cm0, cm1;
        if (obj->GetObjectType() == "Sphere" || obj->GetObjectType() == "Environment") {
            Sphere* sph = (Sphere*) obj;
            //            cout << sph->radius << endl;
            //            cout << sph->pCenter.x << endl;
            //            cout << sph->pCenter.y << endl;
            //            cout << sph->pCenter.z << endl;
            //            cout << (ph - sph->pCenter).length() << endl;
            //            cout << DotProduct(sph->n2, ph - sph->pCenter) << endl;
            double x = DotProduct(sph->n0, ph - sph->pCenter) / sph->radius;
            double y = DotProduct(sph->n1, ph - sph->pCenter) / sph->radius;
            double z = DotProduct(sph->n2, ph - sph->pCenter) / sph->radius;
            //            cout << x << endl;
            //            cout << y << endl;
            //            cout << z << endl;
            
            double phi = PI - acos(z);
            double theta = 0;
            if (z != 1 && z != -1) {
                long double sss = y / sqrt(1 - z*z);
                //                cout << y << endl;
                //                cout << sqrt(1 - z * z) << endl;
                //                cout << "sss: " << sss << endl;
                // Here is a BUG: when sss = -1, theta = nan, couldn't find the reason, so use the following if-else to avoid this situation
                if (sss > -1 && sss < 1) {
                    theta = acos(sss);
                }else if(sss == 1){
                    theta = 0;
                }else if(sss == -1){
                    theta = PI;
                }else{
                    theta = 0;
                }
                
                //cout << "theta: " << theta << endl;
            }else{
                theta = 0;
            }
            
            if (x < 0) {
                theta = 2 * PI - theta;
            }
            //            cout << "phi: " << phi << endl;
            //            cout << "theta: " << theta << endl;
            cm0 = obj->textures[0]->GetColorSphere(theta, phi);
            cm1 = obj->textures[1]->GetColorSphere(theta, phi);
            
            if (obj->textures.size() >= 3 && normal_map_on == true) {
                Color c_nm = sph->textures[2]->GetColorSphere(theta, phi);
                Vector3D v_nm = Vector3D(c_nm.r * 2 / 255.0 - 1, c_nm.g * 2 / 255.0 - 1, c_nm.b * 2 / 255.0 - 1);
                nh = nh + v_nm * normal_map_scale;
            }
        }else if(obj->GetObjectType() == "Plane"){
            Plane* pln = (Plane*) obj;
            //cout << (int)DotProduct(pln->n0, ph - pln->pi) << endl;
            double x = DotProduct(pln->n0, ph - pln->pi) / (pln->textures[0]->sx);
            x = x - int(x);
            double y = DotProduct(pln->n1, ph - pln->pi) / (pln->textures[0]->sy);
            y = y - int(y);
            //cout << "x: " << x << "  y: " << y << endl;
            x = (x < 0)? x + 1 : x;
            y = (y < 0)? y + 1 : y;
            
            
            cm0 = pln->textures[0]->GetColorPlane(x, y);
            cm1 = pln->textures[1]->GetColorPlane(x, y);
            //            cm0 = obj->textures[0]->GetJuliaSet(x, y);
            //            cm1 = obj->textures[1]->GetJuliaSet(x, y);
            
            if (pln->textures.size() >= 3 && normal_map_on == true) {
                Color c_nm = pln->textures[2]->GetColorPlane(x, y);
                Vector3D v_nm = Vector3D(c_nm.r * 2 / 255.0 - 1, c_nm.g * 2 / 255.0 - 1, c_nm.b * 2 / 255.0 - 1);
                //                cout << v_nm.x << endl;
                nh = nh + v_nm * normal_map_scale;
            }
        }else if(obj->GetObjectType() == "Mesh"){
            Mesh* msh = (Mesh*) obj;
            double x = UV.x;
            double y = UV.y;
            cm0 = msh->textures[0]->GetColorPlane(x, y);
            cm1 = msh->textures[1]->GetColorPlane(x, y);
            
            if (obj->textures.size() >= 3 && normal_map_on == true) {
                Color c_nm = msh->textures[2]->GetColorPlane(x, y);
                Vector3D v_nm = Vector3D(c_nm.r * 2 / 255.0 - 1, c_nm.g * 2 / 255.0 - 1, c_nm.b * 2 / 255.0 - 1);
                //                cout << v_nm.x << endl;
                nh = nh + v_nm * normal_map_scale;
            }
        }
        
        return make_tuple(cm0, cm1, nh);
    }
    
    
    
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
    
    
    tuple<Color, Color> SolidTexture(Point3D ph, Vector3D nh, AnyObject* obj, Color cm0_in){
        double t = DotProduct(ph - obj->textures[0]->p_center, obj->textures[0]->n2);
        Point3D pp = ph - obj->textures[0]->n2 * t;
        double x = DotProduct(pp - obj->textures[0]->p00, obj->textures[0]->n0) / obj->textures[0]->sx;
        double y = DotProduct(pp - obj->textures[0]->p00, obj->textures[0]->n1) / obj->textures[0]->sy;
        
//        x = x - int(x);
//        y = y - int(y);
//        x = (x < 0)? x + 1 : x;
//        y = (y < 0)? y + 1 : y;
        Color cm0, cm1;
        if (x > 0 && x < 1 && y > 0 && y < 1) {
            cm0 = obj->textures[0]->GetColorPlane(x, y);
            cm1 = obj->textures[1]->GetColorPlane(x, y);
        }else{
            cm0 = cm0_in;
            cm1 = ambient_color;
            
            //cm0 = Color(0xffffff);
            //cm1 = cm0;
        }
        
        return make_tuple(cm0, cm1);
    }
    
    
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
    Color CalcDiffuse(Color cm0, Color cm1, Point3D ph, Vector3D nh, View eye, Vector3D nhl, int i){
        double t = (DotProduct(nh, nhl * (-1)) + 1) / 2.0;
        if (light[i]->type == 2) {
#ifdef _hard_spot_light_
            if (DotProduct(light[i]->direction, nhl) < cos(light[i]->spot_angle / 180 * PI)) {
                t = 0.2;
            }
#else
            t = t * pow((DotProduct(light[i]->direction, nhl) + 1) / 2.0, 1);
#endif
            
        }
        Color clr = cm0 * (light[i]->color * t) + cm1 * (Vector3D(1, 1, 1) - light[i]->color * t);
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
        if (DotProduct(light[i]->direction, nhl) < cos(light[i]->spot_angle / 180 * PI)) {
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
    
    
    
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
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
                tuple<Color, double, Point3D, Vector3D, int> intsec = objs[i]->CalcIntersect(View(ph, nlh));
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
            if (DotProduct(light[i]->direction, nhl) < cos(light[i]->spot_angle / 180 * PI)) {
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
    
    
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    Color CalcSoftShadow(Color clr, Point3D ph, Vector3D nh, Vector3D nhl, int i, vector<AnyObject*> objs, double m = 0, double n = 0){
        double sh = 1;
        Point3D pl = light[0]->position;
//        cout << m << endl;
        pl = pl + light[0]->n0 * light[0]->sx * m + light[0]->n1 * light[0]->sy * n;
        
        Vector3D nlh = pl - ph;
        if (DotProduct(nh, nlh) <= 0) {
            //in shadow
            sh = 0.5;
        }else{
            int obj_number = (int)objs.size();
            double t_min = FLT_MAX;
            int obstacle_cnt = 0;
            for(int i = 0; i < obj_number; i++){
                tuple<Color, double, Point3D, Vector3D, int> intsec = objs[i]->CalcIntersect(View(ph, nlh));
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
            if (DotProduct(light[i]->direction, nhl) < cos(light[i]->spot_angle / 180 * PI)) {
                sh = 0.2;
            }
        #else
            sh = sh * pow((DotProduct(light[i]->direction, nhl) + 1) / 2.0, 1);
        #endif
        }
        
        
        clr = clr * sh + ambient_color * (1 - sh);
        return clr;
    }
    
    
    
    
    
};


