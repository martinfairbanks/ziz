#ifndef WIN64_PLATFORM_H
#define WIN64_PLATFORM_H

////////////////////////////////////
//~ NOTE: Win64 platform state
#if DEVELOPER
typedef struct Win64DebugInfo
{
    f64 msPerFrame;
    f64 fps;
    f64 fpsRaw;
    i32 monitorRefreshHz;
    
    f64 megaCyclesPerFrame;
    f64 processorSpeed;
    f64 cpuPercentage;
    
    SYSTEM_INFO systemInfo;
    FILETIME processCreationTime;
    FILETIME processExitTime;
    i64 currentUserCPUTime;
    i64 currentKernelCPUTime;
    i64 previousUserCPUTime;
    i64 previousKernelCPUTime;
    i64 currentSystemTime;
    i64 previousSystemTime;
    
} Win64DebugInfo;
#endif

#ifdef HOTLOAD
typedef struct Win64ReplayBuffer
{
    HANDLE fileHandle;
    HANDLE memoryMap;
    char filename[MAX_PATH];
    void *memoryBlock;
} Win64ReplayBuffer;
#endif

typedef struct Win64State
{
    HWND window;
    HDC deviceContext;
    b32 running;
    b32 pause;
    
    char *filename;
    char filepath[256];
    char workingDirectory[256];
    
    // toggles if backbuffer should be at the fixed init size and stretch across the window
    b32 backBufferFixed;
    b32 backBufferDeveloper;
    i32 windowWidth;
    i32 windowHeight;    
    
	// timing
    i32 monitorRefreshHz;
    f32 frameTargetSeconds;
    b32 lockFPS;
    
	f32 deltaTime; // delta seconds per frame
	f32 secondsElapsed; // seconds since program start in float
	u64 milliSecondsElapsed;
    
    // widgets
    HMENU menu;
    void (*processMenuFuncPtr)(int);
    
    // opengl software
    u32 blitTextureHandle;
    
    Renderer rendererType;
    
    #ifdef HOTLOAD
        u64 appMemorySize;
        void *appMemory;
        Win64ReplayBuffer replayBuffers[4];
            
        HANDLE recordingHandle;
        // 0 = not recording
        int inputRecordingIndex;
            
        HANDLE playbackHandle;
        // 0 = not playing back
        int inputPlayingIndex;
    #endif
    
    #if DEVELOPER
        Win64DebugInfo debug;
    #endif
} Win64State;

typedef struct Win64BackBuffer
{
    // NOTE: Pixels are 32-bits wide, 
    // Memory:   0x BB GG RR xx
    // Register: 0x xx RR GG BB
    
    // backbuffer bitmap info structure
    BITMAPINFO info;
    void *memory;
    i32 width;
    i32 height;
    
    // (stride) size of one row of pixels in bytes, width * bytePerPixel
    i32 pitch;
    i32 bytesPerPixel;
} Win64BackBuffer;

typedef struct Win64AppCode
{
    HMODULE gameCodeDLL;
    FILETIME dllLastWriteTime;
    
    UpdateStateAndRenderPtr updateStateAndRender;
    InitializeFuncPtr initialize;
    OnLoadFuncPtr onLoad;
    CleanUpFuncPtr cleanUp;
    
    b32 isValid;
} Win64AppCode;

#endif // WIN32_PLATFORM_H
