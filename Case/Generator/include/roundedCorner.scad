module roundedCube2(dimensions, cornerDimensions) {
    cd = cornerDimensions;
    hull() {
        roundedCorner([cd[0],cd[1],cd[2]]);
        
        translate([dimensions[0],0,0])
        rotate([0,0,90])
        roundedCorner([cd[1],cd[3],cd[2]]);
        
        translate([0,0,dimensions[2]])
        rotate([0,90,0])
        roundedCorner([cd[4],cd[1],cd[0]]);
        
        translate([dimensions[0],0,dimensions[2]])
        rotate([0,90,90])
        roundedCorner([cd[4],cd[3],cd[1]]);
        
        translate([0,dimensions[1],0])
        rotate([90,0,0])
        roundedCorner([cd[0],cd[2],cd[5]]);
        
        translate([dimensions[0],dimensions[1],0])
        rotate([90,0,-90])
        roundedCorner([cd[5],cd[2],cd[3]]);
        
        translate([0,dimensions[1],dimensions[2]])
        rotate([90,90,0])
        roundedCorner([cd[4],cd[0],cd[5]]);
        
        translate([dimensions[0],dimensions[1],dimensions[2]])
        rotate([90,90,-90])
        roundedCorner([cd[4],cd[5],cd[3]]);
    }
}

module roundedCornerNegative(dimensions) {
    difference() {
        translate(dimensions/2)
        cube(size=dimensions, center=true);
        
        translate(dimensions)
        scale(dimensions)
        sphere(1);
    }
}

module roundedCorner(dimensions) {
    intersection() {
        translate(dimensions)
        scale(dimensions)
        sphere(1);
        
        translate(dimensions/2)
        cube(size=dimensions, center=true);
    }
}