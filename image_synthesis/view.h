//
//  view.cpp
//  image_synthesis
//
//  Created by 李镇 on 3/8/16.
//  Copyright © 2016 zl. All rights reserved.
//

#pragma once
#include <stdio.h>
#include "vector3D.h"

class View{
public:
    Point3D pe;  //eye position
    Vector3D npe; //direction of view, npe MUST BE unit vector
    
    View(){
        pe = Point3D(0, 0, 0);
        npe = Vector3D(0, 1, 0);
    }
    
    View(Point3D pe_in, Vector3D npe_in):pe(pe_in), npe(Normalize(npe_in)){};
    
    View(double pe_x, double pe_y, double pe_z, double npe_x, double npe_y, double npe_z){
        pe = Point3D(pe_x, pe_y, pe_z);
        npe = Normalize(Vector3D(npe_x, npe_y, npe_z));
    }
};