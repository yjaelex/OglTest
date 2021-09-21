#include <stdio.h>
#include <GL/glew.h>

#ifdef _WIN32
#include <Windows.h>
#include <GL/wglew.h>
#else
#include <stdarg.h>
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
    char buff[kBufferSize - 32],
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
    char buff[kBufferSize - 32],
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
    char buff[kBufferSize - 32],
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
    char buff[kBufferSize - 32],
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
#else

// Linux
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if WINSYS_EGL
#include <EGL/egl.h>
#else
#include <GL/glx.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#define GLX_CONTEXT_MAJOR_VERSION_ARB       0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB       0x2092
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
#define ESCAPE 27

// Helper to check for extension string presence.  Adapted from:
//   http://www.opengl.org/resources/features/OGLextensions/
static bool isExtensionSupported(const char *extList, const char *extension)
{

    const char *start;
    const char *where, *terminator;

    /* Extension names should not have spaces. */
    where = strchr(extension, ' ');
    if ( where || *extension == '\0' )
        return false;

    /* It takes a bit of care to be fool-proof about parsing the
        OpenGL extensions string. Don't be fooled by sub-strings,
        etc. */
    for ( start = extList; ; )
    {
        where = strstr( start, extension );

        if ( !where )
        break;

        terminator = where + strlen( extension );

        if ( where == start || *(where - 1) == ' ' )
        if ( *terminator == ' ' || *terminator == '\0' )
            return true;

        start = terminator;
    }
    return false;
}

static bool ctxErrorOccurred = false;
static int ctxErrorHandler( Display *dpy, XErrorEvent *ev )
{
    ctxErrorOccurred = true;
    return 0;
}

/* stuff about our window grouped together */
typedef struct {
    Display *display;
    int screen;
    Window win;
    GLXContext ctx;
    Bool fs;
    int x, y;
    unsigned int width, height;
    unsigned int depth;
} GLWindow;
GLWindow GLWin;

#define NOP 0
#define EXIT 1
#define DRAW 2

/**
 * Handle one X event.
 * \return NOP, EXIT or DRAW
 */
static int
handle_event(Display *dpy, Window win, XEvent *event)
{
   (void) dpy;
   (void) win;

   switch (event->type) {
   case Expose:
        return DRAW;
   case ConfigureNotify:
        GLWin.width = event->xconfigure.width;
        GLWin.height = event->xconfigure.height;
        log("Resize event: Width = %d, Height = %d\n", GLWin.width, GLWin.height);
        ReSizeGLScene(event->xconfigure.width, event->xconfigure.height);
        break;
   case KeyPress:
        {
            char buffer[10];
            int code;
            code = XLookupKeysym(&event->xkey, 0);
            if (code == XK_Left)
            {

            }
            else if (code == XK_Right)
            {

            }
            else if (code == XK_Up)
            {

            }
            else if (code == XK_Down)
            {

            }
            else
            {
                XLookupString(&event->xkey, buffer, sizeof(buffer),NULL, NULL);
                if (buffer[0] == 27) {
                /* escape */
                return EXIT;
                }
                else if (buffer[0] == 'a' || buffer[0] == 'A') {
                //animate = !animate;
                }
            }
            return DRAW;
        }
   }
   return NOP;
}

static void
event_loop(Display *dpy, Window win)
{
   while (1)
   {
        int op;
        while (XPending(dpy) > 0)
        {
            XEvent event;
            XNextEvent(dpy, &event);
            op = handle_event(dpy, win, &event);
            if (op == EXIT)
                return;
            else if (op == DRAW)
                break;
        }

        DrawGLScene();
        glXSwapBuffers (GLWin.display, GLWin.win );
    }
}

int main (int argc, char ** argv)
{
    GLWin.display = XOpenDisplay(0);
    XEvent event;

    if ( !GLWin.display )
    {
        error( "Failed to open X display.\n" );
    }

    // Get a matching FB config
    static int visual_attribs[] =
    {
        GLX_X_RENDERABLE    , True,
        GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
        GLX_RENDER_TYPE     , GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
        GLX_RED_SIZE        , 8,
        GLX_GREEN_SIZE      , 8,
        GLX_BLUE_SIZE       , 8,
        GLX_ALPHA_SIZE      , 8,
        GLX_DEPTH_SIZE      , 24,
        GLX_STENCIL_SIZE    , 8,
        GLX_DOUBLEBUFFER    , True,
        //GLX_SAMPLE_BUFFERS  , 1,
        //GLX_SAMPLES         , 4,
        None
    };

    int glx_major, glx_minor;

  // FBConfigs were added in GLX version 1.3.
    if ( !glXQueryVersion( GLWin.display, &glx_major, &glx_minor ) ||
       ( ( glx_major == 1 ) && ( glx_minor < 3 ) ) || ( glx_major < 1 ) )
    {
        error( "Invalid GLX version.\n" );
    }

    log( "Getting matching framebuffer configs.\n");
    int fbcount;
    GLXFBConfig *fbc = glXChooseFBConfig(GLWin.display, DefaultScreen( GLWin.display ),
                                         visual_attribs, &fbcount );
    if ( !fbc )
    {
        error( "Failed to retrieve a framebuffer config.\n");
    }
    log( "Found %d matching FB configs.\n", fbcount );

    // Pick the FB config/visual with the most samples per pixel
    log( "Getting XVisualInfos ...\n" );
    int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;

    int i;
    for ( i = 0; i < fbcount; i++ )
    {
        XVisualInfo *vi = glXGetVisualFromFBConfig( GLWin.display, fbc[i] );
        if ( vi )
        {
            int samp_buf, samples;
            glXGetFBConfigAttrib( GLWin.display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );
            glXGetFBConfigAttrib( GLWin.display, fbc[i], GLX_SAMPLES       , &samples  );

            log( "  Matching fbconfig %d, visual ID 0x%2li: SAMPLE_BUFFERS = %d,"
                    " SAMPLES = %d .\n", i, vi->visualid, samp_buf, samples );

        if ( (best_fbc < 0 || samp_buf) && (samples > best_num_samp) )
            best_fbc = i, best_num_samp = samples;
        if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )
            worst_fbc = i, worst_num_samp = samples;
        }

        XFree( vi );
    }

    GLXFBConfig bestFbc = fbc[ best_fbc ];
    // Be sure to free the FBConfig list allocated by glXChooseFBConfig()
    XFree( fbc );

    ProcessCommandLine(argc, argv);

    // Get a visual
    XVisualInfo *vi = glXGetVisualFromFBConfig( GLWin.display, bestFbc );
    log( "Chosen visual ID = 0x%li .\n", vi->visualid );
    log( "Creating colormap ...\n" );
    XSetWindowAttributes swa;
    Colormap cmap;
    swa.colormap = cmap = XCreateColormap( GLWin.display,
                                         RootWindow( GLWin.display, vi->screen ),
                                         vi->visual, AllocNone );
    swa.background_pixmap = None ;
    swa.border_pixel      = 0;
    swa.event_mask        = StructureNotifyMask;

    GLWin.width = 800;
    GLWin.height = 600;
    GLWin.depth = vi->depth;
    GLWin.x = 0;
    GLWin.y = 0;
    log("Creating window...   Width = %d, Height = %d, Depth = %d\n",
        GLWin.width, GLWin.height, GLWin.depth);

    GLWin.win = XCreateWindow(GLWin.display, RootWindow( GLWin.display, vi->screen ),
                        0, 0, GLWin.width, GLWin.height, 0, vi->depth, InputOutput,
                        vi->visual, CWBorderPixel|CWColormap|CWEventMask, &swa );
    if ( !GLWin.win )
    {
        error( "Failed to create window.\n");
    }

    // Done with the visual info data
    XFree( vi );

    XStoreName( GLWin.display, GLWin.win, "GL Test Window" );

    log( "Mapping window.\n" );
    XMapWindow( GLWin.display, GLWin.win );

    // Get the default screen's GLX extension list
    const char *glxExts = glXQueryExtensionsString( GLWin.display, DefaultScreen( GLWin.display ) );

    // NOTE: It is not necessary to create or make current to a context before
    // calling glXGetProcAddressARB
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
           glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );

    GLXContext ctx = 0;

    // Install an X error handler so the application won't exit if GL 3.0
    // context allocation fails.
    //
    // Note this error handler is global.  All display connections in all threads
    // of a process use the same error handler, so be sure to guard against other
    // threads issuing X commands while this code is running.
    ctxErrorOccurred = false;
    int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);

    // Check for the GLX_ARB_create_context extension string and the function.
    // If either is not present, use GLX 1.3 context creation method.
    if ( !isExtensionSupported( glxExts, "GLX_ARB_create_context" ) ||
        !glXCreateContextAttribsARB )
    {
        printf( "glXCreateContextAttribsARB() not found ... using old-style GLX context.\n" );
        ctx = glXCreateNewContext( GLWin.display, bestFbc, GLX_RGBA_TYPE, 0, True );
    }
    // If it does, try to get a GL 3.0 context!
    else
    {
        int context_attribs[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
            GLX_CONTEXT_MINOR_VERSION_ARB, 1,
            //GLX_CONTEXT_FLAGS_ARB       , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
            None
        };

        log( "Creating context ...\n" );
        ctx = glXCreateContextAttribsARB( GLWin.display, bestFbc, 0,
                                        True, context_attribs );

        // Sync to ensure any errors generated are processed.
        XSync( GLWin.display, False );
        if ( !ctxErrorOccurred && ctx )
        {
            log( "Created GL 4.1 context" );
            GLWin.ctx = ctx;
        }
        else
        {
            // Couldn't create GL context.
            GLWin.ctx = 0;
            error( "Failed to create GL context" );
        }
    }

    // Sync to ensure any errors generated are processed.
    XSync( GLWin.display, False );
    // Restore the original error handler
    XSetErrorHandler( oldHandler );
    if ( ctxErrorOccurred || !ctx )
    {
        error( "Failed to create an OpenGL context.\n" );
    }

    // Verifying that context is a direct context
    if ( ! glXIsDirect ( GLWin.display, ctx ) )
    {
        log( "Indirect GLX rendering context obtained.\n" );
    }
    else
    {
        log( "Direct GLX rendering context obtained.\n" );
    }

    log( "Making context current ...\n" );
    glXMakeCurrent( GLWin.display, GLWin.win, ctx );

    //XSelectInput( GLWin.display,GLWin.win, ExposureMask | KeyPressMask );
    int s = DefaultScreen(GLWin.display);
    log( "Default Screen: %d", s);

    /////////////////////////////////////////
    // initialize!
    InitGL(GLWin.width, GLWin.height);
    ReSizeGLScene(GLWin.width, GLWin.height);
    /////////////////////////////////////////
    event_loop(GLWin.display, GLWin.win);

    DeInitGL();
    glXDestroyContext( GLWin.display, ctx );
    XDestroyWindow( GLWin.display, GLWin.win );
    XFreeColormap( GLWin.display, cmap );
    XCloseDisplay( GLWin.display );

    return 0;
}


#endif
