#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h> // only include this one in the source file with main()!
#include <SDL3_image/SDL_image.h>

#include <iostream>
#include <vector>

#include <Windows.h>
#include <gl/gl.h>

#include <cmath>

#define M_PI       3.14159265358979323846

using namespace std;

struct UIXY {
	float x = 0;
	float y = 0;
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

struct Vec3F {
	float x, y, z;

	void Print() {
		cout << " [ " << x << "\t" << y << "\t" << z << " ] " << endl;
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

	void Print() {
		cout << " [ " << m[0][0] << "\t" << m[0][1] << "\t" << m[0][2] << "\t" << m[0][3] << " ] " << endl;
		cout << " [ " << m[1][0] << "\t" << m[1][1] << "\t" << m[1][2] << "\t" << m[1][3] << " ] " << endl;
		cout << " [ " << m[2][0] << "\t" << m[2][1] << "\t" << m[2][2] << "\t" << m[2][3] << " ] " << endl;
		cout << " [ " << m[3][0] << "\t" << m[3][1] << "\t" << m[3][2] << "\t" << m[3][3] << " ] " << endl;
	}

	Vec3F ApplyOnPoint(Vec3F& p) {
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

	bool mouseCaptureMode;

	bool startSDL();
	void stopSDL();
	void mouseCapture(bool newState) {
		SDL_SetWindowRelativeMouseMode(App.window, newState);

		if (mouseCaptureMode && false == newState) {
			SDL_WarpMouseInWindow(window, width / 2, height / 2);
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
			if (*c == '\n') {
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

struct UIRect {
	UIXY pos;
	UIXY textPos;
	UIXY size;

	string text;
	int id = -1;

	UIFillRGB textColor;
	UIFillRGB background;
	UIFillRGB border;

	void drawBorder() const {
		const UIFillRGB* c = &border;

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

		const UIFillRGB* c = &background;

		glBegin(GL_QUADS);

		glColor3ub(c->bottom.r, c->bottom.g, c->bottom.b);
		glVertex3f(0, size.y, 0.0);
		glVertex3f(size.x, size.y, 0.0);

		glColor3ub(c->top.r, c->top.g, c->top.b);
		glVertex3f(size.x, 0, 0.0);
		glVertex3f(0, 0, 0.0);

		glEnd();
	}

	void render() const {
		glTranslatef(pos.x, pos.y, 0);

		glDisable(GL_TEXTURE_2D);
		drawBackground();
		drawBorder();
		glEnable(GL_TEXTURE_2D);

		glTranslatef(textPos.x, textPos.y, 0);
		TextPainter.drawStringColor(text, textColor);
	}

	float calculateCenter(float space, float box) {
		return (space - box) / 2;
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

struct DrawPlane {

	vector<pair<Vec3F, Vec3F>> lines;
	MovementStrategy movement = MoveHybrid;
	const int fovDiff = 1;
	const float fovMax = 160;
	const float fovMin = 5;

	const float nearPlane = 0.1;
	const float farPlane = 10;

	float frustumRight = 0;
	float frustumTop = 0;

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
	int rotateZ = 0;

	bool refresh = false;

	UIGroup mainUI;


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

		cout << "W: " << surface->w << " " << surface->h << "\n";
		printf("0x%x\n", surface->format);

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

		glViewport(0, 0, App.width, App.height);
		refresh = true;

		//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	}

	void setupUI() {
		mainUI.x = 100;
		mainUI.y = 100;

		UIFillRGB standardFill = { {10,10,200}, {30,30,250} };
		UIFillRGB standardBorder = { {50,50,250}, {20,20,200} };

		UIRect button;

		button.background = standardFill;

		button.border = standardBorder;

		button.pos = { 4,4 };

		button.text = "Sample button";
		button.size = { 150, 30 };
		button.textColor = { { 150, 150, 250 }, { 200, 200, 250 } };

		button.centerText();

		UIRect otherButton;
		otherButton = button;
		otherButton.text = "Create Cube";
		otherButton.centerText();
		otherButton.pos.y += otherButton.size.y + 10;

		mainUI.parts.push_back(button);
		mainUI.parts.push_back(otherButton);
	}

	pair<Vec3F, Vec3F> traceLine(float x, float y) {
		float xRatio = x / App.width * 2 - 1;
		float yRatio = y / App.height * 2 - 1;

		float pRight = xRatio * frustumRight; // minus xRatio because we rotate along Y axis
		float pTop = -yRatio * frustumTop;

		M44 m; m.asRotateX(0);

		M44 mX; mX.asRotateX(rad(aX));
		M44 mY; mY.asRotateY(rad(aY));
		M44 mZ; mZ.asRotateZ(rad(aZ));

		M44 mT; mT.asTranslate(posX, posY, posZ);

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
		p.first.Print();
		p.second.Print();

		return p;
	}

	void applyMovesXYZ() {
		Vec3F vRotated = { 0,0,0 };
		Vec3F v = { moveAlongX * moveSpeed, 0, moveAlongZ * moveSpeed };

		M44 m; m.asRotateX(0);
		M44 mX;	mX.asRotateX(rad(aX));
		M44 mY; mY.asRotateY(rad(aY));

		m.Mult(mY);
		m.Mult(mX);

		vRotated = m.ApplyOnPoint(v);

		posX += vRotated.x;
		posY += moveAlongY * moveSpeed;
		posZ += vRotated.z;
	}

	void applyMovesHybrid() {
		Vec3F vRotated = { 0,0,0 };
		Vec3F v = { moveAlongX * moveSpeed, moveAlongY * moveSpeed, moveAlongZ * moveSpeed };

		M44 m; m.asRotateX(0);

		M44 mX; mX.asRotateX(rad(aX));
		M44 mY; mY.asRotateY(rad(aY));
		M44 mZ; mZ.asRotateZ(rad(aZ));

		m.Mult(mY);
		m.Mult(mX);
		m.Mult(mZ);

		vRotated = m.ApplyOnPoint(v);
		posX += vRotated.x;
		posY += vRotated.y;
		posZ += vRotated.z;
	}

	void applyMoves() {
		if (rotateZ) {
			aZ += 0.8 * rotateZ;
		}

		if (moveAlongX == 0 && moveAlongZ == 0 && moveAlongY == 0) {
			return;
		}

		if (movement == MoveXYZ) {
			applyMovesXYZ();
		}
		else if (movement == MoveHybrid || movement == MoveFreespace) {
			applyMovesHybrid();
		}
	}

	void pointerUpdateHybridXYZ(float dX, float dY) {
		// sic! - moving left-right (pointer X) rotates around axis Y, and pointer Y around axis X
		aY += App.pointerSpeed * -dX;
		aX += App.pointerSpeed * -dY;

		if (aX < -90) aX = -90;
		if (aX > 90) aX = 90;
	}

	void pointerUpdateFreespace(float dX, float dY) {
		float oY = App.pointerSpeed * -dX;
		float oX = App.pointerSpeed * -dY;

		// apply current rotations
		M44 mX; mX.asRotateX(rad(aX));
		M44 mY; mY.asRotateY(rad(aY));
		M44 mZ; mZ.asRotateZ(rad(aZ));

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

		vectorsToAngles(nFwd, nUp);
	}

	void vectorsToAngles(Vec3F& fwd, Vec3F& up) {

		float radY = atan2(-fwd.x, -fwd.z);

		M44 revY; revY.asRotateY(-radY);
		Vec3F rotatedX = revY.ApplyOnPoint(fwd);

		float radX = atan2(rotatedX.y, -rotatedX.z);

		aX = deg(radX);
		aY = deg(radY);

		M44 mX; mX.asRotateX(rad(-aX));
		M44 mY; mY.asRotateY(rad(-aY));

		M44 m; m.asRotateX(0);

		m.Mult(mX);
		m.Mult(mY);

		Vec3F revUp = m.ApplyOnPoint(up);

		float radZ = atan2(-revUp.x, revUp.y);
		aZ = deg(radZ);
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

	void updateProjection(float width, float height) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		bool perspetive = true;
		if (perspetive) {

			GLdouble right;
			GLdouble top;
			GLdouble aspectRatio;
			GLdouble tangent;

			aspectRatio = (1.0 * width) / height;
			tangent = tan(rad(fov / 2));

			top = tangent * nearPlane;
			right = top;
			if (width > height) {
				right *= aspectRatio;
			}
			else {
				top /= aspectRatio;
			}

			glFrustum(-right, right, -top, top, nearPlane, farPlane);
			frustumRight = right;
			frustumTop = top;
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

	void eyeCoords() const {
		glRotatef(-aZ, 0, 0, 1);
		glRotatef(-aX, 1, 0, 0);
		glRotatef(-aY, 0, 1, 0);
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
			if (glGetError()) { cout << "ERR 1\n"; }

			glDisable(GL_BLEND);
			glDisable(GL_TEXTURE_2D);
			glPopMatrix();
		}
	}

	// x=0;y=0 => top left corner
	void enterPixelToPixel2D() {
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, App.width, App.height, 0, -1, 1);

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

	void frame() {
		glLoadIdentity();

		++frames;
		applyMoves();

		if (refresh) {
			refresh = false;
			updateProjection(App.width, App.height);
		}
		glClearColor(0, 0, 0, 1);
		glShadeModel(GL_SMOOTH);
		glClear(GL_COLOR_BUFFER_BIT);

		eyeCoords();

		glTranslatef(-posX, -posY, -posZ);

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

		enterPixelToPixel2D();
		mainUI.render();
		leavePixelToPixel2D();

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
		cout << "Failed to start, error: " << App.lastError << endl;
		return 1;
	}

	App.openglProperties.toStream(cout);

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
					d.lines.push_back(d.traceLine(mouseEvent->x, mouseEvent->y));
				}
				if (mouseEvent->button == SDL_BUTTON_RIGHT) {
					App.mouseCapture(!App.mouseCaptureMode);
				}
			}

			if (event.type == SDL_EVENT_WINDOW_RESIZED) {
				SDL_WindowEvent* windowEvent = (SDL_WindowEvent*)&event;
				App.width = windowEvent->data1;
				App.height = windowEvent->data2;
				d.init();
			}

			if (event.type == SDL_EVENT_MOUSE_MOTION) {
				SDL_MouseMotionEvent* mouseEvent = (SDL_MouseMotionEvent*)&event;
				if (App.mouseCaptureMode) {
					d.pointerUpdate(mouseEvent->xrel, mouseEvent->yrel);
				}
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
				else if (keyEvent->key == SDLK_Q || keyEvent->key == SDLK_E) {
					d.rotateZ = 0;
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
					d.aZ = 0;
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
