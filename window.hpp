#ifndef HEADER_WINDOW
#define HEADER_WINDOW

// post-context GLEW initialisation
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include "gl/glew.h"

#include "SDL2/SDL.h" // context creation
#include "SDL2/SDL_opengl.h" // OpenGL options

#include <map> // input binding
#include <ctime> // update rate
#include <vector> // multiple bindings

#define WINDOW_KEYCODES (128 + 226)
#define WINDOW_MOUSECODES 8

#define INPUT_SENS_KEY 0.01f
#define INPUT_SENS_MOUSE 0.01f
#define INPUT_SENS_SCROLL 0.05f

#define WINDOW_RATES 2

enum WindowFlag{
	WindowResize = SDL_WINDOW_RESIZABLE, 
	WindowGraphic = SDL_WINDOW_OPENGL
};

enum WindowRate{
	WindowDisplay, 
	WindowInput
};

enum WindowState{
	WindowError = -2, 
	WindowQuit = -1, 
	WindowDefault, 
	WindowResizing, 
	WindowResized, 
	WindowEnter, 
	WindowLeave, 
	WindowFocus, 
	WindowUnfocus
};

enum WindowKey{
	KeyA = SDLK_a, 
	KeyB = SDLK_b, 
	KeyC = SDLK_c, 
	KeyD = SDLK_d, 
	KeyE = SDLK_e, 
	KeyF = SDLK_f, 
	KeyG = SDLK_g, 
	KeyH = SDLK_h, 
	KeyI = SDLK_i, 
	KeyJ = SDLK_j, 
	KeyK = SDLK_k, 
	KeyL = SDLK_l, 
	KeyM = SDLK_m, 
	KeyN = SDLK_n, 
	KeyO = SDLK_o, 
	KeyP = SDLK_p, 
	KeyQ = SDLK_q, 
	KeyR = SDLK_r, 
	KeyS = SDLK_s, 
	KeyT = SDLK_t, 
	KeyU = SDLK_u, 
	KeyV = SDLK_v, 
	KeyW = SDLK_w, 
	KeyX = SDLK_x, 
	KeyY = SDLK_y, 
	KeyZ = SDLK_z, 
	KeySpace = SDLK_SPACE, 
	KeyLeftControl = SDLK_LCTRL, 
	KeyTab = SDLK_TAB, 
	KeyLeftAlt = SDLK_LALT, 
};

enum WindowButton{
	MouseLeftClick = SDL_BUTTON_LEFT, 
	MouseRightClick = SDL_BUTTON_RIGHT
};

enum ErrorWindow{ErrorNone, 
	ErrorSDLInit, ErrorSDLWindow, ErrorSDLContext, ErrorGLEWInit, // window
	ErrorMouseDown, ErrorMouseUp, ErrorKeyDown, ErrorKeyUp // input
};

union ErrorContext{
	void *pointer;
	int value;
	GLenum glValue;
};

class Window{
	
	// context
	int width, height;
	SDL_Window *window;
	SDL_GLContext context;
	
	// time
	clock_t timeStart;
	clock_t timePeriod[WINDOW_RATES]; // frame, input
	clock_t timePrev[WINDOW_RATES];
	
	// error
	ErrorWindow errorStatus;
	ErrorContext errorContext;
	
	// input
public:
	int keyMap[WINDOW_KEYCODES];
	int mouseMap[WINDOW_MOUSECODES];
	float mouseMotion[2], mousePosition[2];
	
public:
	
	// setup
	Window(const char *name, Uint32 flags, int w, int h, float frameRate, float inputRate);
	~Window();
	void timer();
	
	// update
	bool cap(WindowRate type);
	WindowState get();
	void clear() const;
	void swap() const;
	void focus();
	void lockCursor();
	void unlockCursor();
	
	// properties
	void setErrorStatus(ErrorWindow code = ErrorNone, ErrorContext data = {0});
	std::string getErrorStatus() const;
	float getAspectRatio() const;
	void getViewport(float &x, float &y) const;
	void getScreenSpace(float &w, float &h, float &cx, float &cy) const;
	
	// input
	int& getMapHandle(WindowKey code);
	int& getMapHandle(WindowButton code);
	float (&getMouseMotionHandle())[2];
	float (&getMousePositionHandle())[2];
};

class InputBind{
	std::map<int, int*> bindings;
	float (&motion)[2], (&position)[2];
	bool isActive;
	template <typename T> void bindAll(std::vector<std::pair<int, T>> const &bindings, Window &w);
public:
	InputBind(float (&m)[2], float (&p)[2]);
	template <typename T> void bind(int id, T code, Window &w);
	void bindAll(std::vector<std::pair<int, WindowKey>> const &bindings, Window &w);
	void bindAll(std::vector<std::pair<int, WindowButton>> const &bindings, Window &w);
	void setActive(bool a);
	int getInactivePress(int id);
	int getPress(int id);
	int getHold(int id);
	void flush();
	void getMouseMotion(float (&m)[2]);
	void getMousePosition(float (&p)[2]);
};

#endif