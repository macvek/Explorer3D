#pragma once
#define M_PI       3.14159265358979323846


struct Vec3F {
	static Vec3F UP;

	float x, y, z;

	void Print() const;

	Vec3F& normalize();
	Vec3F& mult(float c);
	Vec3F& add(const Vec3F& o);
	Vec3F& sub(const Vec3F& o);
	Vec3F crossProduct(const Vec3F& o) const;
	Vec3F rotationYXZ(const Vec3F& up) const; // vector to rotate to point towards [0,0,-1] applying in order : Y, X, Z

	float len() const;
};

double rad(double deg);
double deg(double rad);

