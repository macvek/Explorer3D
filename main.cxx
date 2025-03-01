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
#include <log.h>
#include <trig.h>
#include <m44.h>

using namespace std;

float calculateCenter(float space, float box) {
	return (space - box) / 2;
}

template <typename T> struct XYGeneric {
	T x = 0;
	T y = 0;
};

using XYFloat = XYGeneric<float>;

struct UIEvent {

	UIEvent(XYFloat aCursor, int aButton, bool aDown) : cursor(aCursor), button(aButton), down(aDown) {}

	XYFloat cursor;
	int button;
	bool down;
	bool captured = false;
};

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
	int windowWidth = 1200;
	int windowHeight = 600;

	float pointerSpeed = 0.2;

	SDL_GLContext glcontext;
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
	string lastError;
	OpenGLProperties openglProperties;

	SDL_Cursor* cursorDefault;
	SDL_Cursor* cursorPointer;

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

	cursorDefault = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
	cursorPointer = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
	
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
		for (auto ptr = FontMap.cbegin(); ptr != FontMap.cend(); ++ptr) {
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
		for (auto ptr = text.cbegin(); ptr != text.cend(); ++ptr) {
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

		for (auto c = text.cbegin(); c != text.cend(); ++c) {
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

	void buttonAt(UIEvent &e) {
		if (e.button != SDL_BUTTON_LEFT) {
			return;
		}
		
		if (inRange(e.cursor)) {
			if (e.down && state != UICursorActive) {
				updateState(UICursorActive);
			}
			else if (!e.down && state == UICursorActive) {
				updateState(UICursorHover);
				if (actionEvent) {
					actionEvent->onAction(id);
					e.captured = true;
				}
			}
		}
		else if (!e.down && state == UICursorActive) {
			updateState(UICursorIdle);
			e.captured = true;
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

	void buttonAt(UIEvent &e) {
		XYFloat relativeCursor{ e.cursor.x - x, e.cursor.y - y };
		XYFloat origCursor = e.cursor;
		e.cursor = relativeCursor;

		for (auto each = parts.begin(); each < parts.end(); ++each) {
			each->buttonAt(e);
		}

		e.cursor = origCursor;
	}

	float x = 0;
	float y = 0;

	void render() const {
		glTranslatef(x, y, 0);
		for (auto each = parts.cbegin(); each != parts.cend(); ++each) {
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
	DISPLAYCOORDS_ID,
	CHANGE_CURSOR_ID,
	RUN_HITTEST_ID,
};

enum AllViews {
	VIEW_NONE,
	VIEW_CAMERA,
	VIEW_XZ,
	VIEW_XY,
	VIEW_ZY,
};

struct ClippingRange {
	float left;
	float right;
	float bottom;
	float top;
};

struct Camera {
	static const float fovMax;
	static const float fovMin;

	float nearPlane = 0.1;
	float farPlane = 10;
	
	bool perspective = true;
	bool adjustOrtho = false;
	float frustumRight = 0;
	float frustumTop = 0;
	GLdouble fov = 60;

	Vec3F pos = { 0,0,0 };
	Vec3F angle = { 0,0,0 };

	XYFloat viewPos = { 0,0 };
	XYFloat viewSize = { 0,0 };

	ClippingRange orthoRange = { -1, 1, -1,1 };
	float zoomFactor = 1;
	
	void applyCameraWheel(float dy) {
		if (perspective) {
			updateFov(fov -dy);
		}
		else if (adjustOrtho) {
			zoomFactor -= 0.02 * dy;
		}
	}

	void applyCameraDrag(float dx, float dy) {
		if (perspective) {
			return;
		}

		XYFloat unit = pixelRange();
		Vec3F posDiff = { -dx * unit.x, -dy * unit.y, 0 };

		posDiff = M44F().asRotateX(rad(angle.x)).ApplyOnPoint(posDiff);
		posDiff = M44F().asRotateY(rad(angle.y)).ApplyOnPoint(posDiff);
		posDiff = M44F().asRotateZ(rad(angle.z)).ApplyOnPoint(posDiff);

		pos.add(posDiff);

	}

	void updateFov(float newFov) {
		fov = max<float>(min<float>(newFov, fovMax), fovMin);
	}

	XYFloat pixelRange() const {
		ClippingRange r = calculateClippingRange();
		return {
			(r.right - r.left) / viewSize.x,
			(r.bottom - r.top) / viewSize.y,
		};
	}

	void applyViewport() {
		glViewport(viewPos.x, viewPos.y, viewSize.x, viewSize.y);
	}

	ClippingRange calculateClippingRange() const {
		if (!perspective && !adjustOrtho) {
			return orthoRange;
		}

		ClippingRange c;
		GLdouble aspectRatio;
		aspectRatio = (1.0 * viewSize.x) / viewSize.y;

		if (perspective) {
			GLdouble tangent = tan(rad(fov / 2));

			c.top = tangent * nearPlane;
			c.right = c.top;
			if (viewSize.x > viewSize.y) {
				c.right *= aspectRatio;
			}
			else {
				c.top /= aspectRatio;
			}
		}
		else {
			if (viewSize.x > viewSize.y) {
				c.right = orthoRange.right;
				c.top = orthoRange.top / aspectRatio;
			}
			else {
				c.right = orthoRange.right * aspectRatio;
				c.top = orthoRange.top;
			}

			c.right *= zoomFactor;
			c.top *= zoomFactor;
		}

		c.left = -c.right;
		c.bottom = -c.top;

		return c;

	}

	void applyProjection() {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		ClippingRange c = calculateClippingRange();

		if (perspective) {
			glFrustum(c.left, c.right, c.bottom, c.top, nearPlane, farPlane);
			frustumRight = c.right;
			frustumTop = c.top;
		}
		else {
			glOrtho(c.left, c.right, c.bottom, c.top, nearPlane, farPlane);
		}
	}

	void eyeCoords() const {
		glRotatef(-angle.z, 0, 0, 1);
		glRotatef(-angle.x, 1, 0, 0);
		glRotatef(-angle.y, 0, 1, 0);

		glTranslatef(-pos.x, -pos.y, -pos.z);
	}

	void reset() {
		pos = { 0,0,0 };
		angle = { 0,0,0 };
		fov = 0;
	}

	void displayCoords() const {
		Log.printf("[ %.3f %.3f %.3f ], [%.3f %.3f %.3f]\n", pos.x, pos.y, pos.z, angle.x, angle.y, angle.z);
	}
};

const float Camera::fovMax = 175;
const float Camera::fovMin = 5;

struct Line {
	Vec3F first;
	Vec3F second;
};

struct Triangle {
	int id;
	Vec3F vertices[3];

	Vec3F normal() const {
		Vec3F a = vertices[2];
		Vec3F b = vertices[2];

		a.sub(vertices[0]);
		b.sub(vertices[1]);

		return a.crossProduct(b);
	}
};

struct HitPosition {
	int id;
	Vec3F v;
};

struct Quad {
	int id;
	Vec3F vertices[4];

	pair<Triangle, Triangle> asTris() {
		pair<Triangle, Triangle> p;
		p.first = { id, {vertices[0], vertices[1], vertices[2]} };
		p.second = { id, {vertices[2], vertices[3], vertices[0]} };
		return p;
	}
};

struct HitTest {
	vector<Triangle> tris;
	vector<HitPosition> hits;
	Line line;

	// return 1 if point is above line, -1 if is below, 0 if is on the line or outside of X axis
	int overUnderLine(Vec3F& a, Vec3F& b) const{
		// out of X axis
		if (a.x < 0 && b.x < 0 || a.x > 0 && b.x > 0) {
			return 0;
		}

		// both points are below (Y grows upwards) , so [0,0] is above
		if (a.y < 0 && b.y < 0) {
			return 1;
		}

		// both points are above, so [0,0] is below
		if (a.y > 0 && b.y > 0) {
			return -1;
		}

		// calculate f(0) for line between a and b
		float lA = (b.y - a.y) / (b.x - a.x);
		float lB = a.y - lA * a.x;

		if (lB < 0) return 1;  // point [0,0] is above line hiting X
		if (lB > 0) return -1;
		return 0;
	}

	int pairIsOut(Vec3F& a, Vec3F& b) const {
		if (a.x < 0 && b.x < 0 || a.x > 0 && b.x > 0) {
			return 1;
		}
		else {
			return 0;
		}
	}

	// checks if point [0,0] is inside triangle a,b,c ignoring Z axis ; it will check it for infinite length of trace line, so requires distance check
	bool hitTriangle(Triangle &t) const {
		Vec3F& a = t.vertices[0];
		Vec3F& b = t.vertices[1];
		Vec3F& c = t.vertices[2];
		int outOfX = pairIsOut(a,b) + pairIsOut(b,c) + pairIsOut(a,c);
		if (outOfX == 3) { // it is either 0,1 or 3 for triangle
			return false;
		}
		
		int ab = overUnderLine(a, b);
		int ac = overUnderLine(a, c);
		int bc = overUnderLine(b, c);

		int sum = ab + ac + bc;

		// if is out then will reach -2 or 2 or in corner case of sharing X with vertex => -3 or 3
		return sum >= -1 && sum <= 1;
	}

	// hit is calculated for point [0,0]; triangle is shifted in Z axis; calculate distance to triangle plain on Z axis
	float distanceFromCenter(const Triangle& t) {
		Vec3F n = t.normal();
		// triangle plain equation is n.x*X + n.y*Y + n.z*Z + D = 0 ;
		// to calculate D i pick first vertex from triangle, and then for Z it is n.x*0 + n.y*0 + n.z*Z + D = 0 => Z = -D/n.z

		Vec3F v = t.vertices[0];
		float D = -(n.x*v.x + n.y*v.y + n.z*v.z);

		float ret = -D / n.z;
		// value returned is -ret, because triangle lays on negative plain 
		Log.printf("DIST FROM CENTER: %f\n", -ret);
		return -ret;
	}

	bool check() {
		Vec3F dir = line.second;
		dir.sub(line.first);

		Vec3F angles = dir.rotationYXZ(Vec3F::UP);

		// Align all objects along 'line' so all calculations are along x,y axises
		M44F m;
		m.Mult(M44F().asRotateX(-angles.x))
			.Mult(M44F().asRotateY(-angles.y))
			.Mult(M44F().asTranslate(-line.first.x, -line.first.y, -line.first.z));

		bool ret = false;
		for (auto t = tris.cbegin(); t != tris.cend(); t++) {
			Vec3F a = m.ApplyOnPoint(t->vertices[0]);
			Vec3F b = m.ApplyOnPoint(t->vertices[1]);
			Vec3F c = m.ApplyOnPoint(t->vertices[2]);
			
			Triangle mT = { t->id, {a,b,c} };

			if (hitTriangle(mT)) {
				float distance = distanceFromCenter(mT);
				
				// distance < 0 means that triange is BEHIND relative point [0,0]
				if (distance > 0) {
					ret = true;
					// calculate intersection point, i.e. point on triangle
					hits.push_back({ t->id, {0,0, distance } });
				}

			}
		}

		// sort hits in Z order, figure out the strategy for multiple hits if triangle should hold it or ordered set of all handlers should decide if it passes or not
		// rotate all points back to universal coordinates

		return ret;
	}
};

struct Renderable {
	
	Vec3F pos = { 0,0,0 };
	Vec3F angle = { 0,0,0 };
	Vec3F scale = { 1,1,1 };

	bool showSingle = true;
	GLubyte facesIndices[24] = {
			0,1,2,3, // -z
			4,5,6,7, // +z

			0,1,5,4, // -x
			2,3,7,6, // +x

			1,2,6,5, // -y
			0,3,7,4, // +y
	};

	GLfloat vertices[24] = {
			-1, 1,-1,
			-1,-1,-1,
			 1,-1,-1,
			 1, 1,-1,

			-1, 1, 1,
			-1,-1, 1,
			 1,-1, 1,
			 1, 1, 1,
	};

	GLfloat colors[24] = {
			1.0, 0.0, 0.0,
			0.0, 0.1, 0.0,
			0.0, 0.0, 1.0,
			1.0, 1.0, 0.0,

			0.0, 1.0, 1.0,
			1.0, 0.0, 1.0,
			1.0, 1.0, 1.0,
			0.3, 0.3, 0.3,
	};

	void mesh(vector<Triangle> &fill) {
		M44F m;
		m.asTranslate(pos.x, pos.y, pos.z)
			.Mult(M44F().asRotateZ(angle.z))
			.Mult(M44F().asRotateX(angle.x))
			.Mult(M44F().asRotateY(angle.y))
			.Mult(M44F().asScale(scale.x, scale.y, scale.z));

		

		Vec3F a = { vertices[0], vertices[1], vertices[2] };
		Vec3F b = { vertices[3], vertices[4], vertices[5] };
		Vec3F c = { vertices[6], vertices[7], vertices[8] };
		
		Triangle t = {
			 99, {m.ApplyOnPoint(a),m.ApplyOnPoint(b),m.ApplyOnPoint(c)}
		};

		fill.push_back(t);
	}

	void render(int frames) const {
		// first approach - fixed cube rendering
		glPushMatrix();
		
		bool glMatrix = false;
		if (glMatrix) {
			glTranslatef(pos.x, pos.y, pos.z);

			glRotatef(angle.z, 0, 0, 1);
			glRotatef(angle.x, 1, 0, 0);
			glRotatef(angle.y, 0, 1, 0);

			glScalef(scale.x, scale.y, scale.z);
		}
		else {
			M44F m;
			m.asTranslate(pos.x, pos.y, pos.z)
				.Mult(M44F().asRotateZ(angle.z))
				.Mult(M44F().asRotateX(angle.x))
				.Mult(M44F().asRotateY(angle.y))
				.Mult(M44F().asScale(scale.x, scale.y, scale.z));

			glMultMatrixf(m.ptr());
		}

		glColorPointer(3, GL_FLOAT, 0, colors);
		glVertexPointer(3, GL_FLOAT, 0, vertices);

		if (showSingle) {
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, facesIndices);
		}
		else {
			glDrawElements(GL_QUADS, 6 * 4, GL_UNSIGNED_BYTE, facesIndices);
		}
		glPopMatrix();
	}
};

struct DrawPlane : UITrigger {
	Camera camera;
	Camera consoleView;

	Camera cameraXZ;
	Camera cameraXY;
	Camera cameraZY;

	Camera cameraAngles;

	AllViews focusView;

	list<Line> lines;
	vector<Renderable> renderables;

	MovementStrategy movement = MoveHybrid;
	const int fovDiff = 1;

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

	XYFloat dragXY;
	bool dragging = false;
	

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

	void setupConsoleView() {
		consoleView.farPlane = -1;
		consoleView.nearPlane = 1;
		consoleView.perspective = false;
	}

	void setupXYZCameras() {
		cameraXZ.perspective = false;
		cameraXZ.adjustOrtho = true;
		cameraXZ.orthoRange = { -5,5,-5,5 };
		cameraXZ.farPlane = 100;
		cameraXZ.nearPlane = -100;
		cameraXZ.angle = { -90,0,0 };

		cameraXY = cameraXZ;
		cameraXY.angle = { 0,0,0 };

		cameraZY = cameraXZ;
		cameraZY.angle = { 0,-90,0 };

		cameraAngles.perspective = false;
		cameraAngles.orthoRange = { -1,1,-1,1 };
		cameraAngles.farPlane = 100;
		cameraAngles.nearPlane = -100;
		cameraAngles.viewSize = { 64,64 };
	}

	void init() {
		loadFontTexture();
		setupConsoleView();
		setupXYZCameras();
		setupUI();
		onResize();

		Renderable r;
		r.showSingle = true;
		r.pos = { 5,1,-5 };
		r.angle = { 45,45,45 };
		r.scale = { 1,1,1 };
		renderables.push_back(r);

		//lines.push_back({{ 0.366,0.360, -3.835 }, { 9.869, -0.461, -6.839 }});
	}

	void onResize() {
		camera.viewSize = { (float)App.windowWidth, (float)App.windowHeight };
		
		consoleView.viewSize = { (float)App.windowWidth, (float)App.windowHeight };
		consoleView.orthoRange = { 0, consoleView.viewSize.x, consoleView.viewSize.y, 0 };
	}

	void onAction(int uiId) {
		MainUI_IDs id = (MainUI_IDs)uiId;

		switch (id) {
		case SINGLEVIEW_ID: onSingleView(); break;
		case MULTIVIEW_ID: onMultiView(); break;
		case CAMERARESET_ID: onCameraReset(); break;
		case DISPLAYCOORDS_ID: onDisplayCoords(); break;
		case CHANGE_CURSOR_ID: onChangeCursor(); break;
		case RUN_HITTEST_ID: onRunHittest(); break;
		default:
			Log.printf("Unsupported event id %i\n", uiId); break;
		}
	}

	void onSingleView() {
		Log.printf("Single View\n");
		multiViewEnabled = false;
	}

	void onMultiView() {
		Log.printf("Multi View\n");
		multiViewEnabled = true;
	}

	void onCameraReset() {
		camera.reset();

		camera.updateFov(60);
	}
	
	void addManyButtons(const vector<string> &names, int startingId, XYFloat startingPos, vector<UIRect> &buttonsSink) {
		int id = startingId;
		XYFloat pos = startingPos;

		for (auto name = names.cbegin(); name != names.cend(); ++name) {
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

		vector<string> names = { 
			"Single View", 
			"Multi View", 
			"Reset camera", 
			"Display coords", 
			"Change cursor",
			"Run Hittest"
		};
		addManyButtons(names, SINGLEVIEW_ID, { 0,0 }, mainUI.parts);
	}

	void onDisplayCoords() {
		camera.displayCoords();
	}
	
	void onChangeCursor() {
		SDL_SetCursor(App.cursorPointer);
	}

	void onRunHittest() {
		if (lines.empty()) {
			return;
		}
		
		HitTest ht;
		ht.line = lines.front();
		lines.pop_front();

		renderables[0].mesh(ht.tris);

		int response = ht.check();
		Log.printf("ht.check -> %i\n", response);
	}

	Line traceLine(const Camera& c, const float x, const float y) {
		int updatedY = App.windowHeight - (c.viewPos.y + c.viewSize.y) + y; 
		float xRatio = x / c.viewSize.x * 2 - 1;
		float yRatio = updatedY / c.viewSize.y * 2 - 1;

		float pRight = xRatio * c.frustumRight; // minus xRatio because we rotate along Y axis
		float pTop = -yRatio * c.frustumTop;

		M44F m;
		m
			.Mult(M44F().asTranslate(c.pos.x, c.pos.y, c.pos.z))
			.Mult(M44F().asRotateY(rad(c.angle.y)))
			.Mult(M44F().asRotateX(rad(c.angle.x)))
			.Mult(M44F().asRotateZ(rad(c.angle.z)));

		Vec3F lineStart = { 0,0,0 };
		Vec3F lineEnd = { pRight,pTop,-c.nearPlane };
		lineEnd.normalize().mult(c.farPlane);

		Line l = { m.ApplyOnPoint(lineStart) , m.ApplyOnPoint(lineEnd) };
		Log.printf("Line [%.3f %.3f %.3f] [%.3f %.3f %.3f]\n", l.first.x, l.first.y, l.first.z, l.second.x, l.second.y, l.second.z);
		return l;
	}

	void applyMovesXYZ(Camera &c) {
		Vec3F vRotated = { 0,0,0 };
		Vec3F v = { moveAlongX * moveSpeed, 0, moveAlongZ * moveSpeed };

		M44F m;
		m
			.Mult(M44F().asRotateY(rad(c.angle.y)))
			.Mult(M44F().asRotateX(rad(c.angle.x)));

		vRotated = m.ApplyOnPoint(v);
		vRotated.y = moveAlongY * moveSpeed;

		c.pos.add(vRotated);
	}

	void applyMovesHybrid(Camera &c) {
		Vec3F vRotated = { 0,0,0 };
		Vec3F v = { moveAlongX * moveSpeed, moveAlongY * moveSpeed, moveAlongZ * moveSpeed };

		M44F m; 
		m
			.Mult(M44F().asRotateY(rad(c.angle.y)))
			.Mult(M44F().asRotateX(rad(c.angle.x)))
			.Mult(M44F().asRotateZ(rad(c.angle.z)));

		vRotated = m.ApplyOnPoint(v);
		c.pos.add(vRotated);
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

		M44F m;
		m.Mult(M44F().asRotateY(rad(c.angle.y)))
			.Mult(M44F().asRotateX(rad(c.angle.x)))
			.Mult(M44F().asRotateZ(rad(c.angle.z)))
			.Mult(M44F().asRotateY(rad(oY)))
			.Mult(M44F().asRotateX(rad(oX)));
		

		Vec3F fwd = { 0,0,-1 };
		Vec3F up = { 0,1,0 };

		Vec3F nFwd = m.ApplyOnPoint(fwd);
		Vec3F nUp = m.ApplyOnPoint(up);

		vectorsToAngles(c, nFwd, nUp);
	}

	void vectorsToAngles(Camera &c, Vec3F& fwd, Vec3F& up) {
		Vec3F rotationVector = fwd.rotationYXZ(up);
		
		c.angle.x = deg(rotationVector.x);
		c.angle.y = deg(rotationVector.y);
		c.angle.z = deg(rotationVector.z);
	}

	void pointerUpdate(Camera &c, float dX, float dY) {
		if (movement == MoveHybrid || movement == MoveXYZ) {
			pointerUpdateHybridXYZ(c, dX, dY);
		}
		else if (movement == MoveFreespace) {
			pointerUpdateFreespace(c, dX, dY);
		}
	}

	void cursorUpdate(float x, float y, float dx, float dy) {
		mainUI.cursorAt({ x,y });
		if (multiViewEnabled && dragging) {
			cameraAtXY(dragXY).applyCameraDrag(dx, dy);
		}
	}

	Camera& cameraAtXY(XYFloat xy) {
		if (multiViewEnabled) {
			bool left = xy.x < App.windowWidth / 2;
			bool top = xy.y < App.windowHeight / 2;

			if (left) {
				return top ? camera : cameraXY;
			}
			else {
				return top ? cameraXZ : cameraZY;
			}
		}
		else {
			return camera;
		}
	}

	void handleDragging(XYFloat xy, bool down) {
		if (!dragging && down || dragging && !down) {
			dragging = !dragging;
			dragXY = xy;

			SDL_SetCursor(dragging ? App.cursorPointer : App.cursorDefault);
			Log.printf("Dragging %i at {%f,%f}\n", dragging, dragXY.x, dragXY.y);
		}
	}

	void cursorButton(XYFloat xy, int idx, bool down) {
		UIEvent e(xy, idx, down);

		mainUI.buttonAt(e);
		
		if (SDL_BUTTON_MIDDLE == idx) {
			handleDragging(xy, down);
		}

		if (!e.down && !e.captured) {
			lines.push_back(traceLine(camera, e.cursor.x, e.cursor.y));
		}
	}

	void cursorWheel(float dy, XYFloat xy) {
		cameraAtXY(xy).applyCameraWheel(dy);
	}

	void renderTexturedPlane() {
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

	void renderAngles(Camera& anglesOf) {
		cameraAngles.viewPos = anglesOf.viewPos;
		cameraAngles.angle = anglesOf.angle;
		cameraAngles.applyViewport();
		cameraAngles.applyProjection();

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		cameraAngles.eyeCoords();

		glBegin(GL_LINES);
		glColor4f(1, 0, 0, 1);
		glVertex3f(0, 0, 0);
		glVertex3f(1, 0, 0);

		glColor4f(0, 1, 0, 1);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 1, 0);

		glColor4f(0, 0, 1, 1);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, 1);
		glEnd();

	}

	void renderTraceLines() {
		glBegin(GL_LINES);
		for (auto p = lines.cbegin(); p != lines.cend(); ++p) {
			glColor3f(0, 1, 1); glVertex3f(p->first.x, p->first.y, p->first.z);
			glColor3f(1, 1, 0); glVertex3f(p->second.x, p->second.y, p->second.z);
		}
		glEnd();
	}

	void renderRenderables() const{
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		for (auto ptr = renderables.cbegin(); ptr < renderables.cend(); ++ptr) {
			ptr->render(frames);
		}
	}

	void renderScene(Camera& c) {
		c.applyViewport();
		c.applyProjection();

		glMatrixMode(GL_MODELVIEW);
		glShadeModel(GL_SMOOTH);
		glLoadIdentity();

		c.eyeCoords();
		glColor3f(0.3, 0.3, 0.3);
		drawGrid(0);

		glColor3f(0.3, 0.3, 0.5);
		drawGrid(3);
		
		glEnable(GL_DEPTH_TEST);
		
		glPushMatrix();
		glTranslatef(0, 0, -3);
		drawQuad();
		glPopMatrix();

		renderTexturedPlane();
		renderTraceLines();

		renderRenderables();
		glDisable(GL_DEPTH_TEST);
	}

	void renderOverlay2D() {
		consoleView.applyViewport();
		consoleView.applyProjection();

		glMatrixMode(GL_MODELVIEW);
		glPushAttrib(GL_ENABLE_BIT);
		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);

		if (!App.mouseCaptureMode) {
			glLoadIdentity();
			mainUI.render();
		}

		if (Log.unreadMessages > 0) {
			glLoadIdentity();
			showMessages();
		}

		glPopAttrib();
	}

	void frame() {
		++frames;
		
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		applyMoves(camera);

		if (multiViewEnabled) {
			XYFloat viewSize = { (float)(App.windowWidth / 2), (float)(App.windowHeight / 2) };
			camera.viewSize = viewSize;
			camera.viewPos = { 0,viewSize.y };
			renderScene(camera);
			renderAngles(camera);

			cameraXZ.viewSize = viewSize;
			cameraXZ.viewPos = { viewSize.x, viewSize.y };
			renderScene(cameraXZ);
			renderAngles(cameraXZ);

			cameraXY.viewSize = viewSize;
			cameraXY.viewPos = { 0, 0 };
			renderScene(cameraXY);
			renderAngles(cameraXY);

			cameraZY.viewSize = viewSize;
			cameraZY.viewPos = { viewSize.x, 0 };
			renderScene(cameraZY);
			renderAngles(cameraZY);
		}
		else {
			camera.viewSize = { (float)(App.windowWidth), (float)(App.windowHeight) };
			camera.viewPos = { 0,0 };
			renderScene(camera);
		}

		renderOverlay2D();

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
		glBegin(GL_LINES);
		for (int x = -10; x <= 10; ++x) {
			glVertex3f(x, y, -10);
			glVertex3f(x, y, 10);
		}

		for (int z = -10; z <= 10; ++z) {
			glVertex3f(-10, y, z);
			glVertex3f(10, y, z);
		}
		glEnd();
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
				if ((mouseEvent->button == SDL_BUTTON_LEFT || mouseEvent->button == SDL_BUTTON_MIDDLE) && !App.mouseCaptureMode) {
					d.cursorButton({ mouseEvent->x, mouseEvent->y }, mouseEvent->button, false);

				}
				if (mouseEvent->button == SDL_BUTTON_RIGHT) {
					App.mouseCapture(!App.mouseCaptureMode);
				}
			}
			else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
				SDL_MouseButtonEvent* mouseEvent = (SDL_MouseButtonEvent*)&event;
				if ( (mouseEvent->button == SDL_BUTTON_LEFT || mouseEvent->button == SDL_BUTTON_MIDDLE) && !App.mouseCaptureMode) {
					d.cursorButton({ mouseEvent->x, mouseEvent->y }, mouseEvent->button, true);
				}
			}
			else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
				SDL_MouseWheelEvent* mouseEvent = (SDL_MouseWheelEvent*)&event;
				d.cursorWheel(mouseEvent->y, { mouseEvent->mouse_x, mouseEvent->mouse_y});
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
					d.cursorUpdate(mouseEvent->x, mouseEvent->y, mouseEvent->xrel, mouseEvent->yrel);
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
					d.camera.updateFov(d.camera.fov + d.fovDiff);
					Log.printf("FOV: %f\n", d.camera.fov);
				}
				else if (keyEvent->key == SDLK_KP_MINUS) {
					d.camera.updateFov(d.camera.fov - d.fovDiff);
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
