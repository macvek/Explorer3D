#pragma once
#include <trig.h>

template <typename T> struct M44 {
	T m[4][4] = { 1,0,0,0, 0,1,0,0 , 0,0,1,0, 0,0,0,1 };

	M44& asRotateX(T phi) {
		m[1][1] = cos(phi);	m[2][1] = -sin(phi);
		m[1][2] = sin(phi);	m[2][2] = cos(phi);
		return *this;
	}

	M44& asRotateY(T phi) {
		m[0][0] = cos(phi);   m[2][0] = sin(phi);
		m[0][2] = -sin(phi);  m[2][2] = cos(phi);
		return *this;
	}

	M44& asRotateZ(T phi) {
		m[0][0] = cos(phi);   m[1][0] = -sin(phi);
		m[0][1] = sin(phi);	  m[1][1] = cos(phi);
		return *this;
	}

	M44& asTranslate(T x, T y, T z) {
		m[3][0] = x;
		m[3][1] = y;
		m[3][2] = z;

		return *this;
	}

	void FillFrom(M44& s) {
		for (int y = 0; y < 4; ++y)
			for (int x = 0; x < 4; ++x)
				m[y][x] = s.m[y][x];
	}

	void Mult(M44& o) {
		M44 r;
		for (int l = 0; l < 4; ++l) {
			r.m[0][l] = m[0][l] * o.m[0][0] + m[1][l] * o.m[0][1] + m[2][l] * o.m[0][2] + m[3][l] * o.m[0][3];
			r.m[1][l] = m[0][l] * o.m[1][0] + m[1][l] * o.m[1][1] + m[2][l] * o.m[1][2] + m[3][l] * o.m[1][3];
			r.m[2][l] = m[0][l] * o.m[2][0] + m[1][l] * o.m[2][1] + m[2][l] * o.m[2][2] + m[3][l] * o.m[2][3];
			r.m[3][l] = m[0][l] * o.m[3][0] + m[1][l] * o.m[3][1] + m[2][l] * o.m[3][2] + m[3][l] * o.m[3][3];
		}

		FillFrom(r);
	}

	void Print() const {
		Log.printf("[ %f\t%f\t%f\t%f ]\n", m[0][0], m[1][0], m[2][0], m[3][0]);
		Log.printf("[ %f\t%f\t%f\t%f ]\n", m[0][1], m[1][1], m[2][1], m[3][1]);
		Log.printf("[ %f\t%f\t%f\t%f ]\n", m[0][2], m[1][2], m[2][2], m[3][2]);
		Log.printf("[ %f\t%f\t%f\t%f ]\n", m[0][3], m[1][3], m[2][3], m[3][3]);
	}

	Vec3F ApplyOnPoint(Vec3F& p) const {
		float nX = m[0][0] * p.x + m[1][0] * p.y + m[2][0] * p.z + m[3][0];
		float nY = m[0][1] * p.x + m[1][1] * p.y + m[2][1] * p.z + m[3][1];
		float nZ = m[0][2] * p.x + m[1][2] * p.y + m[2][2] * p.z + m[3][2];

		return Vec3F{ nX , nY, nZ };
	}

};