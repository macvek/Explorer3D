#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h> // only include this one in the source file with main()!
#include <SDL3_image/SDL_image.h>

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <cstdarg>

#include <Windows.h>
#include <gl/gl.h>

#include <cmath>

#define M_PI       3.14159265358979323846

using namespace std;

struct MessageLog {
	list<string> history;
	char buffer[120] = {};
	int unreadMessages = 0;

	void printf(const char* format, ...) {
		va_list args;
		va_start(args, format);
		vsprintf_s(buffer, format, args);
		va_end(args);

		cout << buffer;

		char* start = buffer;
		char* ptr = buffer;

		for (;;) {
			char here = *ptr;
			if (here == '\n' || (here == 0 && ptr-start>1) ) {
				*ptr = 0;
				history.push_back(start);
				++unreadMessages;
				start = ptr + 1;
			}

			if (here == 0) {
				break;
			}
			else {
				++ptr;
			}
		}

		unreadMessages = min(10, unreadMessages);

	}
} Log;

template <typename T> struct XYGeneric {
	T x = 0;
	T y = 0;
};

using XYFloat = XYGeneric<float>;

struct UIRGB {
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;
};

struct UIFillRGB {
	UIRGB top;
	UIRGB bottom;
};

struct UIRGBConfig {
	UIFillRGB textColor;
	UIFillRGB background;
	UIFillRGB border;
};


struct {
	const UIFillRGB standardTextColor = { { 150, 150, 250 }, { 200, 200, 250 } };
	const UIFillRGB darkerTextColor = { { 120, 120, 220 }, { 180, 180, 230 } };

	const UIFillRGB standardFill = { {10,10,200}, {30,30,250} };
	const UIFillRGB standardBorder = { {50,50,250}, {20,20,200} };

	const UIFillRGB brighterFill = { {30,30,220}, {50,50,255} };
	const UIFillRGB brighterBorder = { {80,80,255}, {50,50,250} };

	const UIFillRGB darkerFill = { {10,10,200}, {0,0,120} };
	const UIFillRGB darkerBorder = { {30,30,230}, {10,10,150} };

	UIRGBConfig colorsIdle;
	UIRGBConfig colorsHover;
	UIRGBConfig colorsActive;

	void setup() {
		colorsIdle.background = standardFill;
		colorsIdle.border = standardBorder;
		colorsIdle.textColor = standardTextColor;

		colorsActive.background = brighterFill;
		colorsActive.border = darkerBorder;
		colorsActive.textColor = standardTextColor;

		colorsHover.background = darkerFill;
		colorsHover.border = brighterBorder;
		colorsHover.textColor = darkerTextColor;
	}
} UIPreface;




struct Vec3F {
	float x, y, z;

	void Print() {
		Log.printf("[ %f\t%f\t%f ]\n", x, y, z);
	}

	void normalize() {
		float l = len();
		x /= l;
		y /= l;
		z /= l;
	}

	float len() const {
		return sqrt(x * x + y * y + z * z);
	}
};

struct M44 {
	float m[4][4] = { 0 };

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

	void asTranslate(GLfloat x, GLfloat y, GLfloat z) {
		m[0][0] = 1; m[0][1] = 0; m[0][2] = 0; m[0][3] = x;
		m[1][0] = 0; m[1][1] = 1; m[1][2] = 0; m[1][3] = y;
		m[2][0] = 0; m[2][1] = 0; m[2][2] = 1; m[2][3] = z;
		m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;
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

	void Print() const {
		Log.printf("[ %f\t%f\t%f\t%f ]\n", m[0][0], m[0][1], m[0][2], m[0][3]);
		Log.printf("[ %f\t%f\t%f\t%f ]\n", m[1][0], m[1][1], m[1][2], m[1][3]);
		Log.printf("[ %f\t%f\t%f\t%f ]\n", m[2][0], m[2][1], m[2][2], m[2][3]);
		Log.printf("[ %f\t%f\t%f\t%f ]\n", m[3][0], m[3][1], m[3][2], m[3][3]);
	}

	Vec3F ApplyOnPoint(Vec3F& p) const {
		float nX = m[0][0] * p.x + m[0][1] * p.y + m[0][2] * p.z + m[0][3];
		float nY = m[1][0] * p.x + m[1][1] * p.y + m[1][2] * p.z + m[1][3];
		float nZ = m[2][0] * p.x + m[2][1] * p.y + m[2][2] * p.z + m[2][3];

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

	void print() const {
		Log.printf(
			"VENDOR:   %s\n"
			"RENDERER: %s\n"
			"VERSION:  %s\n"
			"SELECTED: %i.%i\n",

			nameVendor.c_str(), nameRenderer.c_str(), nameVersion.c_str(), major, minor);
	}
};


struct AppContext {
	int windowWidth = 600;
	int windowHeight = 600;

	float pointerSpeed = 0.2;

	SDL_GLContext glcontext;
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
	string lastError;
	OpenGLProperties openglProperties;

	bool mouseCaptureMode;

	bool startSDL();
	void stopSDL();
	void mouseCapture(bool newState) {
		SDL_SetWindowRelativeMouseMode(App.window, newState);

		if (mouseCaptureMode && false == newState) {
			SDL_WarpMouseInWindow(window, (float)(windowWidth / 2), (float)(windowHeight / 2));
		}

		mouseCaptureMode = newState;
	}
} App;

bool AppContext::startSDL() {
	SDL_Init(SDL_INIT_VIDEO);
	if (!SDL_GL_LoadLibrary(nullptr)) {
		lastError = "Failed to load GL library";
		return false;
	}

	int requestedValue = 8;
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, requestedValue);

	window = SDL_CreateWindow("Explorer3D", windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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

struct TextPainterContext {
	const string FontMap = ""
		"abcdefghijklm\n"
		"nopqrstuvwxyz\n"
		"ABCDEFGHIJKLM\n"
		"NOPQRSTUVWXYZ\n"
		"0123456789!@#\n"
		"$%^&*()-=_+[]\n"
		"{};':\",.<>|/\\?";

	GLuint fontTextName = 0;
	int fontCharHeight = 0;
	int fontCharWidth = 0;


	void charCoord(const char c, int& x, int& y) {
		// it just stops and state of x/y is taken as output
		x = 0;
		y = 0;
		for (auto ptr = FontMap.cbegin(); ptr < FontMap.cend(); ++ptr) {
			if (*ptr == c) {
				return;
			}
			if (*ptr == '\n') {
				y += 1;
				x = 0;
			}
			else {
				x += 1;
			}
		}

		// not found case: move to last character; which should be ?
		--x;
	}

	void textSize(const string& text, int& width, int& height) {
		textSizeChars(text, width, height);
		width *= fontCharWidth;
		height *= fontCharHeight;
	}

	void textSizeChars(const string& text, int& width, int& height) {
		int lineMax = 0;
		int x = 0;
		int y = 1;
		for (auto ptr = text.cbegin(); ptr < text.cend(); ++ptr) {
			++x;
			if (*ptr == '\n') {
				y++;
				lineMax = max<int>(x, lineMax);
			}
		}

		width = max<int>(x, lineMax);
		height = y;
	}

	void drawString(const string& text) {
		UIFillRGB color = { { 255,255,255 }, {180,180,180} };
		drawStringColor(text, color);
	}

	const int TAB_SIZE = 8;
	void drawStringColor(const string& text, const UIFillRGB& color) {
		float oX = 0;
		float oY = 0;

		int cX = 0;
		int cY = 0;

		float tX = 0;
		float tY = 0;

		float tUnit = 1.0 / 128.0;
		float tW = fontCharWidth * tUnit;
		float tH = fontCharHeight * tUnit;

		for (auto c = text.cbegin(); c < text.cend(); ++c) {
			if (*c == '\t') {
				int xIdx = oX / fontCharWidth;
				int tabbedIdx = (xIdx / TAB_SIZE + 1) * TAB_SIZE;
				oX = tabbedIdx * fontCharWidth;

			}
			else if (*c == '\n') {
				oX = 0;
				oY += fontCharHeight;
			}
			else if (*c == ' ') {
				oX += fontCharWidth;
			}
			else {
				charCoord(*c, cX, cY);
				tX = cX * fontCharWidth * tUnit;
				tY = cY * fontCharHeight * tUnit;

				glBegin(GL_QUADS);

				glColor3ub(color.top.r, color.top.g, color.top.b);
				glColor3ub(color.top.r, color.top.g, color.top.b);
				glTexCoord2f(tX + tW, tY);		glVertex2f(oX + fontCharWidth, oY);
				glTexCoord2f(tX, tY);			glVertex2f(oX, oY);
				glColor3ub(color.bottom.r, color.bottom.g, color.bottom.b);
				glTexCoord2f(tX, tY + tH);		glVertex2f(oX, oY + fontCharHeight);
				glTexCoord2f(tX + tW, tY + tH); glVertex2f(oX + fontCharWidth, oY + fontCharHeight);


				glEnd();

				oX += fontCharWidth;
			}
		}
	}

	void drawStringAt(string text, float x, float y) {
		glPushMatrix();
		glTranslatef(x, y, 0);

		drawString(text);

		glPopMatrix();
	}

	void bindTexture() {
		glBindTexture(GL_TEXTURE_2D, TextPainter.fontTextName);
	}


} TextPainter;

float calculateCenter(float space, float box) {
	return (space - box) / 2;
}

enum UIRectState {
	UICursorIdle,
	UICursorHover,
	UICursorActive,
};

struct UITrigger {
	virtual void onAction(int id) = 0;
};

struct UIRect {
	XYFloat pos;
	XYFloat textPos;
	XYFloat size;

	string text;
	int id = -1;

	UIRGBConfig* configHover;
	UIRGBConfig* configActive;
	UIRGBConfig* configIdle;

	UITrigger* actionEvent = nullptr;
	UIRGBConfig* currentState = nullptr;

	UIRectState state = UICursorIdle;

	void cursorAt(XYFloat &cursor) {
		// active cursor is controlled by buttonAt
		if (state == UICursorActive) {
			return;
		}

		if (inRange(cursor)) {
			updateState(UICursorHover);
		}
		else {
			updateState(UICursorIdle);
		}
	}

	void buttonAt(XYFloat cursor, int button, bool down) {
		if (button != SDL_BUTTON_LEFT) {
			return;
		}
		
		if (inRange(cursor)) {
			if (down && state != UICursorActive) {
				updateState(UICursorActive);
			}
			else if (!down && state == UICursorActive) {
				updateState(UICursorHover);
				if (actionEvent) {
					actionEvent->onAction(id);
				}
			}
		}
		else if (!down && state == UICursorActive) {
			updateState(UICursorIdle);
		}
	}

	bool inRange(XYFloat& cursor) const {
		return pos.x <= cursor.x && pos.x + size.x > cursor.x && pos.y <= cursor.y && pos.y + size.y > cursor.y;
	}

	void drawBorder() const {
		const UIFillRGB* c = &currentState->border;

		glBegin(GL_LINE_LOOP);

		glColor3ub(c->bottom.r, c->bottom.g, c->bottom.b);
		glVertex3f(0, size.y, 0.0);
		glVertex3f(size.x, size.y, 0.0);

		glColor3ub(c->top.r, c->top.g, c->top.b);
		glVertex3f(size.x, 0, 0.0);
		glVertex3f(0, 0, 0.0);

		glEnd();
	}

	void drawBackground() const {

		const UIFillRGB* c = &currentState->background;

		glBegin(GL_QUADS);

		glColor3ub(c->bottom.r, c->bottom.g, c->bottom.b);
		glVertex3f(0, size.y, 0.0);
		glVertex3f(size.x, size.y, 0.0);

		glColor3ub(c->top.r, c->top.g, c->top.b);
		glVertex3f(size.x, 0, 0.0);
		glVertex3f(0, 0, 0.0);

		glEnd();
	}

	void updateState(UIRectState newState) {
		state = newState;
		switch (state) {
		case UICursorIdle: currentState = configIdle; return;
		case UICursorActive: currentState = configActive; return;
		case UICursorHover: currentState = configHover; return;
		}
	}

	void render() const {
		if (!currentState) {
			Log.printf("[UI ERROR] id: %i has no state set, rendering aborted\n", id);
			return;
		}
		glTranslatef(pos.x, pos.y, 0);

		glDisable(GL_TEXTURE_2D);
		drawBackground();
		drawBorder();
		glEnable(GL_TEXTURE_2D);

		glTranslatef(textPos.x, textPos.y, 0);
		TextPainter.drawStringColor(text, currentState->textColor);
	}

	void centerText() {
		int textWidth;
		int textHeight;
		TextPainter.textSize(text, textWidth, textHeight);

		textPos.x = calculateCenter(size.x, textWidth);
		textPos.y = calculateCenter(size.y, textHeight);
	}
};

struct UIGroup {
	vector<UIRect> parts;

	void cursorAt(XYFloat cursor) {
		XYFloat relativeCursor{ cursor.x - x, cursor.y - y };

		for (auto each = parts.begin(); each < parts.end(); ++each) {
			each->cursorAt(relativeCursor);
		}
	}

	void buttonAt(XYFloat cursor, int button, bool down) {
		XYFloat relativeCursor{ cursor.x - x, cursor.y - y };

		for (auto each = parts.begin(); each < parts.end(); ++each) {
			each->buttonAt(relativeCursor, button, down);
		}
	}

	float x = 0;
	float y = 0;

	void render() const {
		glTranslatef(x, y, 0);
		for (auto each = parts.cbegin(); each < parts.cend(); ++each) {
			glPushMatrix();
			each->render();
			glPopMatrix();
		}
	}
};

enum MainUI_IDs {
	SINGLEVIEW_ID = 100,
	MULTIVIEW_ID,
	CAMERARESET_ID,
	DISPLAYCOORDS_ID
};

struct Camera {
	float frustumRight = 0;
	float frustumTop = 0;
	GLdouble fov = 60;

	Vec3F pos = { 0,0,0 };
	Vec3F angle = { 0,0,0 };

	XYFloat viewPos = { 0,0 };
	XYFloat viewSize = { 0,0 };

	void reset() {
		pos = { 0,0,0 };
		angle = { 0,0,0 };
		fov = 0;
	}

	void displayCoords() const {
		Log.printf("[ %.3f %.3f %.3f ], [%.3f %.3f %.3f]\n", pos.x, pos.y, pos.z, angle.x, angle.y, angle.z);
	}
};

struct DrawPlane : UITrigger {
	Camera camera;

	vector<pair<Vec3F, Vec3F>> lines;
	MovementStrategy movement = MoveHybrid;
	const int fovDiff = 1;
	const float fovMax = 160;
	const float fovMin = 5;

	const float nearPlane = 0.1;
	const float farPlane = 10;

	int frames = 0;

	const float moveSpeed = 0.02;

	int moveAlongX = 0;
	int moveAlongY = 0;
	int moveAlongZ = 0;
	int rotateZ = 0;

	UIGroup mainUI;

	const int framesForMessage = 120;
	int endOfMessageFrame = 0;

	bool multiViewEnabled = false;

	void showMessages() {
		if (endOfMessageFrame == 0 && Log.unreadMessages > 0) {
			endOfMessageFrame = framesForMessage;
		}
		
		auto ptr = Log.history.cend();
		for (int i = Log.unreadMessages - 1; i >= 0; --i) {
			--ptr;

			glPushMatrix();
			glTranslatef(0, i * TextPainter.fontCharHeight + 2, 0);
			TextPainter.drawString(*ptr);
			glPopMatrix();
		}
		

		endOfMessageFrame -= 1;
		if (endOfMessageFrame == 0) {
			Log.unreadMessages -= 1;
		}
	}

	void makeRectRGB(int toSide, unsigned char* from, unsigned char* to) {
		int fromLineLen = toSide * 2 * 4;
		int toLineLen = toSide * 4;

		for (int y = 0; y < toSide; ++y) {
			for (int x = 0; x < toSide; ++x) {
				int toPtr = y * toLineLen + x * 4;

				if (x == 0 || y == 0 || x == toSide - 1 || y == toSide - 1) {
					to[toPtr + 0] = 255;
					to[toPtr + 1] = 0;
					to[toPtr + 2] = 0;
					to[toPtr + 3] = 255;
				}

				else {
					to[toPtr + 0] = 0;
					to[toPtr + 1] = 255;
					to[toPtr + 2] = 255;
					to[toPtr + 3] = 100;
				}

			}
		}
	}

	void scaleDownRGB(int toSide, unsigned char* from, unsigned char* to) {
		int fromLineLen = toSide * 2 * 4;
		int toLineLen = toSide * 4;

		for (int y = 0; y < toSide; ++y) {
			for (int x = 0; x < toSide; ++x) {
				int toPtr = y * toLineLen + x * 4;

				int r = 0;
				int g = 0;
				int b = 0;
				int a = 0;

				for (int py = 0; py < 2; ++py) for (int px = 0; px < 2; ++px) {
					int idx = (y * 2 + py) * fromLineLen + (x * 2 + px) * 4;

					r += from[idx + 0];
					g += from[idx + 1];
					b += from[idx + 2];
					a += from[idx + 3];
				}

				r /= 4;
				g /= 4;
				b /= 4;
				a /= 4;

				to[toPtr + 0] = r;
				to[toPtr + 1] = g;
				to[toPtr + 2] = b;
				to[toPtr + 3] = a;
			}
		}
	}

	// TODO: it requires ground rework as for smaller maps, it shows fainted colors due to averaging;
	// it should treat base color with higher weight
	void createMipmap(int side, unsigned char* source) {
		int a = side / 2;
		if (a == 0) {
			return;
		}

		if (a > 512 || a < 0) {
			cerr << "[ERROR] createMipmap with value not from [0,512], got : " << a << endl;
			return;
		}

		int bufferSize = a * a * 4;

		unique_ptr<unsigned char[]> b1 = make_unique<unsigned char[]>(bufferSize);
		unique_ptr<unsigned char[]> b2 = make_unique<unsigned char[]>(bufferSize);

		unsigned char* buffer = b1.get();
		unsigned char* fromBuffer = source;

		int face = 1;

		while (a) {
			scaleDownRGB(a, fromBuffer, buffer);
			glTexImage2D(GL_TEXTURE_2D, face, GL_RGBA, a, a, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

			if (face % 2 == 1) {
				fromBuffer = b1.get();
				buffer = b2.get();
			}
			else {
				fromBuffer = b2.get();
				buffer = b1.get();
			}

			a /= 2;
			face += 1;
		}
	}

	void loadFontTexture() {
		SDL_Surface* surface = IMG_Load("c:/share/Charmap128.png");
		if (!surface) {
			cerr << "[ERROR] failed to load font texture; file not found\n";
			return;
		}

		Log.printf("W: %i %i\n", surface->w, surface->h);
		Log.printf("Surface format: 0x%x\n", surface->format);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glGenTextures(1, &TextPainter.fontTextName);
		TextPainter.bindTexture();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
		SDL_DestroySurface(surface);

		TextPainter.fontCharHeight = 18;
		TextPainter.fontCharWidth = 9;
	}

	void init() {
		loadFontTexture();
		setupUI();

		onResize();
	}

	void onResize() {
		camera.viewSize = { (float)App.windowWidth, (float)App.windowHeight };
	}

	void onAction(int uiId) {
		MainUI_IDs id = (MainUI_IDs)uiId;

		switch (id) {
		case SINGLEVIEW_ID: onSingleView(); break;
		case MULTIVIEW_ID: onMultiView(); break;
		case CAMERARESET_ID: onCameraReset(); break;
		case DISPLAYCOORDS_ID: onDisplayCoords(); break;
		default:
			Log.printf("Unsupported event id %i\n", uiId); break;
		}
	}

	void onSingleView() {
		camera.viewSize = { (float)(App.windowWidth), (float)(App.windowHeight) };
		Log.printf("Single View\n");
		multiViewEnabled = false;
	}

	void onMultiView() {
		camera.viewSize = { (float)(App.windowWidth / 2), (float)(App.windowHeight / 2) };
		Log.printf("Multi View\n");
		multiViewEnabled = true;
	}

	void onCameraReset() {
		camera.reset();

		updateFov(camera, 60);
	}
	
	void addManyButtons(const vector<string> &names, int startingId, XYFloat startingPos, vector<UIRect> &buttonsSink) {
		int id = startingId;
		XYFloat pos = startingPos;

		for (auto name = names.cbegin(); name < names.cend(); ++name) {
			UIRect button;

			button.pos = pos;

			button.text = *name;
			button.size = { 150, 30 };
			button.centerText();
			button.id = id;
			button.actionEvent = this;

			button.configIdle = &UIPreface.colorsIdle;
			button.configActive = &UIPreface.colorsActive;
			button.configHover = &UIPreface.colorsHover;
			button.updateState(UICursorIdle);

			id += 1;
			pos.y += 35;

			buttonsSink.push_back(button);
		}
	}

	void setupUI() {
		mainUI.x = 100;
		mainUI.y = 100;

		vector<string> names = { "Single View", "Multi View", "Reset camera", "Display coords"};
		addManyButtons(names, SINGLEVIEW_ID, { 0,0 }, mainUI.parts);
	}

	void onDisplayCoords() {
		camera.displayCoords();
	}

	pair<Vec3F, Vec3F> traceLine(const Camera& c, const float x, const float y) {
		float xRatio = x / c.viewSize.x * 2 - 1;
		float yRatio = y / c.viewSize.y * 2 - 1;

		float pRight = xRatio * c.frustumRight; // minus xRatio because we rotate along Y axis
		float pTop = -yRatio * c.frustumTop;

		M44 m; m.asRotateX(0);

		M44 mX; mX.asRotateX(rad(c.angle.x));
		M44 mY; mY.asRotateY(rad(c.angle.y));
		M44 mZ; mZ.asRotateZ(rad(c.angle.z));

		M44 mT; mT.asTranslate(c.pos.x, c.pos.y, c.pos.z);

		m.Mult(mT);

		m.Mult(mY);
		m.Mult(mX);
		m.Mult(mZ);

		pair<Vec3F, Vec3F> p;

		Vec3F lineStart = { 0,0,0 };
		Vec3F lineEnd = { pRight,pTop,-nearPlane };
		lineEnd.normalize();
		lineEnd.x *= farPlane;
		lineEnd.y *= farPlane;
		lineEnd.z *= farPlane;
		p.first = m.ApplyOnPoint(lineStart);
		p.second = m.ApplyOnPoint(lineEnd);

		return p;
	}

	void applyMovesXYZ(Camera &c) {
		Vec3F vRotated = { 0,0,0 };
		Vec3F v = { moveAlongX * moveSpeed, 0, moveAlongZ * moveSpeed };

		M44 m; m.asRotateX(0);
		M44 mX;	mX.asRotateX(rad(c.angle.x));
		M44 mY; mY.asRotateY(rad(c.angle.y));

		m.Mult(mY);
		m.Mult(mX);

		vRotated = m.ApplyOnPoint(v);

		c.pos.x += vRotated.x;
		c.pos.y += moveAlongY * moveSpeed;
		c.pos.z += vRotated.z;
	}

	void applyMovesHybrid(Camera &c) {
		Vec3F vRotated = { 0,0,0 };
		Vec3F v = { moveAlongX * moveSpeed, moveAlongY * moveSpeed, moveAlongZ * moveSpeed };

		M44 m; m.asRotateX(0);

		M44 mX; mX.asRotateX(rad(c.angle.x));
		M44 mY; mY.asRotateY(rad(c.angle.y));
		M44 mZ; mZ.asRotateZ(rad(c.angle.z));

		m.Mult(mY);
		m.Mult(mX);
		m.Mult(mZ);

		vRotated = m.ApplyOnPoint(v);
		c.pos.x += vRotated.x;
		c.pos.y += vRotated.y;
		c.pos.z += vRotated.z;
	}

	void applyMoves(Camera &c) {
		if (rotateZ) {
			c.angle.z += 0.8 * rotateZ;
		}

		if (moveAlongX == 0 && moveAlongZ == 0 && moveAlongY == 0) {
			return;
		}

		if (movement == MoveXYZ) {
			applyMovesXYZ(c);
		}
		else if (movement == MoveHybrid || movement == MoveFreespace) {
			applyMovesHybrid(c);
		}
	}

	void pointerUpdateHybridXYZ(Camera &c, float dX, float dY) {
		// sic! - moving left-right (pointer X) rotates around axis Y, and pointer Y around axis X
		c.angle.y += App.pointerSpeed * -dX;
		c.angle.x += App.pointerSpeed * -dY;

		if (c.angle.x < -90) c.angle.x = -90;
		if (c.angle.x > 90) c.angle.x = 90;
	}

	void pointerUpdateFreespace(Camera &c, float dX, float dY) {
		float oY = App.pointerSpeed * -dX;
		float oX = App.pointerSpeed * -dY;

		// apply current rotations
		M44 mX; mX.asRotateX(rad(c.angle.x));
		M44 mY; mY.asRotateY(rad(c.angle.y));
		M44 mZ; mZ.asRotateZ(rad(c.angle.z));

		M44 mOY; mOY.asRotateY(rad(oY));
		M44 mOX; mOX.asRotateX(rad(oX));

		M44 m; m.asRotateX(0);

		m.Mult(mY);
		m.Mult(mX);
		m.Mult(mZ);

		m.Mult(mOY);
		m.Mult(mOX);


		Vec3F fwd = { 0,0,-1 };
		Vec3F up = { 0,1,0 };

		Vec3F nFwd = m.ApplyOnPoint(fwd);
		Vec3F nUp = m.ApplyOnPoint(up);

		vectorsToAngles(c, nFwd, nUp);
	}

	void vectorsToAngles(Camera &c, Vec3F& fwd, Vec3F& up) {

		float radY = atan2(-fwd.x, -fwd.z);

		M44 revY; revY.asRotateY(-radY);
		Vec3F rotatedX = revY.ApplyOnPoint(fwd);

		float radX = atan2(rotatedX.y, -rotatedX.z);

		c.angle.x = deg(radX);
		c.angle.y = deg(radY);

		M44 mX; mX.asRotateX(rad(-c.angle.x));
		M44 mY; mY.asRotateY(rad(-c.angle.y));

		M44 m; m.asRotateX(0);

		m.Mult(mX);
		m.Mult(mY);

		Vec3F revUp = m.ApplyOnPoint(up);

		float radZ = atan2(-revUp.x, revUp.y);
		c.angle.z = deg(radZ);
	}

	void pointerUpdate(Camera &c, float dX, float dY) {
		if (movement == MoveHybrid || movement == MoveXYZ) {
			pointerUpdateHybridXYZ(c, dX, dY);
		}
		else if (movement == MoveFreespace) {
			pointerUpdateFreespace(c, dX, dY);
		}
	}

	void cursorUpdate(float x, float y) {
		mainUI.cursorAt({ x,y });
	}

	void cursorButton(float x, float y, int idx, bool down) {
		mainUI.buttonAt({ x,y }, idx, down);
	}

	void updateFov(Camera &c, float newFov) {
		c.fov = max<float>(min<float>(newFov, fovMax), fovMin);
	}

	void updateProjection(Camera &c) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		bool perspetive = true;
		if (perspetive) {

			GLdouble right;
			GLdouble top;
			GLdouble aspectRatio;
			GLdouble tangent;

			aspectRatio = (1.0 * c.viewSize.x) / c.viewSize.y;
			tangent = tan(rad(c.fov / 2));

			top = tangent * nearPlane;
			right = top;
			if (c.viewSize.x > c.viewSize.y) {
				right *= aspectRatio;
			}
			else {
				top /= aspectRatio;
			}

			glFrustum(-right, right, -top, top, nearPlane, farPlane);
			c.frustumRight = right;
			c.frustumTop = top;
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

	void eyeCoords(const Camera c) const {
		glRotatef(-c.angle.z, 0, 0, 1);
		glRotatef(-c.angle.x, 1, 0, 0);
		glRotatef(-c.angle.y, 0, 1, 0);
	}

	void texturedPlane() {
		// playing with texture 
		{
			glPushMatrix();
			glEnable(GL_BLEND);
			glEnable(GL_TEXTURE_2D);
			glTranslatef(0, 0, -2);
			TextPainter.bindTexture();
			glColor4f(1, 1, 1, 1);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			{
				glBegin(GL_QUADS);

				glTexCoord2f(0.0, 1.0);  glVertex3f(-1.0, -1.0, 0.0);
				glTexCoord2f(0.0, 0.0);  glVertex3f(-1.0, 1.0, 0.0);
				glTexCoord2f(1.0, 0.0);  glVertex3f(1.0, 1.0, 0.0);
				glTexCoord2f(1.0, 1.0);  glVertex3f(1.0, -1.0, 0.0);

				glEnd();
			}
			GLenum err = glGetError();
			if (err) { Log.printf("[ ERROR ] %i\n", err); }

			glDisable(GL_BLEND);
			glDisable(GL_TEXTURE_2D);
			glPopMatrix();
		}
	}

	// x=0;y=0 => top left corner
	void enterPixelToPixel2D(float w, float h) {
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, w, h, 0, -1, 1);

		glMatrixMode(GL_MODELVIEW);

		glPushAttrib(GL_ENABLE_BIT);
		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);

		glLoadIdentity();
	}

	void leavePixelToPixel2D() {
		glPopAttrib();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}

	void renderScene(Camera& c) {
		applyMoves(c);

		glViewport(c.viewPos.x, c.viewPos.y, c.viewPos.x + c.viewSize.x, c.viewPos.y + c.viewSize.y);
		updateProjection(c);
	
		glClearColor(0, 0, 0, 1);
		glShadeModel(GL_SMOOTH);
		glClear(GL_COLOR_BUFFER_BIT);

		eyeCoords(c);

		glTranslatef(-c.pos.x, -c.pos.y, -c.pos.z);

		glColor3f(0.3, 0.3, 0.3);
		drawGrid(0);

		glColor3f(0.3, 0.3, 0.5);
		drawGrid(3);

		glPushMatrix();
		glTranslatef(0, 0, -3);
		drawQuad();
		glPopMatrix();

		texturedPlane();

		glBegin(GL_LINES);
		for (auto p = lines.cbegin(); p < lines.cend(); ++p) {
			glColor3f(0, 1, 1); glVertex3f(p->first.x, p->first.y, p->first.z);
			glColor3f(1, 1, 0); glVertex3f(p->second.x, p->second.y, p->second.z);
		}
		glEnd();
	}

	void frame() {
		glLoadIdentity();

		++frames;
	
		renderScene(camera);


		if (!App.mouseCaptureMode)  {
			enterPixelToPixel2D(App.windowWidth, App.windowHeight);
			mainUI.render();
			leavePixelToPixel2D();
		}
		
		if (Log.unreadMessages > 0) {
			enterPixelToPixel2D(App.windowWidth, App.windowHeight);
			showMessages();
			leavePixelToPixel2D();
		}

		SDL_GL_SwapWindow(App.window);
	}

	void drawQuad() const {
		glPushMatrix();
		GLdouble spin = 0.2 * frames;

		glRotatef(spin, 0, 1, 0);
		glRotatef(45, 1, 0, 0);


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
		Log.printf("Failed to start, error: %i\n", App.lastError);
		return 1;
	}

	App.openglProperties.print();
	UIPreface.setup();

	bool showEvent = false;
	DrawPlane d;
	d.init();

	App.mouseCapture(true);

	const int FPS = 60;
	int milis = 1000 / FPS;
	d.movement = MoveXYZ;
	for (;;) {
		SDL_Event event;
		if (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
				SDL_MouseButtonEvent* mouseEvent = (SDL_MouseButtonEvent*)&event;
				if (mouseEvent->button == SDL_BUTTON_LEFT && !App.mouseCaptureMode) {
					d.lines.push_back(d.traceLine(d.camera, mouseEvent->x, mouseEvent->y));
					XYFloat cursor = { mouseEvent->x, mouseEvent->y };
					d.mainUI.buttonAt(cursor, SDL_BUTTON_LEFT, false);
				}
				if (mouseEvent->button == SDL_BUTTON_RIGHT) {
					App.mouseCapture(!App.mouseCaptureMode);
				}
			}
			else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
				SDL_MouseButtonEvent* mouseEvent = (SDL_MouseButtonEvent*)&event;
				if (mouseEvent->button == SDL_BUTTON_LEFT && !App.mouseCaptureMode) {
					XYFloat cursor = { mouseEvent->x, mouseEvent->y };
					d.mainUI.buttonAt(cursor, SDL_BUTTON_LEFT, true);
				}
			}
			else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
				SDL_WindowEvent* windowEvent = (SDL_WindowEvent*)&event;
				App.windowWidth = windowEvent->data1;
				App.windowHeight = windowEvent->data2;
				d.onResize();
			}
			else if (event.type == SDL_EVENT_MOUSE_MOTION) {
				SDL_MouseMotionEvent* mouseEvent = (SDL_MouseMotionEvent*)&event;
				if (App.mouseCaptureMode) {
					d.pointerUpdate(d.camera, mouseEvent->xrel, mouseEvent->yrel);
				}
				else {
					d.cursorUpdate(mouseEvent->x, mouseEvent->y);
				}
			}
			else if (event.type == SDL_EVENT_KEY_DOWN) {
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
				else if (keyEvent->key == SDLK_Q) {
					d.rotateZ = 1;
				}
				else if (keyEvent->key == SDLK_E) {
					d.rotateZ = -1;
				}
				else if (keyEvent->key == SDLK_SPACE) {
					d.moveAlongY = 1;
				}
				else if (keyEvent->key == SDLK_LSHIFT) {
					d.moveAlongY = -1;
				}
			}
			else if (event.type == SDL_EVENT_KEY_UP) {
				SDL_KeyboardEvent* keyEvent = (SDL_KeyboardEvent*)&event;
				if (keyEvent->key == SDLK_GRAVE) {
					Log.printf("CONSOLE\n");
				}
				else if (keyEvent->key == SDLK_KP_PLUS) {
					d.updateFov(d.camera, d.camera.fov + d.fovDiff);
					Log.printf("FOV: %f\n", d.camera.fov);
				}
				else if (keyEvent->key == SDLK_KP_MINUS) {
					d.updateFov(d.camera, d.camera.fov - d.fovDiff);
					Log.printf("FOV: %f\n", d.camera.fov);
				}
				else if (keyEvent->key == SDLK_A || keyEvent->key == SDLK_D) {
					d.moveAlongX = 0;
				}
				else if (keyEvent->key == SDLK_W || keyEvent->key == SDLK_S) {
					d.moveAlongZ = 0;
				}
				else if (keyEvent->key == SDLK_Q || keyEvent->key == SDLK_E) {
					d.rotateZ = 0;
				}
				else if (keyEvent->key == SDLK_SPACE || keyEvent->key == SDLK_LSHIFT) {
					d.moveAlongY = 0;
				}
				else if (keyEvent->key == SDLK_8) {
					d.movement = MoveHybrid;
					Log.printf("Movement: hybrid\n");
				}
				else if (keyEvent->key == SDLK_9) {
					d.movement = MoveXYZ;
					d.camera.angle.z = 0;
					Log.printf("Movement: xyz\n");
				}
				else if (keyEvent->key == SDLK_0) {
					d.movement = MoveFreespace;
					Log.printf("Movement: freespace\n");
				}
			}
			else if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
				break;
			}
			else if (showEvent) {
				Log.printf("Event %i\n", event.type);
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
