/*  
- Win64 Platform Layer
                    
  TODO:
    [x] key input
    [x] gamepad input
    [x] mouse input
    [x] timing
    [x] assembly test
    [x] bitmaps
    [ ] directX11 software rendering - blit texture
    [ ] directX11 hardware rendering
    [ ] audio test
    [ ] remove renderer struct and argument from createWindow
    [ ] document all functions
    [ ] save bitmap
    [ ] stb jpg/png
    [ ] fonts, windows and stb
    [ ] audio, wav file loader
    [ ] build with clang
    [ ] make sure that it compiles with cpp compiler

    Bugs:
    [ ] when pausing the opengl1 renderer rendres on two different positions, which casues flickering

*/


////////////////////////////////////
//~ NOTE: import libraries, kernel32.lib is implicitly linked when building with the CRT
#define _CRT_SECURE_NO_WARNINGS
    
#pragma comment(lib, "user32.lib")      // windows utility functions
#pragma comment(lib, "gdi32.lib")       // windows render functions 
#pragma comment(lib, "winmm.LIB")       // for timebeginperiod
#pragma comment(lib, "dwmapi.lib")      // to remove stutter in windowed opengl

////////////////////////////////////
//~ NOTE: Includes
#include "ziz_renderer.h"
#include "shared.h"
    
// reduce warning level for headers
#pragma warning(push, 3)	
    #include <windows.h>
    #include <dwmapi.h>
#pragma warning(pop)
    
#if DEVELOPER
    #include <crtdbg.h>   // check for memory leaks
    #include <psapi.h>    // K32GetProcessMemoryInfo
#endif
    
#include <xinput.h>
#include "win64_platform.h"
    
// TODO: make win64State local to WinMain
global Win64State  win64State;
global Win64BackBuffer win64BackBuffer;
global Win64AppCode appCode;
    
////////////////////////////////////
//~ NOTE: Windows utility functions that can be called by the user
//        Functions that the platform layer provides to the user application    
void quitError_(char *caption, char *msg, ...)
{
    char buf_[256];
    va_list args;
    va_start(args, msg);
    wsprintf(buf_, "%s Windows error code: %d.", msg, GetLastError(), args);
    va_end(args);
    //	OutputDebugStringA(buffer);
    MessageBoxA(0, buf_, caption, MB_ICONERROR);
    if (appCode.cleanUp)
    {
        appCode.cleanUp();
    }
    ExitProcess(1);
}
#define quitPlatformError quitError_
    
// TODO: Threading or fibers, so the app wont freeze when the menu pops up?
// set the function pointer to the users process menu
void createPopUpMenu_(void (*funcPtr)(int))
{
    if (!win64State.menu)
        win64State.menu = CreatePopupMenu();
    win64State.processMenuFuncPtr = funcPtr;
}
    
void menuAdd_(char *menuName, i32 menuID)
{
    if (win64State.menu)
        AppendMenuA(win64State.menu, MF_STRING, menuID, menuName);
}
    
void menuAddSeparator_()
{
    if (win64State.menu)
        AppendMenuA(win64State.menu, MF_SEPARATOR, 0, NULL);
}
    
void destroyPopUpMenu_(void (*funcPtr)(int))
{
    if (!win64State.menu)
    {
        DestroyMenu(win64State.menu);
        win64State.menu = 0;
        win64State.processMenuFuncPtr = 0;
    }
}
    
void setWindowTitle(char *title)
{
    SetWindowTextA(win64State.window, title);
}
    
void showCursor()
{
    ShowCursor(1);
}
    
void hideCursor()
{
    ShowCursor(0);
}
    
#include "win64_file_io.c"
// TODO: put this in shared.h?
#include "strings.c"
    
////////////////////////////////////
//~ NOTE: Windows Internal

#define logPlatformError(M, ...) \
fprintf(stderr, \
"Error in file %s:%d. Errno: %s. WinErrorCode: %lu. " M "\n", __FILE__, __LINE__,\
strerror(errno), GetLastError(), ##__VA_ARGS__)

#define checkPlatfromError(A, M, ...) \
if(!(A)) {\
logError(M, ##__VA_ARGS__); errno=0; quitError_(); }


#if DEVELOPER
inline void
win64DebugPrint(char *message, ...)
{
    static char buffer[1024];
    va_list args;
    va_start(args, message);
    // vsprintf is used to pass printf arguments
    vsprintf_s(buffer, sizeof(buffer), message, args);
    va_end(args);
                    
    OutputDebugStringA(buffer);
    OutputDebugStringA("\n");
}
#endif
    
// NOTE: Unity build (one translation unit), which makes it fast to compile and easier to port
#include "win64_timer.c"
#include "win64_audio_wasapi.c"
#include "win64_xinput.c"
    
#if defined(RENDERER_OGL1_2D) || defined(RENDERER_OGL1_3D) || defined(RENDERER_SOFTWARE_OGL1)
    #include "win64_opengl.c"
#endif

#ifdef RENDERER_OGL4
    #include "win64_opengl4.c"
#endif

void win64ToggleFullscreen()
{
    //SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2); // requires win10
    persist WINDOWPLACEMENT windowPosition = { sizeof(windowPosition) };
    DWORD windowStyleFlags = GetWindowLong(win64State.window, GWL_STYLE);
    if (windowStyleFlags & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO monitorInfo = { sizeof(monitorInfo) };
        if (GetWindowPlacement(win64State.window, &windowPosition) &&
            GetMonitorInfo(MonitorFromWindow(win64State.window, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
        {
            // use SetWindowLongPtr for both 32 and 64bit builds
            SetWindowLong(win64State.window, GWL_STYLE, windowStyleFlags & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(win64State.window, HWND_TOP,
                        monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                        monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                        monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                        SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(win64State.window, GWL_STYLE, windowStyleFlags | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(win64State.window, &windowPosition);
        SetWindowPos(win64State.window, 0, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                    SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}
    
#ifdef HOTLOAD
// used when we are hotloading to determine when the file was created
// cause if the user has compiled a new dll we want to load the new one
internal FILETIME
win64GetLastWriteTime(char *filename)
{
    FILETIME result = {0};
    WIN32_FIND_DATA data;
                    
    if (GetFileAttributesEx(filename, GetFileExInfoStandard, &data))
    {
        result = data.ftLastWriteTime;
    }
                    
    return result;
}
    
internal Win64AppCode
win64LoadAppCode(char *dllFilename, char *dllTempFilename)
{
    Win64AppCode result = {0};
                    
    result.dllLastWriteTime = win64GetLastWriteTime(dllFilename);
                    
    // copy the file and load the temp copy
    // this is so we can compile a new version while were still running
    CopyFile(dllFilename, dllTempFilename, FALSE);
    result.gameCodeDLL = LoadLibraryA(dllTempFilename);
    if(result.gameCodeDLL)
    {
        result.updateStateAndRender = (UpdateStateAndRenderPtr)GetProcAddress(result.gameCodeDLL, "updateStateAndRender");
        result.initialize = (InitializeFuncPtr)GetProcAddress(result.gameCodeDLL, "initialize");
        result.onLoad = (OnLoadFuncPtr)GetProcAddress(result.gameCodeDLL, "onLoad");
        result.cleanUp = (CleanUpFuncPtr)GetProcAddress(result.gameCodeDLL, "cleanUp");
                                    
        result.isValid = (result.updateStateAndRender &&
                        result.initialize &&
                        result.onLoad &&
                        result.cleanUp);
    }
                    
    if(!result.isValid)
    {
        result.initialize = 0;//initializeTemp;
        result.updateStateAndRender = 0;//updateStateAndRenderTemp;
        result.onLoad = 0;
        result.cleanUp = 0; //cleanupTemp;
    }
                    
    return result;
}
    
internal void
win64UnloadAppCode(Win64AppCode *gameCode)
{
    if(gameCode->gameCodeDLL)
    {
        FreeLibrary(gameCode->gameCodeDLL);
        gameCode->gameCodeDLL = 0;
    }
                    
    gameCode->isValid = false;
    gameCode->updateStateAndRender = 0;//updateStateAndRenderTemp;
    gameCode->initialize = 0;//initializeTemp;
    gameCode->onLoad = 0;
    gameCode->cleanUp = 0;//cleanupTemp;
}
    
    
internal void
win64GetInputFileLocation(Win64State *state, b32 inputStream, i32 slotIndex, char *dest, i32 destCount)
{
    char temp[64];
    // one file holds the inputstream and the other one the appState
    wsprintf(temp, "playback_%d_%s.hmi", slotIndex, inputStream ? "input" : "state");
                    
    cStringConcat(win64State.filepath, temp, dest, destCount);    
}
    
    
// the recorded file consist of the memory(appState + empty memory space) + every frame of the input struct
internal void
win64BeginRecordingInput(Win64State *state, int inputRecordingIndex)
{
    state->inputRecordingIndex = inputRecordingIndex;
                    
    char filename[MAX_PATH];
    //win64GetInputFileLocation(state, true, inputPlayingIndex, filename, sizeof(filename));
    cStringConcat(win64State.filepath, "playback.inp", filename, sizeof(filename));    
                    
    state->recordingHandle = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
                	
    DWORD bytesToWrite = (DWORD)state->appMemorySize;
    assert(state->appMemorySize == bytesToWrite);
    DWORD bytesWritten;
    // write the whole gamestate to the file
    WriteFile(state->recordingHandle, state->appMemory, bytesToWrite, &bytesWritten, 0);    
}
    
internal void
win64EndRecordingInput(Win64State *state)
{
    CloseHandle(state->recordingHandle);
    state->inputRecordingIndex = 0;
}
    
internal void 
win64BeginInputPlayBack(Win64State *state, i32 inputPlayingIndex)
{
    state->inputPlayingIndex = inputPlayingIndex;
                    
    char filename[MAX_PATH];
    //win64GetInputFileLocation(state, true, inputPlayingIndex, filename, sizeof(filename));
    cStringConcat(win64State.filepath, "playback.inp", filename, sizeof(filename));    
                    
    state->playbackHandle =
        CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
                    
    // cast u64 to u32 and then assert they are the same to ensure that we're not reading more than
    // 4 gigs of data which is tha maximum that WriteFile can handle
    DWORD bytesToRead = (DWORD)state->appMemorySize;
    assert(state->appMemorySize == bytesToRead);
                    
    DWORD bytesRead;
    // read in the appState
    ReadFile(state->playbackHandle, state->appMemory, bytesToRead, &bytesRead, 0);    
}
    
internal void
win64EndInputPlayBack(Win64State *state)
{
    CloseHandle(state->playbackHandle);
    state->inputPlayingIndex = 0;
}
    
internal void 
win64RecordInput(Win64State *state, Input *input)    
{
    DWORD bytesWritten;
    // write input to file,  1044bytes (size of input) at a time
    WriteFile(state->recordingHandle, input, sizeof(*input), &bytesWritten, 0);
}
    
internal void win64PlayBackInput(Win64State *state, Input *input)
{
    DWORD bytesRead = 0;
    // read file 1044 bytes (size of input) at a time to the input
    if(ReadFile(state->playbackHandle, input, sizeof(*input), &bytesRead, 0))
    {
        // end of file, start read from beginning of file
        if(bytesRead == 0)
        {
            i32 playingIndex = state->inputPlayingIndex;
            win64EndInputPlayBack(state);//close the file
            win64BeginInputPlayBack(state, playingIndex);//open the file again
            ReadFile(state->playbackHandle, input, sizeof(*input), &bytesRead, 0); // read first chunk of input
        }
    }
}
#endif // HOTLOAD
    
#if 0
internal inline void
updateKey(b32 isDown, u32 keyIndex)
{
    //	if (keyCode == keyIndex) {
    input.keys[keyIndex].changed = isDown != input.keys[keyIndex].isDown;
    input.keys[keyIndex].isDown = isDown;
    //}
}
#endif
    
internal void
win64UpdateGamepadButton(DWORD gamepadState, DWORD buttonBitTest, ButtonState *button)
{
    b32 isDown = (gamepadState & buttonBitTest);
    button->changed = isDown != button->isDown;
    button->isDown = isDown;
}
    
internal f32 
win64UpdateGamepadStick(i16 stickValue, i32 deadZoneThreshold)
{
    // return 0 if we got a value that are in the dead zone
    // so we get rid of the "deadzone noise"
    f32 result = 0;
                    
    // sThumbLX is a short, so the readings in X-Y direction range between between -32768 and 32767 (range of short)
    // normalize values (unit length), -1 to 1
    if (stickValue < -deadZoneThreshold) 
        //result = stickValue / 32768.0f;
        // NOTE: if we say that the deadzone ends at 0.2, this will map the value to 0 - 1 instead of 0.2 - 1
        result = (stickValue + deadZoneThreshold) / (32768.0f - deadZoneThreshold);
    else if (stickValue > deadZoneThreshold)
        //result = stickValue / 32767.0f;
        result = (stickValue - deadZoneThreshold) / (32767.0f - deadZoneThreshold);            
                    
    return result;
}
    
internal void
win64UpdateMouseButton(ButtonState *button, b32 isDown)
{
    button->changed = isDown != button->isDown;
    button->isDown = isDown;
}
    
internal inline void
win64UpdateMouse(ButtonState *button, b32 isDown)
{
    button->changed = isDown != button->isDown;
    button->isDown = isDown;
}
    
internal void
win64ProcessInputAndMessages(Input *input)
{
                    
    // NOTE: Clear input
    for (i32 i = 0; i < KEY_COUNT; i++)
    input->keys[i].changed = false;
                    
    for (i32 i = 0; i < MOUSE_BUTTONS_COUNT; i++) 
        input->mouseButtons[i].changed = false;
                    
    input->mouseWheelDelta = 0;
                    
    MSG msg;
    // NOTE: when calling a windows function, windows can suddenly call WindowProc with different messages outside of the message loop
    while(PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
    {
        switch (msg.message)
        {
            case WM_QUIT:
            {
                win64State.running = false;
            } break;
                                                    
            // NOTE: handles ascii
            case WM_CHAR:
            {
                assert(!"Error");
#if 1
                // get the character
                u8 asciiCode = (char)msg.wParam;
                u32 keyState = (u32)msg.lParam;
                // print the ascii code and key state
                debugPrint("WM_CHAR: Character = %c", asciiCode);
                debugPrint("Key State = 0X%X", keyState);
                                                                    
                /*#define ALT_STATE_BIT 0x20000000 
                if (key_state & ALT_STATE_BIT) 
                { 
                                                                                    
                } */
#endif
            } break;
                                                    
            #define processButton(b) \
            input->keys[b].changed = isDown != input->keys[b].isDown; \
            input->keys[b].isDown = isDown; \
                                                                    
            // NOTE: uses scancodes
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                // LPARAM is signed 64-bit on x64
                // WPARAM is unsigned 64bits on x64
                // get virtual key code
                u64 keyCode = msg.wParam;
                // bit 30 in lParam tells the previous key state, if it is set the key was previously down, else it was up
                // set bit 30 and AND it with lParam, wasDown = true if it is set
                b32 wasDown = ((msg.lParam & (1 << 30)) != 0);
                b32 isDown = ((msg.lParam & (1 << 31)) == 0); // bit 31 is set when you release a key (WM_KEYUP message)
                b32 altDown = msg.lParam & (1 << 29);
                
                // KEY_SPACE to KEY_9
                if (keyCode >= 32 && keyCode <= 57)
                {
                    processButton(KEY_SPACE + (keyCode-VK_SPACE));
                }                      
                else if (keyCode >= 'A' && keyCode<= 'Z') // scancodes are mapped to capital letters
                {
                    processButton(KEY_A + (keyCode-'A'));
                }
                else if (keyCode >= VK_F1 && keyCode <= VK_F12)
                {
                    processButton(KEY_F1 + (keyCode-VK_F1));
                }
                
                switch (keyCode)
                {
                    case VK_BACK:
                    {
                        processButton(KEY_BACKSPACE);
                    } break;
                    
                    case VK_TAB:
                    {
                        processButton(KEY_TAB);
                    } break;
                    
                    case VK_RETURN:
                    {
                        processButton(KEY_ENTER);
                    } break;
                    
                    case VK_SHIFT:
                    {
                        processButton(KEY_SHIFT);
                    } break;
                    
                    case VK_CONTROL:
                    {
                        processButton(KEY_CONTROL);
                    } break;
                    
                    case VK_MENU:
                    {
                        processButton(KEY_ALT);
                    } break;
                    
                    case VK_PAUSE:
                    {
                        processButton(KEY_PAUSE);
                    } break;
                    
                    case VK_CAPITAL:
                    {
                        processButton(KEY_CAPS_LOCK);
                    } break;
                    
                    case VK_ESCAPE:
                    {
                        processButton(KEY_ESCAPE);
                    } break;
                    
                    case VK_NUMLOCK:
                    {
                        processButton(KEY_NUM_LOCK);
                    } break;
                    
                    case VK_SCROLL:
                    {
                        processButton(KEY_SCROLL_LOCK);
                    } break;
                    
                    case VK_LSHIFT:
                    {
                        processButton(KEY_LEFT_SHIFT);
                    } break;
                    
                    case VK_RSHIFT:
                    {
                        processButton(KEY_RIGHT_SHIFT);
                    } break;
                    
                    case VK_LCONTROL:
                    {
                        processButton(KEY_LEFT_CONTROL);
                    } break;
                    
                    case VK_RCONTROL:
                    {
                        processButton(KEY_RIGHT_CONTROL);
                    } break;
                    
                    case VK_OEM_PLUS:
                    {
                        processButton(KEY_EQUAL);
                    } break;
                    
                    case VK_OEM_COMMA:
                    {
                        processButton(KEY_COMMA);
                    } break;
                    
                    case VK_OEM_MINUS:
                    {
                        processButton(KEY_MINUS);
                    } break;
                    
                    case VK_OEM_PERIOD:
                    {
                        processButton(KEY_PERIOD);
                    } break;
                    
                    case VK_OEM_1:
                    {
                        processButton(KEY_SEMICOLON);
                    } break;
                    
                    case VK_OEM_2:
                    {
                        processButton(KEY_FORWARD_SLASH);
                    } break;
                    
                    case VK_OEM_3:
                    {
                        processButton(KEY_GRAVE);
                    } break;
                    
                    case VK_OEM_4:
                    {
                        processButton(KEY_LEFT_BRACKET);
                    } break;
                    
                    case VK_OEM_5:
                    {
                        processButton(KEY_BACKSLASH);
                    } break;
                    
                    case VK_OEM_6:
                    {
                        processButton(KEY_RIGHT_BRACKET);
                    } break;
                    
                    case VK_OEM_7:
                    {
                        processButton(KEY_QUOTE);
                    } break;
                }
                
                if (keyCode == VK_F4 && altDown)
                {
                    win64State.running = false;
                    break;
                }
                else if (keyCode == VK_ESCAPE)
                {
                    win64State.running = false;
                    break;
                }
                else if(keyCode == VK_F4)
                {
                    if(isDown)
                    {
                        win64State.pause = !win64State.pause;
                    }
                }
                else if (keyCode == VK_RETURN && altDown && isDown && isDown != wasDown)
                {
                    win64ToggleFullscreen();
                }
                                                                    
                #ifdef HOTLOAD
                else if(keyCode == VK_F1)
                {
                    if(isDown)
                    {
                        // we're not playingback
                        if(win64State.inputPlayingIndex == 0)
                        {
                            // we're not recording
                            if(win64State.inputRecordingIndex == 0)
                            {
                                win64BeginRecordingInput(&win64State, 1);
                            }
                            else
                            {
                                win64EndRecordingInput(&win64State);
                                win64BeginInputPlayBack(&win64State, 1);
                            }
                        }
                        // stop playback if we're playingback
                        else 
                        {
                            win64EndInputPlayBack(&win64State);
                        }
                    }
                }
                #endif // HOTLOAD
            } break;
                                                    
            case WM_LBUTTONDOWN:
            {
                win64UpdateMouse(&input->mouseButtons[MOUSE_LEFT], true);
            } break;
                                                    
            case WM_LBUTTONUP:
            {
                win64UpdateMouse(&input->mouseButtons[MOUSE_LEFT], false);
            } break;
                                                    
            case WM_MBUTTONDOWN:
            {
                win64UpdateMouse(&input->mouseButtons[MOUSE_MIDDLE], true);
            } break;
                                                    
            case WM_MBUTTONUP:
            {
                win64UpdateMouse(&input->mouseButtons[MOUSE_MIDDLE], false);
                                                                    
            } break;
                                                    
            case WM_RBUTTONDOWN:
            {
                win64UpdateMouse(&input->mouseButtons[MOUSE_RIGHT], true);
                                                                    
            } break;
#if 0
            case WM_XBUTTONDOWN:
            {
                b32 x1Down = (msg.wParam & (1 << 17));
                b32 x2Down = (msg.wParam & (1 << 16));
                                                                    
                if ((msg.wParam & (1 << 17)))
                {
                    win64UpdateMouse(&input->mouseButtons[MOUSE_FORWARD], true);
                }
                                                                    
                if ((msg.wParam & (1 << 16)))
                {
                    win64UpdateMouse(&input->mouseButtons[MOUSE_BACK], true);
                }
                                                                    
            } break;
                                                    
            case WM_XBUTTONUP:
            {
                b32 x1Down = ((msg.wParam & (1 << 17)) != 0);
                b32 x2Down = (msg.wParam & (1 << 16));
                                                                    
                if (x1Down)
                {
                    //win64UpdateMouse(&input->mouseButtons[MOUSE_FORWARD], false);
                }
                if (x2Down)
                {
                // win64UpdateMouse(&input->mouseButtons[MOUSE_BACK], false);
                }
                                                                    
            } break;
#endif
            case WM_RBUTTONUP:
            {
                win64UpdateMouse(&input->mouseButtons[MOUSE_RIGHT], false);
                                                                    
                // NOTE: Pop-up menu
                POINT point;                            
                point.x = LOWORD(msg.lParam);
                point.y = HIWORD(msg.lParam);
                ClientToScreen(win64State.window, &point);        
                TrackPopupMenu(win64State.menu, TPM_RIGHTBUTTON, point.x, point.y, 0, win64State.window, NULL);
                                                                    
            } break;
                                                    
            case WM_MOUSEWHEEL:
            {
                i16 wheelDelta = HIWORD(msg.wParam);
                // wheel rotation is a multiple of WHEEL_DELTA, which is set to 120
                input->mouseWheelDelta = (i32)wheelDelta / WHEEL_DELTA;
            } break;
                                                    
            default:
            {
                // translate Windows virtual key character codes to actual char symbols
                TranslateMessage(&msg);
                // send message to WindowProc() for processing
                // this calls code in user32.dll then call us back through WindowProc
                DispatchMessageA(&msg);
            } break;
        }
    }
                    
    // NOTE: Update mouse and gamepad input
                    
    POINT mouseP;
    GetCursorPos(&mouseP);
    // get position relative to window
    ScreenToClient(win64State.window, &mouseP);
                    
                    
    // the highorder bit is set to 1 if the button is down
    //win64UpdateMouseButton(&input->mouseButtons[MOUSE_LEFT],GetKeyState(VK_LBUTTON) & (1 << 15));
    //win64UpdateMouseButton(&input->mouseButtons[MOUSE_MIDDLE],GetKeyState(VK_MBUTTON) & (1 << 15));
    //win64UpdateMouseButton(&input->mouseButtons[MOUSE_RIGHT],GetKeyState(VK_RBUTTON) & (1 << 15));
    // TODO: the X-buttons seems to get a down and up message each time. Something wrong with my mouse?
    //win64UpdateMouseButton(&input->mouseButtons[MOUSE_FORWARD],GetKeyState(VK_XBUTTON1) & (1 << 15));
    //win64UpdateMouseButton(&input->mouseButtons[MOUSE_BACK],GetKeyState(VK_XBUTTON2) & (1 << 15));
                    
    input->mouseX = mouseP.x;
    input->mouseY = mouseP.y; // NOTE: if BOTTOM up DIB: backBuffer.height-mouseP.y;
                    
    if (input->mouseX != input->mousePrevX || input->mouseY != input->mousePrevY)
        input->mouseMoved = true;
    else
        input->mouseMoved = false;
                    
    if (input->mouseButtons[MOUSE_LEFT].isDown == 1 && (input->mouseX != input->mousePrevX || input->mouseY != input->mousePrevY))
        input->mouseDragged = true;
    else
        input->mouseDragged = false;
                    
                    
    // NOTE: this has to do when you unplugg the controller, it seems to get a new index, and you have to restart windows
    // Is this just on my system? TODO: Test with newer controller
    i32 gamepadsConnected = 0;
    XINPUT_STATE controllerState;
    i32 controllerIndex[2];
    controllerIndex[0] = win64GetXInputGamepad(&controllerState, 0);
    if (controllerIndex[0] != -1)
    {
        input->gamepad[0].isConnected = true;
        gamepadsConnected++;
    }
    else
        input->gamepad[0].isConnected = false;
                    
    // get second controller
    controllerIndex[1] = win64GetXInputGamepad(&controllerState, controllerIndex[0]);
    if (controllerIndex[1] != -1)
    {
        input->gamepad[1].isConnected = true;
        gamepadsConnected++;
    }
    else
        input->gamepad[1].isConnected = false;
                    
                    
    // loop over the 2 controllers
    for (i32 i = 0; i < gamepadsConnected; ++i)
    {
        //XINPUT_STATE controllerState;
        if (XInputGetState(controllerIndex[i], &controllerState) == ERROR_SUCCESS)
        {
            //input->gamepadInput[i].isConnected = true;
            XINPUT_GAMEPAD *pad = &controllerState.Gamepad;
            GamepadInput *in = &input->gamepad[i];
                                                    
            in->leftStickX = win64UpdateGamepadStick(pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
            in->leftStickY = win64UpdateGamepadStick(pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
            in->rightStickX = win64UpdateGamepadStick(pad->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
            in->rightStickY = win64UpdateGamepadStick(pad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
                                                    
            //debugPrintVariable(f, in->rightStickX);
            //debugPrintVariable(f, in->rightStickY);
            //debugPrintVariable(f, in->leftStickX);
            //debugPrintVariable(f, in->leftStickY);
                                                    
            // value between 0 and 255 for the left/right trigger analog control
            in->leftTrigger = pad->bLeftTrigger;
            in->rightTrigger = pad->bRightTrigger;
                                                    
            win64UpdateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_DPAD_UP, &in->gamepadButtons[GAMEPAD_UP]);
            win64UpdateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_DPAD_DOWN, &in->gamepadButtons[GAMEPAD_DOWN]);
            win64UpdateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_DPAD_LEFT, &in->gamepadButtons[GAMEPAD_LEFT]);
            win64UpdateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_DPAD_RIGHT, &in->gamepadButtons[GAMEPAD_RIGHT]);
            win64UpdateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_START, &in->gamepadButtons[GAMEPAD_START]);
            win64UpdateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_BACK, &in->gamepadButtons[GAMEPAD_BACK]);
            win64UpdateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_LEFT_THUMB, &in->gamepadButtons[GAMEPAD_LEFT_THUMB]);
            win64UpdateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_RIGHT_THUMB, &in->gamepadButtons[GAMEPAD_RIGHT_THUMB]);
            win64UpdateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER, &in->gamepadButtons[GAMEPAD_LEFT_SHOULDER]);
            win64UpdateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER, &in->gamepadButtons[GAMEPAD_RIGHT_SHOULDER]);
            win64UpdateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_A, &in->gamepadButtons[GAMEPAD_A]);
            win64UpdateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_B, &in->gamepadButtons[GAMEPAD_B]);
            win64UpdateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_X, &in->gamepadButtons[GAMEPAD_X]);
            win64UpdateGamepadButton(pad->wButtons, XINPUT_GAMEPAD_Y, &in->gamepadButtons[GAMEPAD_Y]);
        }
        else
        {
            // NOTE: gamepad is not available
            input->gamepad[i].isConnected = false;
        }
    }
}
    
// creates an 32 pit per pixel  DIB = Device Independant Bitmap (windows name for bitmaps when used with GDI)
// resize the window draw area, the backBuffer that we draw to
// 3 bytes of color (RGB) = 24 bits, 1 byte for padding
internal void 
win64CreateBackbuffer(Win64BackBuffer *buffer, int width, int height)
{
    // if memory exists, release it before we create a new draw area
    if (buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }
                    
    buffer->width = width;
    buffer->height = height;
                	
    // each pixel are 4 bytes (just 3 are used) the fourth is just padding so that we area aligned on four byte boundaries
    buffer->bytesPerPixel = 4; 
    // StretchDIBits needs this structure
    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    // NOTE: if biHeight is positive the bitmap is a bottom-up DIB and it's origin is the lower-left corner, 
    // if it is negative, the bitmap is a top-down DIB and it's origin is the upper-left corner
    buffer->info.bmiHeader.biHeight = -buffer->height;	
    buffer->info.bmiHeader.biPlanes = 1;            // not used
    buffer->info.bmiHeader.biBitCount = 32;         // bits per pixel
    buffer->info.bmiHeader.biCompression = BI_RGB;  // no compression
                    
    // create the backbuffer
    int bitmapMemorySize = (buffer->width * buffer->height) * buffer->bytesPerPixel;
                	
    // allocates certain amounts of memory pages (often 4096 bytes)
    // with VirtualAlloc you can't allocate less than 4096 bytes = 1 page
    buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    // how much we should move to advance to the next row of pixels. 
    buffer->pitch = width * buffer->bytesPerPixel;
                    
}
    
internal void 
win64UpdateWindow(HDC deviceContext, Win64BackBuffer *buffer)
{
    if (!win64State.backBufferDeveloper)
    {
        // rectangle to rectangle copy
        // copy from one buffer to another and stretches it if needed to fit the destination
        StretchDIBits(deviceContext,
                    0, 0, win64State.windowWidth, win64State.windowHeight,
                    0, 0, buffer->width, buffer->height,
                    buffer->memory,
                    &buffer->info,
                    DIB_RGB_COLORS,
                    SRCCOPY);
    }
    else
    {
        StretchDIBits(deviceContext,
                    0, 0, buffer->width, buffer->height,
                    0, 0, buffer->width, buffer->height,
                    buffer->memory,				// pointer to image bits stored in an array of bytes
                    &buffer->info,
                    DIB_RGB_COLORS,				// RGB color format 
                    SRCCOPY);				  	// raster-operation, copy source rect to dest rect
    }
                     
       
}
    
LRESULT CALLBACK 
win64WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    // 0 = we handled the message
    LRESULT result = 0;
    switch(message)
    {
        case WM_SIZE:
        {
            // TODO: getClientRect in the main loop insted?
            // gets the draw area of the window
            RECT clientRect;
            GetClientRect(window, &clientRect);	
            win64State.windowWidth = clientRect.right - clientRect.left;
            win64State.windowHeight = clientRect.bottom - clientRect.top;
                                                    
            // TODO: check if these does the same
            //state->windowWidth = LOWORD(lParam);
            //state->windowHeight = HIWORD(lParam);
                                                    
            if (!win64State.backBufferFixed)
            {
                #ifdef RENDERER_SOFTWARE_GDI
                    win64CreateBackbuffer(&win64BackBuffer, win64State.windowWidth, win64State.windowHeight);
                #endif
                //updateAndRender(&backBuffer, &soundBuffer, &memory); 
                //win64UpdateWindow(&win64BackBuffer);
            }
            else if (win64State.backBufferFixed)
            {
                //win64CreateBackbuffer(&win64BackBuffer, win64State.windowWidth, win64State.windowHeight);
                //win64OpenGLSet2dProjection(win64State.windowWidth, win64State.windowHeight);
                //win64BackBuffer.width = win64State.windowWidth;
                //win64BackBuffer.height = win64State.windowHeight;
                #ifdef RENDERER_SOFTWARE_OGL1
                    glViewport(0, 0, win64State.windowWidth, win64State.windowHeight);
                #endif
            }
            
            #ifdef RENDERER_OGL1_2D
                //oglChangeSize(win64State.windowWidth, win64State.windowHeight);
                win64OpenGLSet2dProjection(win64State.windowWidth, win64State.windowHeight);
                win64BackBuffer.width = win64State.windowWidth;
                win64BackBuffer.height = win64State.windowHeight;
            #endif
            
            #ifdef RENDERER_OGL1_3D
                win64OpenGLSet3dProjection(win64State.windowWidth, win64State.windowHeight, 45.f, .1f, 100.0f);
            #endif            
            
            #ifdef RENDERER_OGL4
                glViewport(0, 0, win64State.windowWidth, win64State.windowHeight);
            #endif                                                    
                                                    
            #if DEVELOPER
                i32 width     = LOWORD(lParam); // width of client area
                i32 height    = HIWORD(lParam); // height of client area            
                debugPrint("WM_SIZE called -  new size (%d,%d)", width, height);
            #endif                            
        } break;
                                    
        case WM_MOVE:
        {
            #if DEVELOPER
                i32 xPos = LOWORD(lParam);
                i32 yPos = HIWORD(lParam);
                debugPrint("WM_MOVE called -  new position (%d,%d)", xPos, yPos);
            #endif
        } break;
                                    
        case WM_QUIT:
        case WM_CLOSE:
        case WM_DESTROY: {
            win64State.running = false;
        } break;
    
        // called when the window needs to repaint
        case WM_PAINT: // when resizing, minimizing/maximizing the window
        {
            // BeginPaint, EndPaint - tells windows to validate the window
            // if we don't do this windows will kepp sending WM_PAINT messages
            // if we delete the WM_PAINT case, windows will take care of this automatically
            PAINTSTRUCT paint;
            HDC deviceContext = BeginPaint(window, &paint);
                                                    
            // if we don't call this here the window will not repaint if you move it in and out of the monitor area
            // TODO: other renderers
            win64UpdateWindow(deviceContext, &win64BackBuffer);
            #if DEVELOPER
                persist int wmPaintCount = 0;
                debugPrint("WM_PAINT called %d times", ++wmPaintCount);
            #endif
            EndPaint(window, &paint);  
        } break;
                                    
        case WM_COMMAND:
        {
            win64State.processMenuFuncPtr(LOWORD(wParam));
                                    
        } break;
        case WM_ACTIVATEAPP:
        {
            if (wParam == true)
            {
                SetLayeredWindowAttributes(window, RGB(0, 0, 0), 255, LWA_ALPHA);
            }
            else
            {
                SetLayeredWindowAttributes(window, RGB(0, 0, 0), 127, LWA_ALPHA);
            }
        } break;
                                    
        default:
        {
            // let windows take care of any unhandled messages
            result = DefWindowProcA(window, message, wParam, lParam); 
        } break;
    }
                    
    return result;
}
    
void createWindow_(i32 winWidth, i32 winHeight, b32 fullscreen, char *title)
{
                    
    // this is so GetMonitorInfo returns the right values.
    // if you change size of text and apps in windows settings it will return wrong values
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
                    
    MONITORINFO gMonitorInfo = { sizeof(MONITORINFO) };
                    
    // get info about monitor
    if (GetMonitorInfoA(MonitorFromWindow(win64State.window, MONITOR_DEFAULTTOPRIMARY), &gMonitorInfo) == 0)
    {
        invalidCodePath
    }
                    
    // TODO: store these?
    i32 monitorWidth = gMonitorInfo.rcMonitor.right - gMonitorInfo.rcMonitor.left;
    i32 monitorHeight = gMonitorInfo.rcMonitor.bottom - gMonitorInfo.rcMonitor.top;
                    
    if (winWidth == 0 && winHeight == 0)
    {
        winWidth = GetSystemMetrics(SM_CXSCREEN);
        winHeight = GetSystemMetrics(SM_CYSCREEN);
    }
    // Create back buffer
    win64CreateBackbuffer(&win64BackBuffer, winWidth, winHeight);
                    
    i32 trueWindowWidth = 0;
    i32 trueWindowHeight = 0;
                    
    // calculates the required size of the window rectangle, based on the desired client-rectangle size,
    // so if we specify 960x540 we get a drawing are of that size
    {
        RECT windowRect;
        windowRect.left = 0;
        windowRect.right = winWidth;
        windowRect.top = 0;
        windowRect.bottom = winHeight;            
                                    
        if (AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, 0))
        {
            trueWindowWidth = windowRect.right - windowRect.left;
            trueWindowHeight = windowRect.bottom - windowRect.top;
        }
    }
                    
    // WNDCLASSA is the ANSI/ASCII version, and the default when you compile with cl, so WNDCLASS is the same
    WNDCLASSA windowClass = {0};
    {
        // CS_HREDRAW | CS_VREDRAW - When we resize the window the whole window will be repainted
        windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        windowClass.lpfnWndProc = win64WindowProc;
        windowClass.hInstance = GetModuleHandle(0);
        windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        windowClass.hIcon = (HICON)LoadImage(0, "icon.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
        //windowClass.hIcon = LoadIcon(NULL, IDI_ASTERISK );
        windowClass.hCursor = LoadCursor(0, IDC_CROSS); // IDC)ARROW
        windowClass.lpszClassName = "ZIZWindowClass";
    }
                    
    if (RegisterClassA(&windowClass) == 0)
    {
        quitPlatformError("Fatal error", "Failed to initialize window class.");
    }
                    
    DWORD dwExStyle;
    DWORD dwStyle;
    if (fullscreen)
    {
        dwExStyle=WS_EX_APPWINDOW;
        dwStyle=WS_POPUP;
        //ShowCursor(FALSE);
        trueWindowWidth = winWidth;
        trueWindowHeight = winHeight;
    }
    else
    {
        dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        dwStyle= WS_OVERLAPPEDWINDOW;
    }
    
    // WS_EX_LAYERED - transperant window WS_EX_TOPMOST else just put a 0
    HWND window = CreateWindowExA(dwExStyle, windowClass.lpszClassName, title,
                                dwStyle,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                trueWindowWidth, trueWindowHeight, 0, 0, GetModuleHandle(0), 0);
                    
    if (!window)
    {
        quitPlatformError("Fatal error", "Failed to create window.");
    }
    
    
    // get a graphics context
    HDC deviceContext = GetDC(window);
                    
    // NOTE: Get monitor refresh rate
    i32 monitorRefreshHz = GetDeviceCaps(deviceContext, VREFRESH);
    ReleaseDC(window, deviceContext);
    
    // 60hz = 0.01667 seconds/frame, 30hz = 0.03333 seconds/frame
    // the delta time, the time it takes to draw a frame
    f32 frameTargetSeconds;
    i32 appRefreshHz = 0;
                    
    // if FPS was not set by the user
    // TODO: setFPS function
    if (appRefreshHz)
    {
        frameTargetSeconds = 1.f / (f32)appRefreshHz;
    }
    else
    {
        frameTargetSeconds = 1.f / (f32)monitorRefreshHz;
        appRefreshHz = monitorRefreshHz;
    }
    
    // NOTE: Windows platform initialization
    {
        win64State.window = window;
        win64State.running = true;
        win64State.lockFPS = true;
        win64State.backBufferFixed = true;
        win64State.backBufferDeveloper = false; // HH style
        win64State.windowWidth = winWidth;
        win64State.windowHeight = winHeight;
        win64State.monitorRefreshHz = monitorRefreshHz;
        win64State.deltaTime = win64State.frameTargetSeconds = frameTargetSeconds;
    }
                    
    // NOTE: Renderer init
    #if defined(RENDERER_SOFTWARE_OGL1) || defined(RENDERER_OGL1_2D) || defined(RENDERER_OGL1_3D)
        win64OpenGLInit(&win64State);
    #endif
    
    #ifdef RENDERER_OGL4
        win64OpenGL4Init(&win64State);
    #endif
    
    #ifdef RENDERER_DIRECTX11
        // win64DirectX11Init(&win64State);
    #endif                    
    
    
#if 1
    if (fullscreen)
    {
        // device mode
        DEVMODE dmScreenSettings;
        memset(&dmScreenSettings,0,sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = winWidth;
        dmScreenSettings.dmPelsHeight = winHeight;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
        
        if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
        {
            if (MessageBox(0, "The requested fullscreen mode is not supported by\nyour video card. Use windowed mode?","ZiZ",
                           MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
            {
                fullscreen = false;
            }
            else
            {
                ExitProcess(1);
            }
        }
    }
    else
    {
        // TODO: this is not working in release mode, find out why
        // center window
        RECT rc;
        GetWindowRect(window, &rc);
                                    
        i32 xPos = (GetSystemMetrics(SM_CXSCREEN) - rc.right)/2;
        i32 yPos = (GetSystemMetrics(SM_CYSCREEN) - rc.bottom)/2;
                                    
        SetWindowPos(window, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }
#endif
    
    win64State.deviceContext = GetDC(window);
    ShowWindow(window, SW_NORMAL);
    
    randomSeed(GetTickCount());
}
    
////////////////////////////////////
//~ WinMain

#ifdef NOCRT
void __stdcall 
WinMainCRTStartup()
#else
int CALLBACK 
WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
#endif
{
    // check for memory leak if debug build
    #if DEVELOPER  
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
    #endif
                    
    // NOTE: Get filenames and directories
    {
        // get the working directory
        GetCurrentDirectory(sizeof(win64State.workingDirectory), win64State.workingDirectory);
                                                    
        char filepath[MAX_PATH];
        DWORD sizeOfFilepath = GetModuleFileNameA(0, filepath, sizeof(filepath));
        char *filename = filepath;
        // get filename
        for(char *i = filepath; *i; ++i)
        {
            // eat characters up until last '\\' to get the filename
            if(*i == '\\')
            {
                filename = i + 1;
            }
        }
                                    
        GetModuleFileNameA(0, win64State.filepath, sizeof(filepath));
        // win64State.filepath = GetCommandLine();
        win64State.filename = filename;
        i32 len = cStringLength(win64State.filename);
        i32 lenPath = cStringLength(win64State.filepath);
        // remove filename
        win64State.filepath[lenPath-len] = 0;
    }
                    
    #ifdef HOTLOAD
        char *dllFilename = "ziz.dll";        
        char dllFullPath[256];
        cStringConcat(win64State.filepath, dllFilename, dllFullPath, 256);
                                            
        char *dllTempFilename = "ziz_temp.dll";
        char dllTempFullPath[256];
        cStringConcat(win64State.filepath, dllTempFilename, dllTempFullPath, 256);
    #endif
                    
    // NOTE: Memory allocation
    AppMemory memory = {0};
    {
        #if DEVELOPER
            //get an baseadress for the start of the memory
            //only used in debug build so we can do the looped live editing
            LPVOID baseAddress = (LPVOID)terabytes(2);
        #else
            // 0 = let Windows decide the memory base address
            LPVOID baseAddress = 0;
        #endif
                                    
        memory.permanentMemorySize = megabytes(64);
                                    
        // don't have to persist, transient scratch space
        memory.tempMemorySize = megabytes(256);
                                    
                                    
        u64 totalSize = memory.permanentMemorySize + memory.tempMemorySize;
        // allocate all the memory and clears it to zero
        // iff the first parameter is 0, the system determines where to allocate the region.
        memory.permanentMemory = VirtualAlloc(baseAddress, (size_t)totalSize,
                                            MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
                                    
        // put the pointer where the temporary memory begins
        memory.tempMemory = ((u8 *)memory.permanentMemory + memory.permanentMemorySize);
                                    
        memory.createWindow = createWindow_;
        memory.setWindowTitle = setWindowTitle;
        memory.createPopUpMenu = createPopUpMenu_;
        memory.menuAdd = menuAdd_;
        memory.menuAddSeparator = menuAddSeparator_;
        memory.destroyPopUpMenu = destroyPopUpMenu_;
        memory.quitError = quitError_;
        memory.loadFile = loadFile_;
        memory.loadTextFile = loadTextFile_;
        memory.freeFile = freeFile_;
        memory.freeMemory = freeMemory_;
#if defined(RENDERER_OGL1_2D) || defined(RENDERER_OGL1_3D)
        memory.loadBitmap = loadBitmap_;
#endif
        #ifdef RENDERER_OGL4
            memory.openGL4LoadFunction = win64OpenGL4LoadFunction;
        #endif                                    
        
        #ifdef HOTLOAD
            win64State.appMemory = memory.permanentMemory;
            win64State.appMemorySize = totalSize;
            memory.codeReload = true;
        #endif
        
        // create memory for the arena
        //initializeMemory(&memoryArena, memory.permanentMemorySize, (u8 *)memory.permanentMemory);
        if (!memory.permanentMemory)
        {
            quitPlatformError("Memory error","Failed to allocate memory.");
        }
        
        #ifdef HOTLOAD
            appCode = win64LoadAppCode(dllFullPath, dllTempFullPath);
        #else
            appCode.updateStateAndRender = updateStateAndRender;
            appCode.initialize = initialize;
            appCode.cleanUp = cleanUp;
        #endif
    }
                    
    // NOTE: XInput init
    Input input = {0};
    {
        win64LoadXInput();                    
        input.deltaTime = win64State.frameTargetSeconds;
    }
    
    // NOTE: App init
    UserState userState = {0};
    {
        // call into the app
        if (appCode.initialize)
        {
            appCode.initialize(&userState, &memory, &input);
        }
    }             
    
    // NOTE: Audio init
    Win64SoundOutput soundOutput = {0};
    {
        #define framesOfAudioLatency 1
        soundOutput.samplesPerSecond = 48000;
        soundOutput.bytesPerSample = sizeof(i16)*2;
        soundOutput.size = soundOutput.samplesPerSecond;
        soundOutput.latencySampleCount = framesOfAudioLatency*(soundOutput.samplesPerSecond / win64State.monitorRefreshHz);
        win64InitWASAPI(&soundOutput, soundOutput.samplesPerSecond, soundOutput.size);
        soundOutput.audioClient->lpVtbl->Start(soundOutput.audioClient);
        // allocate samples buffer
        soundOutput.samples = (i16 *)VirtualAlloc(0, soundOutput.size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        //win64FillSoundBuffer(&soundOutput, 0);
    }
    
    // NOTE: Timing
    Win64Timer timer = {0};
    win64TimerInit(&win64State, &timer);
    win64TimerStart(&timer);
                    
    // NOTE: Main loop
    while (win64State.running)
    {
        #ifdef HOTLOAD
            FILETIME newDLLWriteTime = win64GetLastWriteTime(dllFullPath);
            // check if it's time to reload the DLL by comparing the write time
            if(CompareFileTime(&newDLLWriteTime, &appCode.dllLastWriteTime) != 0)
            {
                win64UnloadAppCode(&appCode);
                appCode = win64LoadAppCode(dllFullPath, dllTempFullPath);
                if (appCode.onLoad)
            {
                    memory.codeReload = true;
                }
            }
        #endif
                                                                                                                                                    
        // NOTE: Update Input
        win64ProcessInputAndMessages(&input);
        input.deltaTime = win64State.deltaTime;
        input.secondsElapsed = win64State.secondsElapsed;
        input.milliSecondsElapsed = win64State.milliSecondsElapsed;
                                    
        if (!win64State.pause)
        {            
            // NOTE: Update Audio
            SoundOutputBuffer soundBuffer = {0};
            int samplesToWrite = 0;
            UINT32 soundPaddingSize;
            if (SUCCEEDED(soundOutput.audioClient->lpVtbl->GetCurrentPadding(soundOutput.audioClient, &soundPaddingSize)))
            {
                samplesToWrite = (i32)(soundOutput.size - soundPaddingSize);
                if (samplesToWrite > soundOutput.latencySampleCount)
                {
                    samplesToWrite = soundOutput.latencySampleCount;
                }
            }
            
            soundBuffer.samplesPerSecond = soundOutput.samplesPerSecond;
            soundBuffer.sampleCount = samplesToWrite;
            soundBuffer.samples = soundOutput.samples;
                                            
            // NOTE: Update Graphics
            // copy the windows platform buffer to a buffer that is defined in the platform independent code (zize.h)
            userState.backBuffer.pixels = userState.pixels = win64BackBuffer.memory;
            userState.backBuffer.width = userState.windowWidth = win64BackBuffer.width;
            userState.backBuffer.height = userState.windowHeight = win64BackBuffer.height;
            userState.backBuffer.pitch = win64BackBuffer.pitch;
            userState.backBuffer.bytesPerPixel = win64BackBuffer.bytesPerPixel;
                                            
            userState.soundBuffer.samplesPerSecond = soundBuffer.samplesPerSecond;
            userState.soundBuffer.sampleCount = soundBuffer.sampleCount;
            userState.soundBuffer.samples = soundBuffer.samples;
                                            
            userState.memory = &memory;
            userState.rendererType = win64State.rendererType;
                                            
            #ifdef HOTLOAD
                // record input if InputRecordingIndex > 0, if we have turned recording on
                if(win64State.inputRecordingIndex)
                {
                    win64RecordInput(&win64State, &input);
                }
                                                                                                                                                                                                                                                                                                                                                                                                                                            
                // playback input if InputPlayingIndex > 0
                if(win64State.inputPlayingIndex)
                {
                    // overwrite input with the recorded input
                    win64PlayBackInput(&win64State, &input);
                }
            #endif
                                            
            if (appCode.updateStateAndRender)
            {
                appCode.updateStateAndRender(&userState, &input); 
            }
            
            // NOTE: Update Audio
            win64FillSoundBuffer(&soundOutput, &soundBuffer, samplesToWrite);
        }
                                    
                            
        // NOTE: Update window
        //HDC deviceContext = GetDC(win64State.window);
        #ifdef RENDERER_SOFTWARE_GDI
        win64UpdateWindow(win64State.deviceContext, &win64BackBuffer);
        //DwmFlush(); // to avoid stutter
        #endif
        
        #ifdef RENDERER_SOFTWARE_OGL1
            //deviceContext = GetDC(win64State.window);
            glBindTexture(GL_TEXTURE_2D, win64State.blitTextureHandle);
            // GL_BGRA is for windows DIBs, GL_RGBA8 is how OpenGL stores our image
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, win64BackBuffer.width, win64BackBuffer.height, 0,
                        GL_BGRA_EXT, GL_UNSIGNED_BYTE, win64BackBuffer.memory);
                                            
            //how GL should do when the texture is smaller or bigger when scaling
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// linear min filter
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// linear mag filter
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
                                            
            // GL_MODULATE = if we have an incoming color multiply the sampled texture with it
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                                            
            glEnable(GL_TEXTURE_2D);
                                            
            glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            //clear the modelview matrix and make a identity matrix (pretend that it dosen't exist)
            //default when OpenGL starts
            glMatrixMode(GL_MODELVIEW);
                                            
            //NOTE: now all our points will be multiplied by the ModelView matrix which are set
            //to the identity matrix which result in that our points is exactly what we passed in as
            //they are just multiplied by 1 (the identity matrix)
            glLoadIdentity();
                                            
            f32 p = 1.0f;
            glBegin(GL_TRIANGLES);
        #if 1 // Y is down
            // lower triangle
            // specify UV-coordinates for the texture
            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(-p, p);
            glTexCoord2f(1.0f, 0.0f);
            glVertex2f(p, p);
            glTexCoord2f(1.0f, 1.0f);
            glVertex2f(p, -p);
            // upper triangle
            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(-p, p);
            glTexCoord2f(1.0f, 1.0f);
            glVertex2f(p, -p);
            glTexCoord2f(0.0f, 1.0f);
            glVertex2f(-p, -p);
        #else // Y is up
            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(-p, -p);
            glTexCoord2f(1.0f, 0.0f);
            glVertex2f(p, -p);
            glTexCoord2f(1.0f, 1.0f);
            glVertex2f(p, p);
            // upper triangle
            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(-p, -p);
            glTexCoord2f(1.0f, 1.0f);
            glVertex2f(p, p);
            glTexCoord2f(0.0f, 1.0f);
            glVertex2f(-p, p);
        #endif
            glEnd();
            SwapBuffers(win64State.deviceContext);
            DwmFlush(); // to avoid stutter
        #endif // RENDERERER_SOFTWARE_OGL1
        
        #if defined(RENDERER_OGL1_2D) || defined(RENDERER_OGL1_3D)
            SwapBuffers(win64State.deviceContext);
            DwmFlush(); // to avoid stutter
        #endif
        
        #ifdef RENDERER_OGL4
            //glFlush();
            //glFinish();
            //wglSwapIntervalEXT(1); // turm on vsync
            //wglSwapLayerBuffers(deviceContext, WGL_SWAP_MAIN_PLANE); // same as SwapBuffers
            SwapBuffers(win64State.deviceContext);
            DwmFlush(); // to avoid stutter
        #endif
                            
        //ReleaseDC(win64State.window, deviceContext);
                            
        // NOTE: Update Time
        #ifdef RENDERER_SOFTWARE_GDI
            win64TimerCalculateFrameTime(&win64State, &input, &timer, win64State.lockFPS);
        #else
            win64TimerCalculateFrameTime(&win64State, &input, &timer, false);
        #endif    
    }
    
    appCode.cleanUp();
                                                                                               
    #ifdef NOCRT
        ExitProcess(0);
    #else
        // windows cleans up resources
        return 0;
    #endif
}



////////////////////////////////////
//~ console
#include <stdio.h>

i32 getAssemblyValue();
i32 getAssemblyMultiplication();

int main()
{
    // get handle to the screen for output
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(consoleHandle, 10);
    SetConsoleTitle("ZiZ");
            
    // x64 assembly test
    printf("Value from assembler function: %d\n", getAssemblyValue());
    printf("Value from mul assembler function: %d\n", getAssemblyMultiplication());
    
    #ifndef NOCRT
        return WinMain(GetModuleHandle(0), 0, GetCommandLineA(), SW_SHOWNORMAL);
    #else
        WinMainCRTStartup();
    #endif
}
