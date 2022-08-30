// Higher definition curves
$fs = 0.01;

function stringIn(string, list) = search([string],list)!=[[]];

module roundedcube(size = [1, 1, 1], center = false, radius = 0.5, apply_to = ["all"]) {
	// If single value, convert to [x, y, z] vector
	size = (size[0] == undef) ? [size, size, size] : size;

	translate_min = radius;
	translate_xmax = size[0] - radius;
	translate_ymax = size[1] - radius;
	translate_zmax = size[2] - radius;

	diameter = radius * 2;

	module build_point(type = "sphere", rotate = [0, 0, 0]) {
		if (type == "sphere") {
			sphere(r = radius);
		} else if (type == "cylinder") {
			rotate(a = rotate)
			cylinder(h = diameter, r = radius, center = true);
		}
	}

	obj_translate = (center == false) ?
		[0, 0, 0] : [
			-(size[0] / 2),
			-(size[1] / 2),
			-(size[2] / 2)
		];

	translate(v = obj_translate) {
		hull() {
			for (translate_x = [translate_min, translate_xmax]) {
				x_at = (translate_x == translate_min) ? "min" : "max";
				for (translate_y = [translate_min, translate_ymax]) {
					y_at = (translate_y == translate_min) ? "min" : "max";
					for (translate_z = [translate_min, translate_zmax]) {
						z_at = (translate_z == translate_min) ? "min" : "max";

						translate(v = [translate_x, translate_y, translate_z])
						if (
							(stringIn("all", apply_to)) ||
							(stringIn("xmin", apply_to) && x_at == "min") || (stringIn("xmax", apply_to) && x_at == "max") ||
							(stringIn("ymin", apply_to) && y_at == "min") || (stringIn("ymax", apply_to) && y_at == "max") ||
							(stringIn("zmin", apply_to) && z_at == "min") || (stringIn("zmax", apply_to) && z_at == "max")
						) {
							build_point("sphere");
						} else {
							rotate = 
								(stringIn("xmin", apply_to) || stringIn("xmax", apply_to) || stringIn("x", apply_to)) ? [0, 90, 0] : (
								(stringIn("ymin", apply_to) || stringIn("ymax", apply_to) || stringIn("y", apply_to)) ? [90, 90, 0] :
								[0, 0, 0]
							);
							build_point("cylinder", rotate);
						}
					}
				}
			}
		}
	}
}