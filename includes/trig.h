#pragma once
#define M_PI       3.14159265358979323846


struct Vec3F {
	float x, y, z;

	void Print() const;

	void normalize();

	float len() const;
};

double rad(double deg);
double deg(double rad);

