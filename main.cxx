#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h> // only include this one in the source file with main()!
#include <iostream>

#include <Windows.h>
#include <gl/gl.h>

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

GLfloat angleY = 45;
GLfloat angleX = 45;

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


struct Drawable {
	virtual void init() = 0;
	virtual void frame() = 0;
};


struct DrawPlane : public Drawable {

	const int fovDiff = 1;
	const double fovMax = 160;
	const double fovMin = 5;

	const double nearPlane = 0.1;
	const double farPlane = 50;

	GLdouble FOV = 60;
	int frames = 0;

	bool refresh = false;
	void init() {

		glViewport(0, 0, WIDTH, HEIGHT);
		refresh = true;
	}

	void updateFov(double newFov) {
		FOV = max<double>(min<double>(newFov, fovMax), fovMin);
		refresh = true;
	}

	void updateProjection() {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		bool perspetive = true;
		if (perspetive) {
			GLdouble aspectRatio = WIDTH / HEIGHT;
			GLdouble tangent = tan(FOV / 2 * M_PI / 180);
			GLdouble right = nearPlane * tangent;
			GLdouble top = right / aspectRatio;

			glFrustum(-right, right, -top, top, nearPlane, farPlane);
		}
		else {
			glOrtho(-1, 1, -1, 1, nearPlane, farPlane);
		}

		glMatrixMode(GL_MODELVIEW);
	}

	void frame() {
		++frames;

		if (refresh) {
			refresh = false;
			updateProjection();
		}
		glClear(GL_COLOR_BUFFER_BIT);
		glLoadIdentity();

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
				angleX += mouseEvent->xrel;
				angleY += mouseEvent->yrel;
			}
			
			if (event.type == SDL_EVENT_KEY_UP) {
				SDL_KeyboardEvent* keyEvent = (SDL_KeyboardEvent*)&event;
				if (keyEvent->key == SDLK_KP_PLUS) {
					d.updateFov(d.FOV + d.fovDiff);
					cout << "FOV:" << d.FOV << endl;
				}
				else if (keyEvent->key == SDLK_KP_MINUS) {
					d.updateFov(d.FOV - d.fovDiff);
					cout << "FOV:" << d.FOV << endl;
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