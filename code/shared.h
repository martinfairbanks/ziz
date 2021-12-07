#ifndef SHARED_H
#define SHARED_H
#include "master.h"

////////////////////////////////////
//~ NOTE: Platform non-specific code - shared between platform layer and app

typedef struct File
{
    union
    {
        char *text;
        void *data;
    };
    u32 size;
} File;

typedef struct PixelBuffer
{
    // NOTE: Pixels are 32-bits wide, 
    // Memory:   0x BB GG RR xx
    // Register: 0x xx RR GG BB
    
    void *pixels;
    i32 width;
    i32 height;
    
    i32 pitch;
    i32 bytesPerPixel;
} PixelBuffer;

typedef struct SoundOutputBuffer
{
    i32 samplesPerSecond;
    i32 sampleCount;
    i16 *samples;
    
    // running time value for sinewave, progress through the period of the wave
	f32 tSine;
} SoundOutputBuffer;

typedef enum Renderer
{
    RendererSoftware_GDI,
    RendererSoftware_OpenGL,
    RendererOpenGL2D,
    RendererOpenGL3D,
    RendererOpenGL4,
    RendererDirectX11
} Renderer;

////////////////////////////////////
//~ NOTE: Input

typedef enum
{
    KEY_BACKSPACE, // 8
    KEY_TAB, // 9
    KEY_ENTER, // 13
    KEY_SHIFT, // 16
    KEY_CONTROL,
    KEY_ALT, 
    KEY_PAUSE,
    KEY_CAPS_LOCK, // 20
    KEY_ESCAPE, // 27
    KEY_SPACE, // 32
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
    KEY_END,
    KEY_HOME,
    KEY_LEFT,
    KEY_UP,
    KEY_RIGHT,
    KEY_DOWN,
    KEY_SELECT,
    KEY_PRINT,
    KEY_EXECUTE_NOT_USED,
    KEY_PRINT_SCREEN,
    KEY_INSERT,
    KEY_DELETE,
    KEY_HELP, // 47
    KEY_0, // 48
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9, // 57
    KEY_A, // 65
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_F1, // 112             
    KEY_F2,              
    KEY_F3,              
    KEY_F4,              
    KEY_F5,              
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12, // 123             
    KEY_NUM_LOCK, // 144
    KEY_SCROLL_LOCK,       
    KEY_LEFT_SHIFT, // 160      
    KEY_RIGHT_SHIFT,     
    KEY_LEFT_CONTROL,    
    KEY_RIGHT_CONTROL,   //163
    KEY_EQUAL, // 187
    KEY_COMMA,
    KEY_MINUS,
    KEY_PERIOD, // 190
    KEY_SEMICOLON,       
    KEY_FORWARD_SLASH, // 191
    KEY_GRAVE, //192           
    KEY_LEFT_BRACKET, // 219
    KEY_BACKSLASH,
    KEY_RIGHT_BRACKET,
    KEY_QUOTE, // 222
	KEY_COUNT
} KeyState;

enum
{
	MOUSE_LEFT,
	MOUSE_MIDDLE,
	MOUSE_RIGHT,
    MOUSE_FORWARD,
    MOUSE_BACK,
	MOUSE_BUTTONS_COUNT
};

enum
{
	GAMEPAD_UP,
	GAMEPAD_DOWN,
	GAMEPAD_LEFT,
	GAMEPAD_RIGHT,
	GAMEPAD_START,
	GAMEPAD_BACK,
	GAMEPAD_LEFT_THUMB,
	GAMEPAD_RIGHT_THUMB,
	GAMEPAD_LEFT_SHOULDER,
	GAMEPAD_RIGHT_SHOULDER,
	GAMEPAD_A,
	GAMEPAD_B,
	GAMEPAD_X,
	GAMEPAD_Y,
    
	GAMEPAD_BUTTONS_COUNT
};

typedef enum Gamepads
{
    GAMEPAD1,
    GAMEPAD2,
    
    GAMEPADS_COUNT
} Gamepads;


typedef struct
{
	b32 isDown;
	b32 changed;
} ButtonState;

typedef struct
{
    b32 isConnected;
    
	f32 leftStickX;
	f32 leftStickY;
	f32 rightStickX;
	f32 rightStickY;  
	u8 leftTrigger;
	u8 rightTrigger;
    
    union
	{
        ButtonState gamepadButtons[GAMEPAD_BUTTONS_COUNT];
		struct
		{
			ButtonState moveUp; 
			ButtonState moveDown;
			ButtonState moveLeft;
			ButtonState moveRight;
            
			ButtonState start;
			ButtonState back;
            
			ButtonState leftThumb;
			ButtonState rightThumb;
            ButtonState leftShoulder;
			ButtonState rightShoulder;
            ButtonState actionA; 
			ButtonState actionB;
			ButtonState actionX;
			ButtonState actionY;
		};
	};
} GamepadInput;

typedef struct
{
    ButtonState keys[KEY_COUNT];
    ButtonState mouseButtons[MOUSE_BUTTONS_COUNT];
    // TODO: support more than 1 controller?
    GamepadInput gamepad[GAMEPADS_COUNT];
    
	i32 mouseX;
    i32 mouseY;
    i32 mousePrevX;
    i32 mousePrevY;
	i32 mouseWheelDelta;
	b32 mouseDragged;
    b32 mouseMoved;
    
	f32 deltaTime; // delta seconds per frame
	f32 secondsElapsed; // seconds since program start in float
	u64 milliSecondsElapsed;
} Input;

////////////////////////////////////
//~ NOTE: Memory

// function pointers to call into the platform layer
typedef void (*CreateWindowFuncPtr)(i32 winWidth, i32 winHeight, b32 fullscreen, char *title);
typedef void (*SetWindowTitleFuncPtr)(char *title);
typedef void (*CreatePopUpMenuFuncPtr)(void (*funcPtr)(int));
typedef void (*MenuAddFuncPtr)(char *menuName, i32 menuID);
typedef void (*MenuAddSeparatorFuncPtr)();
typedef void (*DestroyPopUpMenuFuncPtr)(void (*funcPtr)(int));
typedef void (*QuitErrorFuncPtr)(char *caption, char *msg, ...);
typedef File (*LoadFileFuncPtr)(char *filename);
typedef File (*LoadTextFileFuncPtr)(char *filename);
typedef void (*FreeFileFuncPtr)(File *file);
typedef void (*FreeMemoryFuncPtr)(u32 *file);
typedef void* (*OpenGLLoadFunctionFuncPtr)(char *name);
typedef void (*LoadBitmapFuncPtr)(char *filename, u32 *texId);

typedef struct
{
    b32 codeReload;
    
    // state of the game, memory that have to persist from one frame to the next
    u64 permanentMemorySize;
    void *permanentMemory; 
    
    // don't have to persist, scratch space
    u64 tempMemorySize; 
    void *tempMemory;
    
    // NOTE: function pointers to call into the platform layer
    // these are loaded in the win32 platform layer
    CreateWindowFuncPtr createWindow;
	SetWindowTitleFuncPtr setWindowTitle;
    CreatePopUpMenuFuncPtr createPopUpMenu;
    
    MenuAddFuncPtr menuAdd;
    MenuAddSeparatorFuncPtr menuAddSeparator;
    DestroyPopUpMenuFuncPtr destroyPopUpMenu;
    
    LoadFileFuncPtr loadFile;
    LoadTextFileFuncPtr loadTextFile;
    FreeFileFuncPtr freeFile;
    FreeMemoryFuncPtr freeMemory;
    QuitErrorFuncPtr quitError;
    OpenGLLoadFunctionFuncPtr openGL4LoadFunction;
    LoadBitmapFuncPtr loadBitmap;
} AppMemory;

typedef struct MemoryArena
{
    u64 size;
    // baseaddress of memory
    u8 *base;
    // how much memory that is currently used, this goes up whenever we alloate more memory
    u64 used;
    
    // used for the temporary memory
    i32 tempCount;
} MemoryArena;

typedef struct UserState
{
    i32 windowWidth;
    i32 windowHeight;
    i32 *pixels;
    PixelBuffer backBuffer;
    Renderer rendererType;
    
    SoundOutputBuffer soundBuffer;
    
    MemoryArena arena;
    AppMemory *memory;
} UserState;

typedef void (*UpdateStateAndRenderPtr)(UserState *state, Input *newInput);
typedef void (*InitializeFuncPtr)(UserState *state, AppMemory *memory, Input *input);
typedef void (*OnLoadFuncPtr)();
typedef void (*CleanUpFuncPtr)();

#ifdef HOTLOAD
    void onLoad();
#endif

//__declspec(dllexport) does the same thing as passing /EXPORT:updateStateAndRender to the linker
void updateStateAndRender(UserState *state, Input *newInput);
void initialize(UserState *state, AppMemory *memory, Input *input);
void cleanUp();

void setup();
void updateAndDraw();
#endif
