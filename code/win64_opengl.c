
//~ OpenGL
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#include <gl/gl.h>
#include <GL/glu.h>
//#include "../ext/include/gl/glcorearb.h"
#include "../ext/include/gl/wglext.h"

//
// OpenGL1.1 - immediate mode (fixed function pipeline)
//


// load the WGL function pointers
void* win64OpenGLLoadFunction(char *name)
{
    void *p = (void *)wglGetProcAddress(name);
    if(!p || p == (void *)0x1 || p == (void *)0x2 || p == (void *)0x3 || p == (void *)-1)
        return 0;
    else
        return p;
}
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;


//~ NOTE: Fonts

void buildFont(Win64State *state, const char *fontName, int fontSize)
{
    HFONT font;
    u32 base = glGenLists(96); // font base display list
    
    font = CreateFont(-fontSize,                   // font height, negative number = get a font based on the character height
                      0,                           // font width
                      0,                           // angle of escapement
                      0,                           // orientation angle
                      FW_BOLD,                     // font weight
                      FALSE,                       // italic
                      FALSE,                       // underline
                      FALSE,                       // strikeout
                      ANSI_CHARSET,                // character set identifier
                      OUT_TT_PRECIS,               // output precision, true type
                      CLIP_DEFAULT_PRECIS,         // clipping precision
                      ANTIALIASED_QUALITY,         // output quality
                      FF_DONTCARE | DEFAULT_PITCH, // family and pitch
                      fontName);                   // font name
    
    HDC dc = GetDC(state->window);
    SelectObject(dc, font);
    // create 96 characters, starting at character 32
    wglUseFontBitmaps(dc, 32, 96, base);
    ReleaseDC(state->window, dc);
}

void freeFont()
{
    u32 base = 1;
    //    if (base != 0)
    glDeleteLists(base, 96);
}

// scale to window size
// y point up
void oglChangeSize(int w, int h)
{
    f32 aspectRatio;
    
    // prevent divide by zero
    if(h == 0)
        h = 1;
    
    glViewport(0, 0, w, h);
    
    
    // reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    #if 0
        f32 halfWidth = (f32)w/2.f;
        f32 halfHeight = (f32)h/2.f;
            
        // set clipping volume (left, right, bottom, top, near, far)
        //gluPerspective(35.0f, fAspect, 1.0f, 50.0f);
        aspectRatio = (f32)w / (f32)h;
        // TODO: not working, investigate further
        if (w <= h)
            //glOrtho (0.f, w/aspectRatio, h/aspectRatio, 0.0f, 1.0, -1.0);
            glOrtho (-halfWidth, halfWidth, -halfHeight / aspectRatio, halfHeight / aspectRatio, 1.0, -1.0);
        else
            //glOrtho (0.f, w*aspectRatio, h*aspectRatio, 0.0f, 1.0, -1.0);
            glOrtho (-halfWidth * aspectRatio, halfWidth * aspectRatio, -halfHeight, halfHeight, 1.0, -1.0);
    #endif
    
    f32 windowWidth = 100;
    f32 windowHeight = 100;
    // Establish clipping volume (left, right, bottom, top, near, far)
	// An aspect ratio of 1.0 defines a square aspect ratio.An aspect ratio of 0.5 specifies 
	// that for every two pixels in the horizontal direction for a unit of length, there is one
	// pixel in the vertical direction for the same unit of length
	aspectRatio = (GLfloat)w / (GLfloat)h;
	if (w <= h)
	{
		windowWidth = 100;
		windowHeight = 100 / aspectRatio;
		glOrtho(-100.0, 100.0, -windowHeight, windowHeight, 1.0, -1.0);
	}
	else//if windows is wider than it is tall
	{
		windowWidth = 100 * aspectRatio;
		windowHeight = 100;
		glOrtho(-windowWidth, windowWidth, -100.0, 100.0, 1.0, -1.0);
	}
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// 2D projection, used for 2D drawing
void win64OpenGLSet2dProjection(i32 width, i32 height)
{
	glViewport(0, 0, width, height);
    
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    
	gluOrtho2D(0, width, height, 0);
    
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    
	glDisable(GL_DEPTH_TEST);
	//renderer.projection3DFlag = false;
    
	// blending
	glEnable(GL_BLEND);
	// color = (a*source)+(b*dest)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// set perspective projection, objects further away appear smaller
void win64OpenGLSet3dProjection(i32 width, i32 height, f32 fov, f32 nearZ, f32 farZ)
{
	// prevent divide by zero
	if (!height) return;
    
    f32 aspect = (f32)width / (f32)height;
    
    glViewport(0, 0, width, height);
    
	// center
	//glViewport(renderer.windowWidth / 4.f, renderer.windowHeight / 4.f, renderer.windowWidth / 2.f, renderer.windowHeight / 2.f);
	
	// switch to the projection matrix and reset it
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    
	// set camera perspective
    gluPerspective(fov,	   // camera (viewing) angle, field of view in degrees
                   aspect,	// aspect ratio
                   nearZ,	 // near z clipping coordinate
                   farZ);	 // far z clipping coordinate, start and end point for how deep we can draw into the screen
    
	// switch to GL_MODELVIEW, tells OGL that all future transformations will affect what we draw
	glMatrixMode(GL_MODELVIEW);
	// reset the modelview matrix, wich is where the object information is stored, sets x,y,z to zero
	glLoadIdentity();
    
	// enable depth buffer
	glEnable(GL_DEPTH_TEST);	
    
	glShadeModel(GL_SMOOTH);							
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);    
}

// initialize OpenGL 1.1
internal void
win64OpenGLInit(Win64State *state)
{
    // define the pixelformat for OpenGL, double buffering and RGBA
    PIXELFORMATDESCRIPTOR pfd = { 0 };
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; 
    // PFD_DRAW_TO_BITMAP | PFD_GENERIC_ACCELERATED | PFD_SWAP_LAYER_BUFFERS; 
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32; // bits per pixel
    pfd.dwLayerMask = PFD_MAIN_PLANE; // sets the pfd to be the main drawing plane
    
    // NOTE: is theese needed?
    pfd.cDepthBits = 32;
    pfd.cStencilBits = 8;
    pfd.cAlphaBits = 8;
    
    HDC deviceContext = GetDC(state->window);
    
    // get the best matching pixel format
    i32 pixelFormatIndex = ChoosePixelFormat(deviceContext, &pfd);
    
    // NOTE: is this needed?
    //PIXELFORMATDESCRIPTOR pixelFormat;
    //DescribePixelFormat(win64State.deviceContext, pixelFormatIndex,
    //sizeof(pixelFormat), &pixelFormat);
    
    // try to set the pixel format on the window
    SetPixelFormat(deviceContext, pixelFormatIndex, &pfd);
    
    // create a opengl rendering context
    HGLRC openGLRC = wglCreateContext(deviceContext);
    
    // activate the specified OpenGL rendering context to make it the current rendering context
    if (wglMakeCurrent(deviceContext, openGLRC) == 0)
    {
        // NOTE: the program won't crash if the other OpenGL calls would fail so we can
        // safely catch the error here
        quitPlatformError("OpenGL Error", "Failed to create OpenGL rendering context.");
    }
    
    #ifdef RENDERER_SOFTWARE_OGL1
        // setup 1 texture for blitting
        glGenTextures(1, &state->blitTextureHandle);
        glViewport(0, 0, state->windowWidth, state->windowHeight);
    #endif
    
    #ifdef RENDERER_OGL1_2D
        win64OpenGLSet2dProjection(state->windowWidth, state->windowHeight);
        //oglChangeSize(state->windowWidth, state->windowHeight);
    #endif
    
    #ifdef RENDERER_OGL1_3D
        win64OpenGLSet3dProjection(state->windowWidth, state->windowHeight, 60.f, 1.0f, 500.0f);
        //win64Sset3dProjection(win64State.windowWidth, win64State.windowHeight, 60.f, 1.0f, 500.0f);
    #endif        
    
    // enable texture mapping
    glEnable(GL_TEXTURE_2D);
    
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)win64OpenGLLoadFunction("wglSwapIntervalEXT");
    
    b32 vsync = true;
    wglSwapIntervalEXT(vsync ? 1 : 0);
    
    ReleaseDC(state->window, deviceContext);
    
    const u8 *version = glGetString(GL_VERSION);
    debugPrint("%s\n", version);
    
    buildFont(state, "Verdana", 18);
    //setWindowTitle((char*)version);
}

#if defined(RENDERER_OGL1_2D) || defined(RENDERER_OGL1_3D)
// loads 24-bit true color bitmap, width and height must be a power of 2
void loadBitmap_(char *filename, u32 *texid)
{
    HBITMAP hBMP;
    BITMAP BMP;
    
    glGenTextures(1, texid);
    hBMP=(HBITMAP)LoadImage(GetModuleHandle(NULL), filename, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE );
    
    if (!hBMP)
    {
        quitPlatformError("File error", "Failed to load bitmap.");
    }

    GetObject(hBMP, sizeof(BMP), &BMP);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    
    glBindTexture(GL_TEXTURE_2D, *texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, BMP.bmWidth, BMP.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits);
    
    DeleteObject(hBMP);
}
#endif

