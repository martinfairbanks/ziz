////////////////////////////////////
//~ NOTE: File I/O, only supports files up to 4gig

// TODO: using the memory arena instead of allocating new memory
// NOTE: remember to free the memory after you called this function!
File loadFile_(char *filename)
{
    File result = {0};
    
    HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER fileSize;//64-bits	
        if (GetFileSizeEx(fileHandle, &fileSize))
        {
            // ReadFile can only take a maximum of 32-bits file size (4gig file)            
            // quadpart = the whole 64-bits of the LARGE_INTEGER structure
            u32 fileSize32 = u32SafeTruncate(fileSize.QuadPart);
            
            //result.data = malloc(fileSize32); 
            //result.data = memoryPushSize(&memoryArena, fileSize32);
            result.data = VirtualAlloc(0, fileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (result.data)
            {
                DWORD bytesRead;
                // (fileSize32 == bytesRead) = if we were able to read the whole file
                if (ReadFile(fileHandle, result.data, fileSize32, &bytesRead, 0) && (fileSize32 == bytesRead))
                {
                    // NOTE: File read successfully
                    result.size = fileSize32;
                }
                else
                {
                //    free(result.data);
                    result.data = 0;
                    quitPlatformError("File I/O error", "Failed to read file!");
                }
            }
            else
            {
                quitPlatformError("File I/O error", "Failed to allocate memory for file!");
            }
        }
        else
        {
            quitPlatformError("File I/O error", "Failed to load file!");
        }
        
        CloseHandle(fileHandle);
    }
    else
    {
        quitPlatformError("File I/O error!", "Failed to load file!");
    }
    
    return result;
}

void freeFile_(File *file)
{
	if (file->data)
	{
		VirtualFree(file->data, 0, MEM_RELEASE);
        //free(file->data);
        file->data = 0;
        file->size = 0;
	}
}

// this is for the textures
void freeMemory_(u32 *memory)
{
    free(memory);
}

// TODO: allocate using the memory arena
// NOTE: remember to free the memory after you called this function!
File loadTextFile_(char *filename)
{
    File result = {0};
    
    HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER fileSize;//64-bits	
        if (GetFileSizeEx(fileHandle, &fileSize))
        {
            // ReadFile can only take a maximum of 32-bits file size (4gig file)
            u32 fileSize32 = (u32)fileSize.QuadPart;
            
            result.data = malloc(fileSize32+1); 
            //contents = memoryPushSize(&memoryArena, fileSize32);
            // VirtualAlloc(0, fileSize32, MEM_RESERVE | MEM_COMMIT, //PAGE_READWRITE);
            if (result.data)
            {
                DWORD bytesRead;
                // (fileSize32 == bytesRead) = if we were able to read the whole file
                if (ReadFile(fileHandle, result.text, fileSize32, &bytesRead, 0) && (fileSize32 == bytesRead))
                {
                    // NOTE: File read successfully
                    result.size = fileSize32;
                    // insert null terminator
                    result.text[result.size] = 0;
                }
                else
                {
                    free(result.data);
                    result.data = 0;
                    quitPlatformError("File I/O error", "Failed to read file!");
                }
            }
            else
            {
                quitPlatformError("File I/O error", "Failed to allocate memory for file!");
            }
        }
        else
        {
            quitPlatformError("File I/O error", "Failed to load file!");
        }
        
        CloseHandle(fileHandle);
    }
    else
    {
        quitPlatformError("File I/O error", "Failed to load file!");
    }
    
    return result;
}

// NOTE: This doesn't protect agianst lost data because the existing file will be overwritten
b32 writeFile(char *filename, void *memory, u32 memorySize)
{
	b32 result = false;
    
	HANDLE fileHandle = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD bytesWritten;
		if(WriteFile(fileHandle, memory, memorySize, &bytesWritten, 0))
		{
			// NOTE: File read successfully
			result = (bytesWritten == memorySize);
		}
		else
		{
			quitPlatformError("File I/O error", "Failed to write to file!");
		}	
		CloseHandle(fileHandle);
	}
	else
	{
		quitPlatformError("File I/O error", "Failed to write to file!");
	}
	return result;
}
