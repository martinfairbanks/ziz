typedef struct Win64Timer
{
    // LARGE_INTEGER = a union with a QuadPart field that is a signed 64bit integer
    LARGE_INTEGER countsPerSecond;
    LARGE_INTEGER frameCounter;
    b32 timerResolution;
    u64 frameCycleCounter;
    
    i64 programStartCounter;    
} Win64Timer;

internal void
win64TimerInit(Win64State *winState, Win64Timer *timer)
{
    // minimum timer resolution, in ms, for the application or device driver. lower value = more accurate resolution.
    // set the windows scheduler (all the processing threads) granularity to 1ms so that our Sleep() can be more granular.
    timer->timerResolution = (timeBeginPeriod(1) == TIMERR_NOERROR);
    
    // get the frequency of the performance counter in counts per seconds
    QueryPerformanceFrequency(&timer->countsPerSecond);
            
    #if DEVELOPER    
        // Get processor information
        GetSystemInfo(&winState->debug.systemInfo);
        GetSystemTimeAsFileTime((FILETIME *)&winState->debug.previousSystemTime);
    #endif    
}

internal void
win64TimerStart(Win64Timer *timer)
{
    // get the current number of ticks 
    QueryPerformanceCounter(&timer->frameCounter);
    timer->programStartCounter = timer->frameCounter.QuadPart;
    
    // returns the processors timestamp counter, which increments with every clock cycle since the computer started (last reset)
    timer->frameCycleCounter = __rdtsc();
}

internal inline f64 
win64GetSecondsElapsed(Win64Timer *timer, u64 start, u64 end)
{
    f64 result = ((f64)(end - start) / (f64)timer->countsPerSecond.QuadPart);
    return result;
}

internal void
win64TimerCalculateFrameTime(Win64State *winState, Input *input, Win64Timer *timer, b32 lockFPS)
{
    
    LARGE_INTEGER endCounter;
    QueryPerformanceCounter(&endCounter);
    
    // calculate how much time amd cycles elapsed this frame
    i64 counterElapsed = endCounter.QuadPart - timer->frameCounter.QuadPart;
    f32 frameSecondsElapsed = (f32)counterElapsed / (f32)timer->countsPerSecond.QuadPart;
    i64 accumulatedCounterElapsed = counterElapsed;
    
    winState->secondsElapsed = (f32)win64GetSecondsElapsed(timer, timer->programStartCounter, timer->frameCounter.QuadPart);
//    winState->secondsElapsed = ((f32)(timer->startCounter.QuadPart - timer->programStartCounter) / (f32)timer->countsPerSecond.QuadPart);
    
    winState->milliSecondsElapsed = (u64)((1000 * (endCounter.QuadPart - timer->programStartCounter)) / timer->countsPerSecond.QuadPart);
    
    // NOTE: Timing - fixed framerate
    if (lockFPS && timer->timerResolution)
    {
#if 1
        if (frameSecondsElapsed < win64State.frameTargetSeconds)
        {           
            while (frameSecondsElapsed < win64State.frameTargetSeconds)
            {
                QueryPerformanceCounter(&endCounter);
                counterElapsed = endCounter.QuadPart - timer->frameCounter.QuadPart;
                frameSecondsElapsed = (f32)counterElapsed / (f32)timer->countsPerSecond.QuadPart;
                
                // if the time is less than 75% of the target seconds, then rest
                if (frameSecondsElapsed < (win64State.frameTargetSeconds*0.75f))
                {
                    // can be from 1ms to a full system tick (15.625ms)
                    Sleep(1);
                }
            }
        }
        else
        {
            debugPrint("Missed frame rate!!!");
        }
 #elif 0
        if (frameSecondsElapsed < win64State.frameTargetSeconds)
        {
            i32 msToSleep = (i32)((win64State.frameTargetSeconds - frameSecondsElapsed) * 1000.f);
            //debugPrintVariable(i32, msToSleep);
            if (msToSleep > 0)
            {
                Sleep(msToSleep);
            }
            
            if (frameSecondsElapsed < win64State.frameTargetSeconds)
            {           
                debugPrint("Missed frame rate!!!");
            }
            
            while (frameSecondsElapsed < win64State.frameTargetSeconds) 
            {
                QueryPerformanceCounter(&endCounter);
                counterElapsed = endCounter.QuadPart - timer->frameCounter.QuadPart;
                frameSecondsElapsed = (f32)counterElapsed / (f32)timer->countsPerSecond.QuadPart;
            }
        }
#endif    
    }
    
    QueryPerformanceCounter(&endCounter);
    counterElapsed = endCounter.QuadPart - timer->frameCounter.QuadPart;
    // the endCounter is the new frameCounter
    // this ensures that we capture ALL the time spent
    timer->frameCounter = endCounter;
    
    frameSecondsElapsed = (f32)counterElapsed / (f32)timer->countsPerSecond.QuadPart;
    // delta seconds per frame in floats
    winState->deltaTime = frameSecondsElapsed;
        
#if DEVELOPER
    u64 endCycleCount = __rdtsc();
    u64 cyclesElapsed = endCycleCount - timer->frameCycleCounter;
    
    winState->debug.monitorRefreshHz = winState->monitorRefreshHz;
    // to get seconds elapsed we have to divide counterElapsed / timer->countsPerSecond.QuadPart
    // then to get milliseconds we have to multiply by 1000
    winState->debug.msPerFrame = (1000.0f*(f64)counterElapsed / (f64)timer->countsPerSecond.QuadPart);
    winState->debug.fps = (f64)timer->countsPerSecond.QuadPart / (f64)counterElapsed;
    winState->debug.fpsRaw = (f64)timer->countsPerSecond.QuadPart / (f64)accumulatedCounterElapsed;
    
    // million of cycles per frame
    winState->debug.megaCyclesPerFrame = (f64)(cyclesElapsed / (1000.0f * 1000.0f));
    // fps*megacycles / 1000 = processor speed
    winState->debug.processorSpeed = winState->debug.fps*winState->debug.megaCyclesPerFrame/1000.0f;
    
    static i32 lastTime = 0;
    if (winState->secondsElapsed - lastTime > 2)
    {        
        DWORD handleCount;
        PROCESS_MEMORY_COUNTERS_EX memInfo;
        
        debugPrint("%.03fms, %.03fFPSRaw, %.03fFPS, %.03fMEGAc/f", winState->debug.msPerFrame, winState->debug.fpsRaw,
                   winState->debug.fps, winState->debug.megaCyclesPerFrame);
        debugPrint("Seconds Elapsed: %.5f\nMilliseconds Elapsed: %lld\nDelta Time: %.05f", winState->secondsElapsed,
                   winState->milliSecondsElapsed, winState->deltaTime);
        debugPrint("Monitor refresh rate: %dHz", winState->debug.monitorRefreshHz);
        
        GetProcessHandleCount(GetCurrentProcess(), &handleCount);
        K32GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&memInfo, sizeof(memInfo));
        debugPrint("Memory: %llukB\tHandles: %lu", memInfo.PrivateUsage/1024, handleCount);
        
        // calculate CPU percentage
        GetSystemTimeAsFileTime((FILETIME *)&winState->debug.currentSystemTime);
        GetProcessTimes(GetCurrentProcess(), &winState->debug.processCreationTime, &winState->debug.processExitTime, 
                        (FILETIME*)&winState->debug.currentKernelCPUTime, (FILETIME *)&winState->debug.currentUserCPUTime);
        winState->debug.cpuPercentage = (f64)(winState->debug.currentKernelCPUTime - winState->debug.previousKernelCPUTime) +
                                             (winState->debug.currentUserCPUTime - winState->debug.previousUserCPUTime);
        winState->debug.cpuPercentage /= (winState->debug.currentSystemTime - winState->debug.previousSystemTime);
        winState->debug.cpuPercentage /= winState->debug.systemInfo.dwNumberOfProcessors;
        winState->debug.cpuPercentage *= 100;
        debugPrint("Processor clock speed: %.01f", winState->debug.processorSpeed);
        debugPrint("CPU Percentage: %.02f \tLogical Cores: %lu", winState->debug.cpuPercentage, winState->debug.systemInfo.dwNumberOfProcessors);
        
        debugPrint("Window: width=%d, height=%d", winState->windowWidth, winState->windowHeight);
        debugPrint("Backbuffer: width=%d, height=%d\n", win64BackBuffer.width, win64BackBuffer.height);
        debugPrint("Mouse: X=%d, Y=%d, Wheel delta=%d\n", input->mouseX, input->mouseY, input->mouseWheelDelta);
        lastTime = (i32)winState->secondsElapsed;
    }
    timer->frameCycleCounter = endCycleCount;    
#endif    
}
