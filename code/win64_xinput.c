////////////////////////////////////
//~ NOTE: XInput - gamepad input

// NOTE: setup function pointers so we can load the XInput library, without using a import library
typedef DWORD WINAPI xInputGetState(DWORD dwUserIndex, XINPUT_STATE *pState);
DWORD WINAPI xInputGetStateTemp(DWORD dwUserIndex, XINPUT_STATE *pState)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
global xInputGetState *xInputGetStateProc = xInputGetStateTemp;


typedef DWORD WINAPI xInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration);
DWORD WINAPI xInputSetStateTemp(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
global xInputSetState *xInputSetStateProc = xInputSetStateTemp;

// so we can call the functions with it's old names
#define XInputGetState xInputGetStateProc
#define XInputSetState xInputSetStateProc

internal void 
win64LoadXInput(void)
{
    HMODULE xInputLibrary = LoadLibraryA("xinput1_4.dll");
    if(!xInputLibrary)
    {
        xInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    if(!xInputLibrary)
    {
        xInputLibrary = LoadLibraryA("xinput9_1_0.dll");
    }
    if(xInputLibrary)
    {
		// GetProcAddress = retrieves the adress of an exported function or variable from the specific dll
		XInputGetState = (xInputGetState *)GetProcAddress(xInputLibrary, "XInputGetState");
        if (!XInputGetState)
            XInputGetState = xInputGetStateTemp;
        
		XInputSetState = (xInputSetState *)GetProcAddress(xInputLibrary, "XInputSetState");
        if (!XInputGetState) 
            XInputSetState = xInputSetStateTemp;
	}
}

// NOTE: this has to do when you unplugg the controller, it seems to get a new index, and you have to restart windows
// is this just on my system?
// TODO: Test with new controller
internal i32
win64GetXInputGamepad(XINPUT_STATE *controllerState, i32 controllerIndex)
{
    // start on a higher count for the second controller
    for (int i = controllerIndex; i < XUSER_MAX_COUNT; i++)
        if (XInputGetState(i, controllerState) == ERROR_SUCCESS)
            return i;
    
    return -1;
}

// turn on rumble: gamepadRumbleOn(0, 10000, 10000);
void gamepadRumbleOn(i32 controller, i16 leftMotorSpeed, i16 rightMotorSpeed)
{
    XINPUT_VIBRATION vibration;
    vibration.wLeftMotorSpeed = leftMotorSpeed;
    vibration.wRightMotorSpeed = rightMotorSpeed;
    XInputSetState(controller, &vibration);
}

// turn off rumble: gamepadRumbleOff(0); 
void gamepadRumbleOff(i32 controller)
{
    XINPUT_VIBRATION vibration;
    vibration.wLeftMotorSpeed = 0;
    vibration.wRightMotorSpeed = 0;
    XInputSetState(controller, &vibration);
}
