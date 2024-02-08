pcb_height=10;

module base_pcb() {
  // com port side, [0,0] right lower. Points Counterclockwise
  polygon([
        [0,0],              
        [0,132],
        [50,132],
        [50,132+65],
        [-40,132+65],
        [-40, 173],
        [-50, 173],
        [-50,128],
        [-131,128],
        [-131,65],
        [-116,65],
        [-116,0]]);
};

module extrude_pcb() {
    difference() {
        linear_extrude(pcb_height) base_pcb();
        
        for(i=[0:$children-1]) 
            translate([0,0,-.5]) linear_extrude(pcb_height+1) children(i);
    }
};

module semicircle_rect(w,h) {
    translate([w/2,h-w/2]) circle(d=w);
    translate([0,-1]) square([w,h-w/2+1]);
};

module pcb_screw_hole() {
    circle(d=4);
};


module main_pcb_no_led() {
    extrude_pcb() {
        translate([-41,8]) pcb_screw_hole();
        translate([-109,23]) pcb_screw_hole();
        translate([-124,75]) pcb_screw_hole();
        translate([-124,118]) pcb_screw_hole();
        translate([-77,0,0]) 
            semicircle_rect(4,8); // right bottom insert
        translate([-104,0,0]) 
            semicircle_rect(4,8); // left bottom insert
        translate([8,185+12]) 
            rotate([0,0,180]) semicircle_rect(8,12); // left top insert
        translate([30+17,180+17])
            rotate([0,0,180]) semicircle_rect(17,17); // right top insert
        translate([45,180]) square([15,25]);
    };
};

// add space for superior right led
union() {
  main_pcb_no_led();
  linear_extrude(pcb_height) translate([50-5,180+1]) semicircle_rect(5,5);  
}

$fn=20;