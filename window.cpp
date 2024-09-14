#include "window.hpp"

// internal methods

namespace{
	int keysym(int code){
		int key = code;
		if(key >= 128) key -= 0x40000039;
		if(key < 0) return WINDOW_KEYCODES;
		return key + 128;
	}
}

// setup methods

Window::Window(const char *name, Uint32 flags, int w, int h, float frameRate, float inputRate){
	setErrorStatus(ErrorNone);
	
	// SDL
	window = NULL;
	width = w;
	height = h;
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
		setErrorStatus(ErrorSDLInit);
		return;
	}
	if((window = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags)) == NULL){
		setErrorStatus(ErrorSDLWindow);
		return;
	}
	
	// SDL OpenGL
	context = NULL;
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	if((context = SDL_GL_CreateContext(window)) == NULL){
		setErrorStatus(ErrorSDLContext);
		return;
	}
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	// OpenGL
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(.2f, .2f, .2f, 1.f);
	
	// GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if(err != GLEW_OK){
		setErrorStatus(ErrorGLEWInit, ErrorContext{.glValue = err});
		return;
	}
	
	// time
	timeStart = 0;
	timePrev[0] = timePrev[1] = 0;
	timePeriod[0] = CLOCKS_PER_SEC * (1.f / frameRate);
	timePeriod[1] = CLOCKS_PER_SEC * (1.f / inputRate);
	
	// input
	for(int i = 0; i < WINDOW_KEYCODES; i++) keyMap[i] = 0;
	for(int i = 0; i < WINDOW_MOUSECODES; i++) mouseMap[i] = 0;
	mousePosition[0] = mousePosition[1] = 0;
	mouseMotion[0] = mouseMotion[1] = 0;
	
	// focus
	SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
}

Window::~Window(){
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Window::timer(){
	timeStart = clock();
	for(int i = 0; i < WINDOW_RATES; i++) timePrev[i] = timeStart - timePeriod[i];
}

// update methods

bool Window::cap(WindowRate type){
	if(clock() - timePrev[type] > timePeriod[type]){
		timePrev[type] += timePeriod[type];
		return true;
	}
	return false;
}

WindowState Window::get(){
	
	// poll stored events until window state change
	SDL_Event event;
	int key;
	while(SDL_PollEvent(&event)){
		switch(event.type){
			
			// terminate button
			case SDL_QUIT:
				return WindowQuit;
			
			// mouse click
			case SDL_MOUSEBUTTONDOWN:
				if(event.button.button < WINDOW_MOUSECODES) mouseMap[event.button.button] = (mouseMap[event.button.button] == 0 ? 1 : -1);
				else{
					setErrorStatus(ErrorMouseDown, ErrorContext{.value = event.button.button});
					return WindowError;
				}
				break;
			
			// mouse raise
			case SDL_MOUSEBUTTONUP:
				if(event.button.button < WINDOW_MOUSECODES) mouseMap[event.button.button] = 0;
				else{
					setErrorStatus(ErrorMouseUp, ErrorContext{.value = event.button.button});
					return WindowError;
				}
				break;
			
			// key press
			case SDL_KEYDOWN:
				key = keysym(event.key.keysym.sym);
				if(key < WINDOW_KEYCODES) keyMap[key] = (keyMap[key] == 0 ? 1 : -1);
				else{
					setErrorStatus(ErrorKeyDown, ErrorContext{.value = key});
					return WindowError;
				}
				break;
			
			// key raise
			case SDL_KEYUP:
				key = keysym(event.key.keysym.sym);
				if(key < WINDOW_KEYCODES) keyMap[key] = 0;
				else{
					setErrorStatus(ErrorKeyUp, ErrorContext{.value = key});
					return WindowError;
				}
				break;
			
			case SDL_MOUSEMOTION:
				
				// mouse position
				mousePosition[0] = 2.f * event.motion.x / width - 1.f;
				mousePosition[1] = -(2.f * event.motion.y / height - 1.f);
				
				// mouse cumulative motion
				mouseMotion[0] += 2.f * event.motion.xrel / width;
				mouseMotion[1] -= 2.f * event.motion.yrel / height;
				break;
			
			case SDL_WINDOWEVENT:
				switch(event.window.event){
					
					// cursor presence
					case SDL_WINDOWEVENT_ENTER:
						return WindowEnter;
					case SDL_WINDOWEVENT_LEAVE:
						return WindowLeave;
					
					// focusing
					case SDL_WINDOWEVENT_FOCUS_GAINED:
						return WindowFocus;
					case SDL_WINDOWEVENT_FOCUS_LOST:
						return WindowUnfocus;
					
					// resizing
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						width = event.window.data1;
						height = event.window.data2;
						glViewport(0, 0, width, height);
						return WindowResizing;
					case SDL_WINDOWEVENT_RESIZED:
						width = event.window.data1;
						height = event.window.data2;
						glViewport(0, 0, width, height);
						return WindowResized;
				}
				break;
			default:
				break;
		}
	}
	return WindowDefault;
}

void Window::clear() const {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::swap() const {
	SDL_GL_SwapWindow(window);
}

void Window::focus(){
	SDL_RaiseWindow(window);
}

void Window::lockCursor(){
	if(SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS && !SDL_GetRelativeMouseMode()){
		mouseMotion[0] = 0;
		mouseMotion[1] = 0;
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
}

void Window::unlockCursor(){
	SDL_SetRelativeMouseMode(SDL_FALSE);
}

// properties

void Window::setErrorStatus(ErrorWindow code, ErrorContext data){
	errorStatus = code;
	errorContext = data;
}
		
std::string Window::getErrorStatus() const {
	std::string errorMessage;
	switch(errorStatus){
		case ErrorSDLInit:
			errorMessage = "SDL init failed: " + std::string(SDL_GetError());
			break;
		case ErrorSDLWindow:
			errorMessage = "SDL window creation failed: " + std::string(SDL_GetError());
			break;
		case ErrorSDLContext:
			errorMessage = "GL context creation failed: " + std::string(SDL_GetError());
			break;
		case ErrorGLEWInit:
			errorMessage = "GLEW init failed: " + std::string((const char*)glewGetErrorString(errorContext.glValue));
			break;
		case ErrorMouseDown:
			errorMessage = "Input error: MOUSEDOWN code out of bounds: " + std::to_string(errorContext.value);
			break;
		case ErrorMouseUp:
			errorMessage = "Input error: MOUSEUP code out of bounds: " + std::to_string(errorContext.value);
			break;
		case ErrorKeyDown:
			errorMessage = "Input error: KEYDOWN code out of bounds: " + std::to_string(errorContext.value);
			break;
		case ErrorKeyUp:
			errorMessage = "Input error: KEYUP code out of bounds: " + std::to_string(errorContext.value);
			break;
		default:
			errorMessage = "No error";
			break;
	}
	return errorMessage;
}

float Window::getAspectRatio() const {
	return (float)width / height;
}

void Window::getViewport(float &x, float &y) const {
	if(width > height){
		x = (float)width / height;
		y = 1.f;
		return;
	}
	x = 1.f;
	y = (float)height / width;
}

void Window::getScreenSpace(float &w, float &h, float &cx, float &cy) const {
	w = (float)width / 2.f;
	h = (float)height / 2.f;
	cx = (float)width / 2.f;
	cy = (float)height / 2.f;
}

// input

int& Window::getMapHandle(WindowKey code){
	return keyMap[keysym(code)];
}

int& Window::getMapHandle(WindowButton code){
	return mouseMap[code];
}

float (&Window::getMouseMotionHandle())[2]{
	return mouseMotion;
}

float (&Window::getMousePositionHandle())[2]{
	return mousePosition;
}

// bindings

InputBind::InputBind(float (&m)[2], float(&p)[2]) : motion(m), position(p), isActive(false) {}

template <typename T>
void InputBind::bind(int id, T code, Window &w){
	bindings.insert({id, &w.getMapHandle(code)});
}

template <typename T>
void InputBind::bindAll(std::vector<std::pair<int, T>> const &bindings, Window &w){
	for(int i = 0; i < bindings.size(); i++) bind(std::get<0>(bindings[i]), std::get<1>(bindings[i]), w);
}

void InputBind::bindAll(std::vector<std::pair<int, WindowKey>> const &bindings, Window &w){
	bindAll<WindowKey>(bindings, w);
}

void InputBind::bindAll(std::vector<std::pair<int, WindowButton>> const &bindings, Window &w){
	bindAll<WindowButton>(bindings, w);
}

void InputBind::setActive(bool a){
	isActive = a;
}

int InputBind::getInactivePress(int id){
	if(*bindings[id] == 1){
		*bindings[id] = -1;
		return 1;
	}
	return 0;
}

int InputBind::getPress(int id){
	if(*bindings[id] == 1){
		*bindings[id] = -1;
		return 1 * isActive;
	}
	return 0;
}

int InputBind::getHold(int id){
	return abs(*bindings[id]) * isActive;
}

void InputBind::flush(){
	for(std::map<int,int*>::iterator binding = bindings.begin(); binding != bindings.end(); binding++){
		if(*binding->second == 1) *binding->second = -1;
	}
}

void InputBind::getMouseMotion(float (&m)[2]){
	m[0] = motion[0] * isActive;
	m[1] = motion[1] * isActive;
	motion[0] = motion[1] = 0;
}

void InputBind::getMousePosition(float (&p)[2]){
	p[0] = position[0] * isActive;
	p[1] = position[1] * isActive;
}