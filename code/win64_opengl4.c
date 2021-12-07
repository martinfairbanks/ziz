
//
//~ OpenGL4 - Retained mode
//

#pragma comment(lib, "opengl32.lib")

#include "../ext/include/gl/gl.h"
//#include "../ext/include/gl/glcorearb.h"
#include "../ext/include/gl/wglext.h"

// load the WGL function pointers
void* win64OpenGL4LoadFunction(char *name)
{
    void *p = (void *)wglGetProcAddress(name);
    if(!p || p == (void *)0x1 || p == (void *)0x2 || p == (void *)0x3 || p == (void *)-1)
        return 0;
    else
        return p;
}

PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
PFNWGLMAKECONTEXTCURRENTARBPROC wglMakeContextCurrentARB;
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;


void win64OpenGL4InitCore(Win64State *state)
{
    // create temporary window to load extensions
    // we have to do this cause you could only set the pixel format once
    // and it is needed to be set again with wglChoosePixelFormatARB to get a GL core profile context
    
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW | CS_OWNDC, DefWindowProcA, 0, 0,
		GetModuleHandle(0), NULL, NULL, (HBRUSH)GetStockObject(BLACK_BRUSH), NULL, "TempWindow", NULL };
    
    if (RegisterClassEx(&wc) == 0)
    {
        quitPlatformError("Fatal error", "Failed to initialize window class.");
    }
    
	HWND tempWindow = CreateWindowExA(0, "TempWindow", "TempWindow",
                                      0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                      NULL, NULL, GetModuleHandle(0), NULL);
    
    if (!tempWindow)
    {
        quitPlatformError("Fatal error", "Failed to create window.");
    }
    
	HDC dc = GetDC(tempWindow);
    
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
    pfd.cAlphaBits = 8;
	pfd.cDepthBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;
    
	int pixelFormat = ChoosePixelFormat(dc, &pfd);
    
	SetPixelFormat(dc, pixelFormat, &pfd);
    
	// create a temprary gl context
	HGLRC tempRC = wglCreateContext(dc);
	wglMakeCurrent(dc, tempRC);
    
    // activate the specified OpenGL rendering context to make it the current rendering context
    if (wglMakeCurrent(dc, tempRC) == 0)
    {
        // NOTE: the program won't crash if the other OpenGL calls would fail so we can
        // safely catch the error here
        quitPlatformError("OpenGL Error", "Failed to create OpenGL rendering context.");
    }
#if 0
    
	bool bResult = true;
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		MessageBox(hWndFake, "Couldn't initialize GLEW!", "Fatal Error", MB_ICONERROR);
		bResult = false;
	}
    
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRCFake);
	DestroyWindow(hWndFake);
    
	HWND window = CreateWindowExA(0, "OpenGL4", "OpenGL4",
                                  WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, SCREEN_WIDTH, SCREEN_HEIGHT,
                                  NULL, NULL, instance, NULL);
    
	
	dc = GetDC(window);
    #endif
}
// TODO: fix error handling
b32 win64OpenGL4Init(Win64State *state)
{
    b32 result = false;
    i32 pixelFormat = 0;
    
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,
        8,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    
    HDC deviceContext = GetDC(state->window);
    pixelFormat = ChoosePixelFormat(deviceContext, &pfd);
    
    if(pixelFormat)
    {
        SetPixelFormat(deviceContext, pixelFormat, &pfd);
        HGLRC tempRenderContext = wglCreateContext(deviceContext);
        wglMakeCurrent(deviceContext, tempRenderContext);
        
        wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)win64OpenGL4LoadFunction("wglChoosePixelFormatARB");
        wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)win64OpenGL4LoadFunction("wglCreateContextAttribsARB");
        wglMakeContextCurrentARB = (PFNWGLMAKECONTEXTCURRENTARBPROC)win64OpenGL4LoadFunction("wglMakeContextCurrentARB");
        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)win64OpenGL4LoadFunction("wglSwapIntervalEXT");
        
        i32 pixelAttribs[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB, 32,
            WGL_DEPTH_BITS_ARB, 24,
            WGL_STENCIL_BITS_ARB, 8,
            0
        };
        
        u32 numFormats = 0;
        wglChoosePixelFormatARB(deviceContext, pixelAttribs, 0, 1, &pixelFormat, &numFormats);
        
        if(pixelFormat)
        {
            i32 contextAttribs[] =
            {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                WGL_CONTEXT_MINOR_VERSION_ARB, 6,
                WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                0,
            };
            
            HGLRC glRenderContext = wglCreateContextAttribsARB(deviceContext, tempRenderContext, contextAttribs);
            if(glRenderContext)
            {
                wglMakeCurrent(deviceContext, 0);
                wglDeleteContext(tempRenderContext);
                wglMakeCurrent(deviceContext, glRenderContext);
                result = true;
            }
        }
    }
    
    b32 vsync = true;
    wglSwapIntervalEXT(vsync ? 1 : 0);
    
    ReleaseDC(state->window, deviceContext);
    SetWindowText(state->window, (LPCSTR)glGetString(GL_VERSION));
    return result;
}

#if 0
void oglCleanup(HDC *deviceContext)
{
    wglMakeCurrent(*deviceContext, 0);
    wglDeleteContext(win64State.glRenderContext);
}
#endif
