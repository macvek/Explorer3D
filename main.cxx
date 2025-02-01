#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h> // only include this one in the source file with main()!
#include <iostream>

#include <Windows.h>
#include <gl/gl.h>

#include <cmath>

#define M_PI       3.14159265358979323846
const int WIDTH = 600;
const int HEIGHT = 600;

using namespace std;

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

	window = SDL_CreateWindow("Explorer3D", WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
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

struct DrawPlane {

	const int fovDiff = 1;
	const double fovMax = 160;
	const double fovMin = 5;

	const double nearPlane = 0.1;
	const double farPlane = 10;

	GLdouble fov = 60;
	int frames = 0;

	double posX = 0;
	double posY = 0;
	double posZ = 0;

	double aX = 0;
	double aY = 0;

	const double moveSpeed = 0.02;

	int moveAlongX = 0;
	int moveAlongZ = 0;

	bool refresh = false;
	void init() {

		glViewport(0, 0, WIDTH, HEIGHT);
		refresh = true;
	}

	

	void applyMoves() {

		double zOff = cos(rad(-aY));
		double xOff = sin(rad(-aY));
		
		if (moveAlongX != 0 || moveAlongZ != 0) {
			posX += -moveSpeed * xOff;
			posZ += -moveSpeed * zOff;
		}
	}

	void pointerUpdate(float dX, float dY) {
		// sic! - moving left-right (pointer X) rotates around axis Y, and pointer Y around axis X
		aY += 0.2 * dX;
		aX += 0.2 * dY;

		cout << "aX:" << aX << "\t" << "aY:" << aY << "\n";
	}

	void updateFov(double newFov) {
		fov = max<double>(min<double>(newFov, fovMax), fovMin);
		refresh = true;
	}

	void updateProjection() {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		bool perspetive = true;
		if (perspetive) {
			GLdouble aspectRatio = WIDTH / HEIGHT;
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

	void frame() {
		++frames;

		applyMoves();

		if (refresh) {
			refresh = false;
			updateProjection();
		}
		glClear(GL_COLOR_BUFFER_BIT);
		glLoadIdentity();

		glRotatef(aX, 1, 0, 0);
		glRotatef(aY, 0, 1, 0);
		
		glTranslatef(-posX, -posY, -posZ);

		drawGrid();

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

	void drawGrid() {
		float c = 0.3;
		glColor3f(c, c, c);
		for (int x = -10; x <= 10; ++x) {
			glBegin(GL_LINES);
			glVertex3f(x, -0.2, -10);
			glVertex3f(x, -0.2, 10);
			glEnd();
		}

		for (int z = -10; z <= 10; ++z) {
			glBegin(GL_LINES);
			glVertex3f(-10, -0.2, z);
			glVertex3f(10, -0.2, z);
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
			if (event.type == SDL_EVENT_MOUSE_MOTION) {
				SDL_MouseMotionEvent* mouseEvent = (SDL_MouseMotionEvent*) &event;
				d.pointerUpdate(mouseEvent->xrel, mouseEvent->yrel);
			}
			
			if (event.type == SDL_EVENT_KEY_DOWN) {
				SDL_KeyboardEvent* keyEvent = (SDL_KeyboardEvent*)&event;
				if (false) {}
				
				else if (keyEvent->key == SDLK_LEFT) {
					d.moveAlongX = -1;
				}
				else if (keyEvent->key == SDLK_RIGHT) {
					d.moveAlongX = 1;
				}
				else if (keyEvent->key == SDLK_UP) {
					d.moveAlongZ = -1;
				}
				else if (keyEvent->key == SDLK_DOWN) {
					d.moveAlongZ = 1;
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
				else if (keyEvent->key == SDLK_LEFT || keyEvent->key == SDLK_RIGHT) {
					d.moveAlongX = 0;
				}
				else if (keyEvent->key == SDLK_UP || keyEvent->key == SDLK_DOWN) {
					d.moveAlongZ = 0;
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