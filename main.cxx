#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h> // only include this one in the source file with main()!
#include <iostream>

#include <Windows.h>
#include <gl/gl.h>

#define M_PI       3.14159265358979323846

SDL_GLContext glcontext;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

const int WIDTH = 600;
const int HEIGHT = 600;

struct Drawable {
	virtual void init() = 0;
	virtual void frame() = 0;
};


struct Draw_017 : public Drawable {

	void processHits(GLint hits, GLuint buffer[])
	{
		unsigned int i, j;
		GLuint names, * ptr;
		printf("hits = %d\n", hits);
		ptr = (GLuint*)buffer;
		for (i = 0; i < hits; i++) { /* for each hit */
			names = *ptr;
			printf(" number of names for hit = %d\n", names); ptr++;
			printf(" z1 is %g;", (float)*ptr / 0x7fffffff); ptr++;
			printf(" z2 is %g\n", (float)*ptr / 0x7fffffff); ptr++;
			printf(" the name is ");
			for (j = 0; j < names; j++) { /* for each name */
				printf("%d ", *ptr); ptr++;
			}
			printf("\n");
		}
	}

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
		GLuint selectBuf[500];
		GLint hits;
		glSelectBuffer(500, selectBuf);
		glRenderMode(GL_SELECT);
		std::cout << "RENDER MODE SELECT\n";
		
		glInitNames();
		glPushName(10);

		glLoadIdentity();
		glTranslatef(0, -0.5, 0);
		glRotatef(45, 1, 1, 0);
		
		glBegin(GL_QUADS);
		glVertex2f(-0.4, -0.4);
		glVertex2f(0.4, -0.4);
		glVertex2f(0.4, 0.4);
		glVertex2f(-0.4, 0.4);
		glEnd();

		glLoadIdentity();
		glTranslatef(0, 0.5, -1);
		glRotatef(45, 1, 1, 0);
		glLoadName(20);
		
		glBegin(GL_QUADS);
		glVertex2f(-0.3, -0.3);
		glVertex2f(0.3, -0.3);
		glVertex2f(0.3, 0.3);
		glVertex2f(-0.3, 0.3);
		glEnd();

		glFlush();
		hits = glRenderMode(GL_RENDER);
		processHits(hits, selectBuf);
		std::cout << "Hits done: \n";
		
		SDL_GL_SwapWindow(window);
	}

	void frame() {

	}
};

int main(int argc, char** argv) {

	SDL_Init(SDL_INIT_VIDEO);
	if (!SDL_GL_LoadLibrary(nullptr)) {
		std::cout << "Failed to load GL" << std::endl;
	}
	
	int value;
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	window = SDL_CreateWindow("Hello SDL", WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
	renderer = SDL_CreateRenderer(window, "opengl");

	glcontext = SDL_GL_CreateContext(window);
	
	SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &value);
	std::cout << "STENCIL_SIZE: " << value << std::endl;

	const GLubyte* nameVendor = glGetString(GL_VENDOR);
	const GLubyte* nameRenderer = glGetString(GL_RENDERER);
	const GLubyte* nameVersion = glGetString(GL_VERSION);
	const GLubyte* nameExtension = glGetString(GL_EXTENSIONS);

	std::cout << "Version: " << nameVendor << " ; " << nameRenderer << " ; " << nameVersion << " ; " << nameExtension << std::endl << std::endl;

	int major, minor;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);

	std::cout << "MAJOR: " << major << "; MINOR:" << minor << std::endl;
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

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}