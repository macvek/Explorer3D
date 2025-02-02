#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h> // only include this one in the source file with main()!
#include <iostream>

#include <Windows.h>
#include <gl/gl.h>

#include <cmath>

#define M_PI       3.14159265358979323846

using namespace std;

struct Vec3F {
	double x,y,z;

	void Print() {
		cout << " [ " << x << "\t" << y << "\t" << z << " ] " << endl;
	}
};

struct M44 {
	double m[4][4] = { 0 };

	void asRotateX(GLfloat phi) {
		m[0][0] = 1; m[0][1] = 0;			m[0][2] = 0;		 m[0][3] = 0;
		m[1][0] = 0; m[1][1] = cos(phi);	m[1][2] = -sin(phi); m[1][3] = 0;
		m[2][0] = 0; m[2][1] = sin(phi);	m[2][2] = cos(phi);	 m[2][3] = 0;
		m[3][0] = 0; m[3][1] = 0;			m[3][2] = 0;		 m[3][3] = 1;
	}

	void asRotateY(GLfloat  phi) {
		m[0][0] = cos(phi);   m[0][1] = 0;			m[0][2] = sin(phi);	m[0][3] = 0;
		m[1][0] = 0;		  m[1][1] = 1;		   	m[1][2] = 0;		m[1][3] = 0;
		m[2][0] = -sin(phi);  m[2][1] = 0;			m[2][2] = cos(phi); m[2][3] = 0;
		m[3][0] = 0;		  m[3][1] = 0;			m[3][2] = 0;		m[3][3] = 1;
	}

	void asRotateZ(GLfloat phi) {
		m[0][0] = cos(phi);   m[0][1] = -sin(phi);	m[0][2] = 0; m[0][3] = 0;
		m[1][0] = sin(phi);	  m[1][1] = cos(phi);	m[1][2] = 0; m[1][3] = 0;
		m[2][0] = 0;		  m[2][1] = 0;			m[2][2] = 1; m[2][3] = 0;
		m[3][0] = 0;		  m[3][1] = 0;			m[3][2] = 0; m[3][3] = 1;
	}

	void FillFrom(M44& s) {
		for (int y = 0; y < 4; ++y)
			for (int x = 0; x < 4; ++x)
				m[y][x] = s.m[y][x];
	}

	void Mult(M44& o) {
		M44 r;
		for (int l = 0; l < 4; ++l) {
			r.m[l][0] = m[l][0] * o.m[0][0] + m[l][1] * o.m[1][0] + m[l][2] * o.m[2][0] + m[l][3] * o.m[3][0];
			r.m[l][1] = m[l][0] * o.m[0][1] + m[l][1] * o.m[1][1] + m[l][2] * o.m[2][1] + m[l][3] * o.m[3][1];
			r.m[l][2] = m[l][0] * o.m[0][2] + m[l][1] * o.m[1][2] + m[l][2] * o.m[2][2] + m[l][3] * o.m[3][2];
			r.m[l][3] = m[l][0] * o.m[0][3] + m[l][1] * o.m[1][3] + m[l][2] * o.m[2][3] + m[l][3] * o.m[3][3];
		}

		FillFrom(r);
	}

	void Print() {
		cout << " [ " << m[0][0] << "\t" << m[0][1] << "\t" << m[0][2] << "\t" << m[0][3] << " ] " << endl;
		cout << " [ " << m[1][0] << "\t" << m[1][1] << "\t" << m[1][2] << "\t" << m[1][3] << " ] " << endl;
		cout << " [ " << m[2][0] << "\t" << m[2][1] << "\t" << m[2][2] << "\t" << m[2][3] << " ] " << endl;
		cout << " [ " << m[3][0] << "\t" << m[3][1] << "\t" << m[3][2] << "\t" << m[3][3] << " ] " << endl;
	}

	Vec3F ApplyOnPoint(Vec3F& p) {
		double nX = m[0][0] * p.x + m[0][1] * p.y + m[0][2] * p.z + m[0][3];
		double nY = m[1][0] * p.x + m[1][1] * p.y + m[1][2] * p.z + m[1][3];
		double nZ = m[2][0] * p.x + m[2][1] * p.y + m[2][2] * p.z + m[2][3];

		return Vec3F{ nX , nY, nZ };
	}

};

struct OpenGLProperties {
	string nameVendor;
	string nameRenderer;
	string nameVersion;
	string nameExtension;

	int major;
	int minor;

	void toStream(std::ostream& out) {
		out << "VENDOR:   " << nameVendor << endl
			<< "RENDERER: " << nameRenderer << endl
			<< "VERSION:  " << nameVersion << endl
			<< "SELECTED: " << major << "." << minor << endl;
	}
};


struct AppContext {
	int width = 600;
	int height = 600;

	float pointerSpeed = 0.2;

	SDL_GLContext glcontext;
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
	string lastError;
	OpenGLProperties openglProperties;

	bool startSDL();
	void stopSDL();
} App;

bool AppContext::startSDL() {
	SDL_Init(SDL_INIT_VIDEO);
	if (!SDL_GL_LoadLibrary(nullptr)) {
		lastError = "Failed to load GL library";
		return false;
	}
	
	int requestedValue = 8;
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, requestedValue);

	window = SDL_CreateWindow("Explorer3D", width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	renderer = SDL_CreateRenderer(window, "opengl");
	glcontext = SDL_GL_CreateContext(window);

	int loadedValue;
	SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &loadedValue);
	if (loadedValue < requestedValue) {
		lastError = "Failed to get expected stencil size; aborting";
		return false;
	}
	
	openglProperties.nameVendor = string((char*)(glGetString(GL_VENDOR)));
	openglProperties.nameRenderer = string((char*)(glGetString(GL_RENDERER)));
	openglProperties.nameVersion = string((char*)(glGetString(GL_VERSION)));
	openglProperties.nameExtension = string((char*)(glGetString(GL_EXTENSIONS)));

	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &openglProperties.major);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &openglProperties.minor);

	return true;
}

void AppContext::stopSDL() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

// Determines which coordinates are used for angle and movement calculation
enum MovementStrategy {
	MoveHybrid,		// goes towards direction, up is relative up; but rotates along Y axis with a perception of looking down
	MoveFreespace,	// as if it space ship; all rotations are relative to observer
	MoveXYZ,		// looking around with respect to top/bottom, moving on XZ plane, moving Up/Down only with dedicated commands
};

struct DrawPlane {

	MovementStrategy movement = MoveHybrid;
	const int fovDiff = 1;
	const float fovMax = 160;
	const float fovMin = 5;

	const float nearPlane = 0.1;
	const float farPlane = 10;

	GLdouble fov = 60;
	int frames = 0;

	float posX = 0;
	float posY = 0;
	float posZ = 0;

	float aX = 0;
	float aY = 0;
	float aZ = 0;

	const float moveSpeed = 0.02;

	int moveAlongX = 0;
	int moveAlongY = 0;
	int moveAlongZ = 0;
	
	bool refresh = false;
	void init() {
		glViewport(0, 0, App.width, App.height);
		refresh = true;
	}

	void applyMovesXYZ() {
		Vec3F vRotated = { 0,0,0 };
		Vec3F v = { moveAlongX * moveSpeed, 0, moveAlongZ * moveSpeed };

		M44 mX;	mX.asRotateX(rad(-aX));
		M44 m; m.asRotateY(rad(-aY));

		m.Mult(mX);

		vRotated = m.ApplyOnPoint(v);

		posX += vRotated.x;
		posY += moveAlongY * moveSpeed;
		posZ += vRotated.z;
	}

	void applyMovesHybrid() {
		Vec3F vRotated = { 0,0,0 };
		Vec3F v = { moveAlongX * moveSpeed, moveAlongY * moveSpeed, moveAlongZ * moveSpeed };

		M44 mX; mX.asRotateX(rad(-aX));
		M44 m; m.asRotateY(rad(-aY));

		m.Mult(mX);

		vRotated = m.ApplyOnPoint(v);
		posX += vRotated.x;
		posY += vRotated.y;
		posZ += vRotated.z;
	}

	void applyMoves() {
		if (moveAlongX == 0 && moveAlongZ == 0 && moveAlongY == 0) {
			return;
		}

		if (movement == MoveXYZ) {
			applyMovesXYZ();
		}
		else if (movement == MoveHybrid) {
			applyMovesHybrid();
		}
	}

	void pointerUpdateHybridXYZ(float dX, float dY) {
		// sic! - moving left-right (pointer X) rotates around axis Y, and pointer Y around axis X
		aY += App.pointerSpeed * dX;
		aX += App.pointerSpeed * dY;

		if (aX < -90) aX = -90;
		if (aX > 90) aX = 90;
	}

	void pointerUpdateFreespace(float dX, float dY) {
		// normalize UP vector and aY to global coordinates;
	}

	void pointerUpdate(float dX, float dY) {
		if (movement == MoveHybrid || movement == MoveXYZ) {
			pointerUpdateHybridXYZ(dX, dY);
		}
		else if (movement == MoveFreespace) {
			pointerUpdateFreespace(dX, dY);
		}
	}

	void updateFov(float newFov) {
		fov = max<float>(min<float>(newFov, fovMax), fovMin);
		refresh = true;
	}

	void updateProjection() {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		bool perspetive = true;
		if (perspetive) {
			GLdouble aspectRatio = (1.0*App.width) / App.height;
			GLdouble tangent = tan(rad(fov / 2));
			GLdouble right = nearPlane * tangent;
			GLdouble top = right / aspectRatio;

			glFrustum(-right, right, -top, top, nearPlane, farPlane);
		}
		else {
			glOrtho(-1, 1, -1, 1, nearPlane, farPlane);
		}

		glMatrixMode(GL_MODELVIEW);
	}

	double rad(double deg) {
		return M_PI / 180 * deg;
	}

	double deg(double rad) {
		return 180 / M_PI * rad;
	}

	void eyeCoordsHybridXYZ() {
		glRotatef(aX, 1, 0, 0);
		glRotatef(aY, 0, 1, 0);
		glRotatef(aZ, 0, 0, 1);
	}

	void eyeCoordsFreespace() {
		// rotate to make UP vector facing [0,1,0] 
		// rotate along Y axis


	}

	void frame() {
		++frames;

		applyMoves();

		if (refresh) {
			refresh = false;
			updateProjection();
		}
		glClear(GL_COLOR_BUFFER_BIT);
		glLoadIdentity();

		if (movement == MoveHybrid || movement == MoveXYZ) {
			eyeCoordsHybridXYZ();
		}
		if (movement == MoveFreespace) {
			eyeCoordsFreespace();
		}
		
		
		glTranslatef(-posX, -posY, -posZ);

		glColor3f(0.3, 0.3, 0.3);
		drawGrid(0);

		glColor3f(0.3, 0.3, 0.5);
		drawGrid(3);

		glPushMatrix();
			glTranslatef(0, 0, -3);
			drawQuad();
		glPopMatrix();

		SDL_GL_SwapWindow(App.window);
	}

	void drawQuad() {
		glPushMatrix();
		GLdouble spin = 0.2 * frames;
		glRotatef(spin, 1, 0.4, 0.2);

		glColor4f(1, 1, 1, 1);
		glBegin(GL_QUADS);

		glColor3f(1, 1, 0);  glVertex2f(-1, -1);
		glColor3f(1, 0, 1);  glVertex2f(1, -1);
		glColor3f(0, 1, 1);  glVertex2f(1, 1);
		glColor3f(1, 1, 1);  glVertex2f(-1, 1);

		glEnd();
		glPopMatrix();
	}

	void drawGrid(GLfloat y) {
		for (int x = -10; x <= 10; ++x) {
			glBegin(GL_LINES);
			glVertex3f(x, y, -10);
			glVertex3f(x, y, 10);
			glEnd();
		}

		for (int z = -10; z <= 10; ++z) {
			glBegin(GL_LINES);
			glVertex3f(-10, y, z);
			glVertex3f(10, y, z);
			glEnd();
		}
	}

};

int main(int argc, char** argv) {
	if (!App.startSDL()) {
		cout << "Failed to start, error: " << App.lastError << endl;
		return 1;
	}

	App.openglProperties.toStream(cout);
	SDL_SetWindowRelativeMouseMode(App.window, true);

	bool showEvent = false;
	DrawPlane d;

	d.init();
	
	const int FPS = 60;
	int milis = 1000 / 60;

	for (;;) {
		SDL_Event event;
		if (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_WINDOW_RESIZED) {
				SDL_WindowEvent* windowEvent = (SDL_WindowEvent*)&event;
				App.width = windowEvent->data1;
				App.height = windowEvent->data2;
				d.init();
			}
			
			if (event.type == SDL_EVENT_MOUSE_MOTION) {
				SDL_MouseMotionEvent* mouseEvent = (SDL_MouseMotionEvent*) &event;
				d.pointerUpdate(mouseEvent->xrel, mouseEvent->yrel);
			}
			
			if (event.type == SDL_EVENT_KEY_DOWN) {
				SDL_KeyboardEvent* keyEvent = (SDL_KeyboardEvent*)&event;
				if (false) {}
				
				else if (keyEvent->key == SDLK_A) {
					d.moveAlongX = -1;
				}
				else if (keyEvent->key == SDLK_D) {
					d.moveAlongX = 1;
				}
				else if (keyEvent->key == SDLK_W) {
					d.moveAlongZ = -1;
				}
				else if (keyEvent->key == SDLK_S) {
					d.moveAlongZ = 1;
				}
				else if (keyEvent->key == SDLK_SPACE) {
					d.moveAlongY = 1;
				}
				else if (keyEvent->key == SDLK_LSHIFT) {
					d.moveAlongY = -1;
				}
			}

			if (event.type == SDL_EVENT_KEY_UP) {
				SDL_KeyboardEvent* keyEvent = (SDL_KeyboardEvent*)&event;
				if (keyEvent->key == SDLK_KP_PLUS) {
					d.updateFov(d.fov + d.fovDiff);
					cout << "FOV:" << d.fov << endl;
				}
				else if (keyEvent->key == SDLK_KP_MINUS) {
					d.updateFov(d.fov - d.fovDiff);
					cout << "FOV:" << d.fov << endl;
				}
				else if (keyEvent->key == SDLK_A || keyEvent->key == SDLK_D) {
					d.moveAlongX = 0;
				}
				else if (keyEvent->key == SDLK_W || keyEvent->key == SDLK_S) {
					d.moveAlongZ = 0;
				}
				else if (keyEvent->key == SDLK_SPACE || keyEvent->key == SDLK_LSHIFT) {
					d.moveAlongY = 0;
				}
				else if (keyEvent->key == SDLK_8) {
					d.movement = MoveHybrid;
					cout << "Movement: hybrid\n";
				}
				else if (keyEvent->key == SDLK_9) {
					d.movement = MoveXYZ;
					cout << "Movement: xyz\n";
				}
				else if (keyEvent->key == SDLK_0) {
					d.movement = MoveFreespace;
					cout << "Movement: freespace\n";
				}
			}

			if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
				break;
			}
			else if (showEvent) {
				std::cout << "Event " << event.type << std::endl;
			}
		}
		else {
			SDL_Delay(milis);
			d.frame();
			
		}
	}

	App.stopSDL();
	return 0;
}