#include <trig.h>
#include <log.h>
#include <m44.h>

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

Vec3F& Vec3F::sub(const Vec3F& o) {
	x -= o.x;
	y -= o.y;
	z -= o.z;
	return *this;
}

Vec3F Vec3F::UP = { 0,1,0 };

Vec3F Vec3F::crossProduct(const Vec3F& o) const {
	return {
		y * o.z - z * o.y,
		z * o.x - x * o.z,
		x * o.y - y * o.x
	};
}


Vec3F Vec3F::rotationYXZ(const Vec3F &up) const{
	float radY = atan2f(-x, -z);

	M44F revY; revY.asRotateY(-radY);
	Vec3F rotatedX = revY.ApplyOnPoint(*this);

	float radX = atan2f(rotatedX.y, -rotatedX.z);

	Vec3F ret = {
		radX,
		radY,
		0
	};

	M44F m;
	m
		.Mult(M44F().asRotateX(-ret.x))
		.Mult(M44F().asRotateY(-ret.y));


	if (up.x != 0 || up.y != 1 || up.z == 0) {
		Vec3F revUp = m.ApplyOnPoint(up);
		float radZ = atan2f(-revUp.x, revUp.y);
		ret.z = radZ;
	}

	return ret;
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
