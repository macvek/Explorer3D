#include <trig.h>
#include <log.h>

void Vec3F::Print() const{
	Log.printf("[ %f\t%f\t%f ]\n", x, y, z);
}

Vec3F& Vec3F::normalize() {
	float l = len();
	return mult(1 / l);
}

Vec3F& Vec3F::mult(float c) {
	x *= c;
	y *= c;
	z *= c;
	return *this;
}

Vec3F& Vec3F::add(const Vec3F& o) {
	x += o.x;
	y += o.y;
	z += o.z;
	return *this;
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
