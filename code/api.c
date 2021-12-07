////////////////////////////////////
//~ NOTE: API

#include "shared.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../ext/include/stb_image.h"

typedef struct TemporaryMemory
{
    MemoryArena *arena;
    u64 used;
} TemporaryMemory;

global UserState *state;
global Input *input;

typedef struct InternalAppState
{
    // running time value for sinewave, progress through the period of the wave
	f32 tSine;
} InternalAppState;

global InternalAppState *internalAppState;

////////////////////////////////////
//~ NOTE: Memory Allocator
//

// initialize memory
inline void 
initializeArena(MemoryArena *arena, u64 size, void *base)
{
    arena->size = size;
    arena->base = (u8 *)base;
    arena->used = sizeof(InternalAppState);
    arena->tempCount = 0;
}

#define memoryPushStruct(T) (T *)pushSize_(sizeof(T))
#define memoryPushArray(arena, count, T) (T *)pushSize_(arena, (count)*sizeof(T))
#define memoryPushSize(arena, size) pushSize_(arena, size)
#define memoryMapStruct(s, T) s = (T *)((u8*)state->memory->permanentMemory+sizeof(InternalAppState)); \
    state->arena.used += sizeof(T);

// returns the adress to the beginning of the memory
inline void *
pushSize_(u64 size)
{
    assert((state->arena.used + size) <= state->arena.size);
    void *result = state->arena.base + state->arena.used;
    state->arena.used += size;
    
    return result;
}

// use memory from the temporary memory space
// this is used so we can restore the state between frames
inline TemporaryMemory
beginTemporaryMemory(MemoryArena *arena)
{
    TemporaryMemory result;
    result.arena = arena;
    result.used = arena->used;
    ++arena->tempCount;
    return result;
}

// set the stackpointer back on our temporary memory to where it was before
inline void
endTemporaryMemory(TemporaryMemory tempMem)
{
    MemoryArena *arena = tempMem.arena;
    assert(arena->used >= tempMem.used);
    arena->used = tempMem.used;
    assert(arena->tempCount > 0);
    --arena->tempCount;
}

inline void
checkArena(MemoryArena *arena)
{
    assert(arena->tempCount == 0);
}


////////////////////////////////////
//~ NOTE: wrapper functions that calls into the platform layer
void createWindow(i32 winWidth, i32 winHeight, b32 fullscreen, char *title)
{    
    state->memory->createWindow(winWidth, winHeight, fullscreen, title);
}

void createPopUpMenu(void (*processMenu)(int))
{
    state->memory->createPopUpMenu(processMenu);
}

void menuAdd(char *menuName, i32 menuID)
{
    state->memory->menuAdd(menuName, menuID);
}

void menuAddSeparator()
{
    state->memory->menuAddSeparator();
}

File loadFile(char *filename)
{
    File result = state->memory->loadFile(filename);
    return result;
}

File loadTextFile(char *filename)
{
    File result = state->memory->loadTextFile(filename);
    return result;
}

void freeFile(File *file)
{
    state->memory->freeFile(file);
}

#if defined(RENDERER_OGL1_2D) || defined(RENDERER_OGL1_3D)
void loadBitmap(char *filename, u32 *texId)
{
    state->memory->loadBitmap(filename, texId);
}
#endif
//#include <stdio.h>
//#include <stdarg.h>
void quitError(char *caption, char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    state->memory->quitError(caption, msg, args);
}

#define logError(M, ...) \
    fprintf(stderr, \
    "Error in file %s:%d. Errno: %s." M "\n", __FILE__, __LINE__,\
    strerror(errno), ##__VA_ARGS__)

#define check(A, M, ...) \
    if(!(A)) {\
        logError(M, ##__VA_ARGS__); errno=0; quitError("Error", M); }


////////////////////////////////////
//~ NOTE: Renderers

#include "renderer_master.c"

#if defined(RENDERER_SOFTWARE_GDI) || defined(RENDERER_SOFTWARE_OGL1)
    #include "renderer_software.c"
#endif

#if defined(RENDERER_OGL1_2D) || defined(RENDERER_OGL1_3D)
    #include "renderer_opengl.c"
#endif

#ifdef RENDERER_OGL4
    #include "renderer_opengl4.c"
#endif


////////////////////////////////////
//~ NOTE: Internal functions
//

void initialize(UserState *newState, AppMemory *memory, Input *newInput)
{
    state = newState;
	// let the appState point to the permanent storage
    internalAppState = (InternalAppState *)memory->permanentMemory;
    newState->memory = memory;
    input = newInput;
    internalAppState->tSine = 8.0f;
    
    // create memory for the arena
    initializeArena(&state->arena, (u64)memory->permanentMemory, (u8 *)memory->permanentMemory);
    
    setup();
}

void updateStateAndRender(UserState *newState, Input *newInput)
{
    // NOTE: Update state
    //-
    {
        state = newState;
        input = newInput;
    }
    
#ifdef HOTLOAD
	// if the code needs to reload
    // this is true the first time around
    if (state->memory->codeReload)
	{
        // let the appState point to the permanent storage
        internalAppState = (InternalAppState *)state->memory->permanentMemory;
        
        #ifdef RENDERER_OGL4
            initOpenGL4(state);
        #endif
        
        onLoad();
        state->memory->codeReload = false;
    }
#endif
    updateAndDraw();
}

////////////////////////////////////
//~ NOTE: Input

b32 keyPressed(u32 b)
{
    b32 result = input->keys[b].isDown && input->keys[b].changed;
    return result;
}

b32 keyUp(u32 b)
{
    b32 result = !input->keys[b].isDown && input->keys[b].changed;
    return result;
}

b32 keyDown(u32 b)
{
    b32 result = input->keys[b].isDown;
    return result;
}

b32 gamepadButtonPressed(Gamepads gamepad, u32 b)
{
    b32 result = input->gamepad[gamepad].gamepadButtons[b].isDown && input->gamepad[gamepad].gamepadButtons[b].changed;
    return result;
}

b32 gamepadButtonUp(Gamepads gamepad, u32 b)
{
    b32 result = !input->gamepad[gamepad].gamepadButtons[b].isDown && input->gamepad[gamepad].gamepadButtons[b].changed;
    return result;
}

b32 gamepadButtonDown(Gamepads gamepad, u32 b)
{
    b32 result = input->gamepad[gamepad].gamepadButtons[b].isDown;
    return result;
}

f32 gamepadReadLStickX(Gamepads gamepad)
{
    f32 result = input->gamepad[gamepad].leftStickX;
    return result;
}

f32 gamepadReadLStickY(Gamepads gamepad)
{
    f32 result = input->gamepad[gamepad].leftStickY;
    return result;
}

f32 gamepadReadRStickX(Gamepads gamepad)
{
    f32 result = input->gamepad[gamepad].rightStickX;
    return result;
}

f32 gamepadReadRStickY(Gamepads gamepad)
{
    f32 result = input->gamepad[gamepad].rightStickY;
    return result;
}

b32 mousePressed(u32 b)
{
    b32 result = input->mouseButtons[b].isDown && input->mouseButtons[b].changed;
    return result;
}

b32 mouseUp(u32 b)
{
    b32 result = !input->mouseButtons[b].isDown && input->mouseButtons[b].changed;
    return result;
}

b32 mouseDown(u32 b)
{
    b32 result = input->mouseButtons[b].isDown;
    return result;
}

b32 mouseDragged()
{
    b32 result = input->mouseDragged;
    return result;
}

b32 mouseMoved()
{
    b32 result = input->mouseMoved;
    return result;
}

v2 mousePosition()
{
    v2 result = {(f32)input->mouseX, (f32)input->mouseY};
    return result;
}


////////////////////////////////////
//~ NOTE: Audio

internal void
playTone(SoundOutputBuffer *soundBuffer, i32 toneHz, i32 toneVolume)
{
    // ToneHz
    // frequency of the square wave
    // 440Hz = middle A
    // to fill a buffer that is 1 second long it should have 440 copies of that wave pattern (cycles) for middle A
    // if our samplerate is 48000Hz (samples/second), each period of the wave (wave period) would have 48000/440 samples for middle A. 
    // so 1 wave period would contain 109 samples. middle a goes upp and down 440 times, so 440 * 109 samples = 48 0000
    
    // Volume
    // volume of the wave - 32767 (the largest representable int16) is loudest, 0 is silent.
    
    // wavePeriod - period of wave in samples, i.e. the number of samples before it repeats
    i32 wavePeriod = soundBuffer->samplesPerSecond / toneHz;
	i16 *sampleOut = soundBuffer->samples;
    
	// fill in samples
	for (i32 sampleIndex = 0; sampleIndex < soundBuffer->sampleCount; sampleIndex++)
	{
		f32 sineValue = sinf(internalAppState->tSine);
		// scale sinewave from -1 to 1 to toneVolume 
		i16 sampleValue = (i16)(sineValue * toneVolume);
		*sampleOut++ = sampleValue;
		*sampleOut++ = sampleValue;
        
		// 1 sine period = 2*PI
		// 2.0f*PI*1.0f / (f32)soundOutput->wavePeriod = 0.057643905
        internalAppState->tSine += 2.0f*PI*1.0f/(f32)wavePeriod;
        
        // this is due to the loss of precision of the floating-point value
        // tSine becomes less and less precise very quickly, which causes the pitch to change
        if (internalAppState->tSine > 2.0f * PI)
        {
            internalAppState->tSine -= 2.0f * PI;
        }
	}
}

// TODO: duration
void playNote(SoundOutputBuffer *soundOutput, char note, i32 volume)
{
    i32 toneHz = 0;
    switch (note)
    {
        case 'C':
        {
            toneHz = 261;
        } break;
        
        case 'D':
        {
            toneHz = 293;
        } break;
        
        case 'E':
        {
            toneHz = 329;
        } break;
        
        case 'F':
        {
            toneHz = 349;
        } break;
        
        case 'G':
        {
            toneHz = 392;
        } break;
        
        case 'A':
        {
            toneHz = 440;
            
        } break;
        
        case 'B':
        {
            toneHz = 523;
        } break;
    }
    
    playTone(soundOutput, toneHz, volume);
}
