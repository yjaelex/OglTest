#include <stdio.h>
#include <GL/glew.h>

#ifdef _WIN32
#include <Windows.h>
#include <GL/wglew.h>
#endif

#include <string>
using namespace std;

#ifndef OutputDebugString
#   define OutputDebugString(_x)
#endif

#ifndef _WINDEF_
#   define sprintf_s(_buf, _sizeofbuf, _fmt, _args) sprintf(_buf, _fmt, _args) 
#   define vsprintf_s(_buf, _sizeofbuf, _fmt, _args) vsprintf(_buf, _fmt, _args) 
#endif

static const int kBufferSize = 4096;

extern bool InitGL(size_t Width, size_t Height);
extern void ReSizeGLScene(size_t Width, size_t Height);
extern void DrawGLScene(void);
extern void DeInitGL(void);
extern void ProcessCommandLine(int argc, char* argv[]);

void print(const char* _string)
{
    OutputDebugString(_string);

#ifdef LINUX
	fprintf(stderr, "%s", _string);
#endif // DEBUG

}

// ----------------------------------------------------------------------------------------------------------------
void debug(const char* _fmt, ...)
{
    char buff[kBufferSize],
        finalBuff[kBufferSize];
    va_list args;
    va_start(args, _fmt);
    vsprintf_s(buff, sizeof(buff), _fmt, args);
    va_end(args);

    sprintf_s(finalBuff, sizeof(finalBuff), "debug: %s\n", buff);
    print(finalBuff);
}

// ----------------------------------------------------------------------------------------------------------------
void log(const char* _fmt, ...)
{
    char buff[kBufferSize],
        finalBuff[kBufferSize];
    va_list args;
    va_start(args, _fmt);
    vsprintf_s(buff, sizeof(buff), _fmt, args);
    va_end(args);

    sprintf_s(finalBuff, sizeof(finalBuff), "%s\n", buff);
    print(finalBuff);
}

// ----------------------------------------------------------------------------------------------------------------
void warn(const char* _fmt, ...)
{
    char buff[kBufferSize],
        finalBuff[kBufferSize];
    va_list args;
    va_start(args, _fmt);
    vsprintf_s(buff, sizeof(buff), _fmt, args);
    va_end(args);

    sprintf_s(finalBuff, sizeof(finalBuff), "Warning: %s\n", buff);
    print(finalBuff);
}

// ----------------------------------------------------------------------------------------------------------------
void error(const char* _fmt, ...)
{
    char buff[kBufferSize],
        finalBuff[kBufferSize];
    va_list args;
    va_start(args, _fmt);
    vsprintf_s(buff, sizeof(buff), _fmt, args);
    va_end(args);

    sprintf_s(finalBuff, sizeof(finalBuff), "ERROR: %s\n", buff);
    print(finalBuff);

    exit(-1);
}


bool CheckError(const char* Title)
{
	int Error;
	if ((Error = glGetError()) != GL_NO_ERROR)
	{
		std::string ErrorString;
		switch (Error)
		{
		case GL_INVALID_ENUM:
			ErrorString = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			ErrorString = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			ErrorString = "GL_INVALID_OPERATION";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			ErrorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
		case GL_OUT_OF_MEMORY:
			ErrorString = "GL_OUT_OF_MEMORY";
			break;
		default:
			ErrorString = "UNKNOWN";
			break;
		}
		log("OpenGL Error(%s): %s\n", ErrorString.c_str(), Title);
	}
	return Error == GL_NO_ERROR;
}


bool CheckFramebuffer(GLuint FramebufferName)
{
	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	switch (Status)
	{
	case GL_FRAMEBUFFER_UNDEFINED:
		log("OpenGL Error(%s)\n", "GL_FRAMEBUFFER_UNDEFINED");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		log("OpenGL Error(%s)\n", "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		log("OpenGL Error(%s)\n", "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		log("OpenGL Error(%s)\n", "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		log("OpenGL Error(%s)\n", "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		log("OpenGL Error(%s)\n", "GL_FRAMEBUFFER_UNSUPPORTED");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		log("OpenGL Error(%s)\n", "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
		log("OpenGL Error(%s)\n", "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS");
		break;
	}

	return Status == GL_FRAMEBUFFER_COMPLETE;
}

bool CheckExtension(char const* ExtensionName)
{
	GLint ExtensionCount = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &ExtensionCount);
	for (GLint i = 0; i < ExtensionCount; ++i)
		if (string((char const*)glGetStringi(GL_EXTENSIONS, i)) == string(ExtensionName))
			return true;
	log("Failed to find Extension: \"%s\"\n", ExtensionName);
	return false;
}


#ifdef _WIN32
static	HGLRC hRC;		// Permanent Rendering Context
static	HDC hDC;		// Private GDI Device Context

BOOL	keys[256];		// Array Used For The Keyboard Routine

static void
init_opengl_extensions(void)
{
	// Before we can load extensions, we need a dummy OpenGL context, created using a dummy window.
	// We use a dummy window because you can only set the pixel format for a window once. For the
	// real window, we want to use wglChoosePixelFormatARB (so we can potentially specify options
	// that aren't available in PIXELFORMATDESCRIPTOR), but we can't load and use that before we
	// have a context.

	WNDCLASS	wc;			// Windows Class Structure Used To Set Up The Type Of Window

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = DefWindowProcA;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(0);
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TEXT("Dummy_WGL_dju");

	if(!RegisterClass(&wc))
	{
		error("Failed to register dummy OpenGL window.");
	}

	HWND dummy_window = CreateWindowEx(
		0,
		wc.lpszClassName,
		TEXT("Dummy OpenGL Window"),
		0,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		wc.hInstance,
		0);

	if (!dummy_window) {
		error("Failed to create dummy OpenGL window.");
	}

	HDC dummy_dc = GetDC(dummy_window);

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),		// Size Of This Pixel Format Descriptor
		1,									// Version Number (?)
		PFD_DRAW_TO_WINDOW |				// Format Must Support Window
		PFD_SUPPORT_OPENGL |				// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,					// Must Support Double Buffering
		PFD_TYPE_RGBA,						// Request An RGBA Format
		32,
		0, 0, 0, 0, 0, 0,					// Color Bits Ignored (?)
		0,									// No Alpha Buffer
		0,									// Shift Bit Ignored (?)
		0,									// No Accumulation Buffer
		0, 0, 0, 0,							// Accumulation Bits Ignored (?)
		24,									// 16Bit Z-Buffer (Depth Buffer)  
		0,									// No Stencil Buffer
		0,									// No Auxiliary Buffer (?)
		PFD_MAIN_PLANE,						// Main Drawing Layer
		0,									// Reserved (?)
		0, 0, 0								// Layer Masks Ignored (?)
	};

	int pixel_format = ChoosePixelFormat(dummy_dc, &pfd);
	if (!pixel_format) {
		error("Failed to find a suitable pixel format.");
	}
	if (!SetPixelFormat(dummy_dc, pixel_format, &pfd)) {
		error("Failed to set the pixel format.");
	}

	HGLRC dummy_context = wglCreateContext(dummy_dc);
	if (!dummy_context) {
		error("Failed to create a dummy OpenGL rendering context.");
	}

	if (!wglMakeCurrent(dummy_dc, dummy_context)) {
		error("Failed to activate dummy OpenGL rendering context.");
	}

	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress(
		"wglCreateContextAttribsARB");
	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress(
		"wglChoosePixelFormatARB");

	wglMakeCurrent(dummy_dc, 0);
	wglDeleteContext(dummy_context);
	ReleaseDC(dummy_window, dummy_dc);
	DestroyWindow(dummy_window);
}

static HGLRC
init_opengl_wgl(HDC real_dc)
{
	init_opengl_extensions();

	// Now we can choose a pixel format the modern way, using wglChoosePixelFormatARB.
	int pixel_format_attribs[] = {
		WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
		WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
		WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB,         32,
		WGL_DEPTH_BITS_ARB,         24,
		WGL_STENCIL_BITS_ARB,       8,
		0
	};

	int pixel_format;
	UINT num_formats;
	wglChoosePixelFormatARB(real_dc, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
	if (!num_formats)
	{
		error("Failed to set the OpenGL pixel format.");
	}

	PIXELFORMATDESCRIPTOR pfd;
	DescribePixelFormat(real_dc, pixel_format, sizeof(pfd), &pfd);
	if (!SetPixelFormat(real_dc, pixel_format, &pfd)) {
		error("Failed to set the OpenGL pixel format.");
	}

	// Specify that we want to create an OpenGL core profile context
	int gl41_attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 1,
		WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0,
	};

	HGLRC gl41_context = wglCreateContextAttribsARB(real_dc, 0, gl41_attribs);
	if (!gl41_context) {
		error("Failed to create OpenGL Core context.");
	}

	if (!wglMakeCurrent(real_dc, gl41_context)) {
		error("Failed to activate OpenGL 3.3 rendering context.");
	}

	return gl41_context;
}

LRESULT CALLBACK WndProc(	HWND	hWnd,
				UINT	message,
				WPARAM	wParam,
				LPARAM	lParam)
{
	RECT	Screen;							// Used Later On To Get The Size Of The Window

	switch (message)						// Tells Windows We Want To Check The Message
	{
		case WM_CREATE:
			hDC = GetDC(hWnd);				// Gets A Device Context For The Window
			hRC = init_opengl_wgl(hDC);

			GetClientRect(hWnd, &Screen);
			InitGL(Screen.right, Screen.bottom);
			break;

		case WM_DESTROY:
		case WM_CLOSE:
			ChangeDisplaySettings(NULL, 0);

			DeInitGL();
			wglMakeCurrent(hDC,NULL);
			wglDeleteContext(hRC);
			ReleaseDC(hWnd,hDC);

			PostQuitMessage(0);
			break;

		case WM_KEYDOWN:
			keys[wParam] = TRUE;
			break;

		case WM_KEYUP:
			keys[wParam] = FALSE;
			break;

		case WM_SIZE:
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));
			break;

		default:
			return (DefWindowProc(hWnd, message, wParam, lParam));
	}
return (0);
}


int WINAPI WinMain(	HINSTANCE	hInstance, 
					HINSTANCE	hPrevInstance, 
					LPSTR		lpCmdLine, 
					int			nCmdShow)
{
	MSG			msg;		// Windows Message Structure
	WNDCLASS	wc;			// Windows Class Structure Used To Set Up The Type Of Window
	HWND		hWnd;		// Storage For Window Handle

	ProcessCommandLine(__argc, __argv);

	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc		= (WNDPROC) WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= NULL;
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= TEXT("OPENGL_WINDOW");

	if(!RegisterClass(&wc))
	{
		MessageBox(0,TEXT("Failed To Register The Window Class."),TEXT("Error"),MB_OK|MB_ICONERROR);
		return FALSE;
	}

	hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
                            TEXT("OPENGL_WINDOW"),
                            TEXT("OpenGL window"),
                            WS_OVERLAPPEDWINDOW,
                            0, 0,
                            800, 600,
                            NULL,
                            NULL,
                            hInstance,
                            NULL);

	if(!hWnd)
	{
		MessageBox(0,TEXT("Window Creation Error."),TEXT("Error"),MB_OK|MB_ICONERROR);
		return FALSE;
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	SetFocus(hWnd);
	wglMakeCurrent(hDC,hRC);

	while (1)
	{
		// Process All Messages
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (GetMessage(&msg, NULL, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				return TRUE;
			}
		}

		DrawGLScene();
		SwapBuffers(hDC);
		if (keys[VK_ESCAPE]) SendMessage(hWnd,WM_CLOSE,0,0);
	}
}

#endif
