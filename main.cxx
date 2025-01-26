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


struct Draw_017 : public Drawable {

	void init() {
		std::cout << "\nDraw_017... \n\n";
		
		glClear(GL_COLOR_BUFFER_BIT);

		glColor4f(1, 0, 1, 1);
		glViewport(0, 0, WIDTH, HEIGHT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-1, 1, 1, -1, -1, 1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glBegin(GL_QUADS);
		glVertex2f(-0.6, 0.0);
		glVertex2f(0.0, 0.6);
		glVertex2f(0.6, 0.0);
		glVertex2f(0.0, -0.6);
		glEnd();
		GLuint texName;
		glGenTextures(1, &texName);
		glBindTexture(GL_TEXTURE_2D, texName);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 44, 44, 512, 512, 0);

		
		glClear(GL_COLOR_BUFFER_BIT);

		glEnable(GL_TEXTURE_2D);
		glTranslatef(0, -0.5,0 );
		glRotatef(45, 1, 1, 0);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);  glVertex2f(-0.4, -0.4);
		glTexCoord2f(4, 0); glVertex2f(0.4, -0.4);
		glTexCoord2f(4, 4); glVertex2f(0.4, 0.4);
		glTexCoord2f(0, 4); glVertex2f(-0.4, 0.4);
		glEnd();
		glLoadIdentity();
		glTranslatef(0, 0.5, 0);
		glRotatef(60, -1, 0.5, 0);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);  glVertex2f(-0.4, -0.4);
		glTexCoord2f(4, 0); glVertex2f(0.4, -0.4);
		glTexCoord2f(4, 4); glVertex2f(0.4, 0.4);
		glTexCoord2f(0, 4); glVertex2f(-0.4, 0.4);
		glEnd();
		
		SDL_GL_SwapWindow(App.window);
	}

	void frame() {

	}
};

int main(int argc, char** argv) {

	if (!App.startSDL()) {
		cout << "Failed to start, error: " << App.lastError << endl;
		return 1;
	}

	App.openglProperties.toStream(cout);

	bool showEvent = false;
	Drawable& d = Draw_017();

	d.init();
	
	const int FPS = 60;
	int milis = 1000 / 60;

	
	for (;;) {
		SDL_Event event;
		if (SDL_PollEvent(&event)) {
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