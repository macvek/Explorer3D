#include <trig.h>
#include <log.h>

void Vec3F::Print() const{
	Log.printf("[ %f\t%f\t%f ]\n", x, y, z);
}

void Vec3F::normalize() {
	float l = len();
	x /= l;
	y /= l;
	z /= l;
}

float Vec3F::len() const {
	return sqrt(x * x + y * y + z * z);
}


double rad(double deg) {
	return M_PI / 180 * deg;
}

double deg(double rad) {
	return 180 / M_PI * rad;
}
