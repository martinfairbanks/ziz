////////////////////////////////////
//~ NOTE: WASAPI - audio
#include <mmdeviceapi.h>
#include <audioclient.h>

typedef struct Win64SoundOutput
{
    u32 size;
    i32 channelCount;
    i32 samplesPerSecond;
    i32 bytesPerSample;
    i32 runningSampleIndex;
    i32 latencySampleCount;
    
    i16 *samples;
    i32 samplesToWrite;
    
    IAudioClient* audioClient;
    IAudioRenderClient* audioRenderClient;
    b32 soundIsValid;
} Win64SoundOutput;

enum SoundWave
{
	SQUAREWAVE,
	SINEWAVE
};

//i32 globalSoundWave = SQUAREWAVE;

static const GUID IID_IAudioClient = {0x1CB9AD4C, 0xDBFA, 0x4c32, 0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2};
static const GUID IID_IAudioRenderClient = {0xF294ACFC, 0x3146, 0x4483, 0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2};
static const GUID CLSID_MMDeviceEnumerator = {0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E};
static const GUID IID_IMMDeviceEnumerator = {0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6};
static const GUID PcmSubformatGuid = {STATIC_KSDATAFORMAT_SUBTYPE_PCM};

typedef HRESULT (*CoCreateInstanceFuncPtr)(REFCLSID rclsid, LPUNKNOWN *pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv);
HRESULT CoCreateInstanceTemp(REFCLSID rclsid, LPUNKNOWN *pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
{
    return 1;
}

typedef HRESULT (*CoInitializeExFuncPtr)(LPVOID pvReserved, DWORD dwCoInit);
HRESULT CoInitializeExTemp(LPVOID pvReserved, DWORD dwCoInit)
{
    return 1;
}

internal void 
win64WASAPIError(Win64SoundOutput *soundOutput)
{
    if (MessageBox(0, "Failed to initialize audio.\nRun without sound?", "WASAPI Error",
                   MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
    {
        soundOutput->soundIsValid = false;
    }
    else
    {
        ExitProcess(1);
    }    
}

internal void
win64InitWASAPI(Win64SoundOutput *soundOutput, i32 samplesPerSecond, i32 bufferSizeInSamples)
{
    CoCreateInstanceFuncPtr CoCreateInstanceProc = CoCreateInstanceTemp;
    CoInitializeExFuncPtr CoInitializeExProc = CoInitializeExTemp;
    
    HMODULE lib = LoadLibraryA("ole32.dll");
    if(lib)
    {
        CoCreateInstanceProc = (CoCreateInstanceFuncPtr)GetProcAddress(lib, "CoCreateInstance");
        CoInitializeExProc = (CoInitializeExFuncPtr)GetProcAddress(lib, "CoInitializeEx");
    }
    else
    {
        CoCreateInstanceProc = CoCreateInstanceTemp;
        CoInitializeExProc = CoInitializeExTemp;
        win64WASAPIError(soundOutput);
    }
    
    if (FAILED(CoInitializeExProc(0, COINIT_SPEED_OVER_MEMORY)))
    {
        win64WASAPIError(soundOutput);
    }
    
    IMMDeviceEnumerator *deviceEnum;
    if (FAILED(CoCreateInstanceProc(&CLSID_MMDeviceEnumerator, 0, CLSCTX_ALL, &IID_IMMDeviceEnumerator, (LPVOID *)(&deviceEnum))))
    {
        win64WASAPIError(soundOutput);
    }
    
    IMMDevice *device;
    if (FAILED(deviceEnum->lpVtbl->GetDefaultAudioEndpoint(deviceEnum, eRender, eConsole, &device)))
    {
        win64WASAPIError(soundOutput);
    }
    
    if (FAILED(device->lpVtbl->Activate(device, &IID_IAudioClient, CLSCTX_ALL, 0, (void **)&soundOutput->audioClient)))
    {
        win64WASAPIError(soundOutput);
    }
    
    WAVEFORMATEXTENSIBLE waveFormat;
    waveFormat.Format.cbSize = sizeof(waveFormat);
    waveFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    waveFormat.Format.wBitsPerSample = 16;
    waveFormat.Format.nChannels = 2;
    waveFormat.Format.nSamplesPerSec = (DWORD)samplesPerSecond;
    waveFormat.Format.nBlockAlign = (WORD)(waveFormat.Format.nChannels * waveFormat.Format.wBitsPerSample / 8);
    waveFormat.Format.nAvgBytesPerSec = waveFormat.Format.nSamplesPerSec * waveFormat.Format.nBlockAlign;
    waveFormat.Samples.wValidBitsPerSample = 16;
    waveFormat.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
    waveFormat.SubFormat = PcmSubformatGuid;
    
    REFERENCE_TIME bufferDuration = 10000000ULL * bufferSizeInSamples / samplesPerSecond;
    if (FAILED(soundOutput->audioClient->lpVtbl->Initialize(soundOutput->audioClient, AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_NOPERSIST,
                                                            bufferDuration, 0, &waveFormat.Format, 0)))
    {
        win64WASAPIError(soundOutput);
    }
    
    if (FAILED(soundOutput->audioClient->lpVtbl->GetService(soundOutput->audioClient,&IID_IAudioRenderClient, (void **)&soundOutput->audioRenderClient)))
    {
        win64WASAPIError(soundOutput);
    }
    
    u32 soundFrameCount;
    if (FAILED(soundOutput->audioClient->lpVtbl->GetBufferSize(soundOutput->audioClient, &soundFrameCount)))
    {
        win64WASAPIError(soundOutput);
    }
}


internal void
win64FillSoundBuffer(Win64SoundOutput *soundOutput, SoundOutputBuffer *buffer, u32 samplesToWrite)
{
    BYTE *soundBufferData;
    if (SUCCEEDED(soundOutput->audioRenderClient->lpVtbl->GetBuffer(soundOutput->audioRenderClient, samplesToWrite, &soundBufferData)))
    {
        i16 *sourceSample = buffer->samples;
        i16 *destSample = (i16*)soundBufferData;
        for(i32 sampleIndex = 0; sampleIndex < (i32)samplesToWrite; ++sampleIndex)
        {
            *destSample++ = *sourceSample++; 
            *destSample++ = *sourceSample++; 
            ++soundOutput->runningSampleIndex;
        }
        
        soundOutput->audioRenderClient->lpVtbl->ReleaseBuffer(soundOutput->audioRenderClient, samplesToWrite, 0);
    }
}
