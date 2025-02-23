#pragma once
#define M_PI       3.14159265358979323846


struct Vec3F {
	float x, y, z;

	void Print() const;

	Vec3F& normalize();
	Vec3F& mult(float c);
	Vec3F& add(const Vec3F& o);
	Vec3F rotationXYZ(const Vec3F& up) const; // vector to rotate to point towards [0,0,-1]

	float len() const;
};

double rad(double deg);
double deg(double rad);

