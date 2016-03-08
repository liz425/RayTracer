//
//  light.cpp
//  image_synthesis
//
//  Created by 李镇 on 3/8/16.
//  Copyright © 2016 zl. All rights reserved.
//

#pragma once
#include <stdio.h>

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

