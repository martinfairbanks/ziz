////////////////////////////////////
//~ NOTE: OpenGL4 Renderer
//

#pragma comment(lib, "opengl32.lib")
#pragma warning(push, 3)
    #include <windows.h>
    #include <gl/gl.h>
#pragma warning(pop)
#include "../ext/include/gl/glext.h"
//#include "../ext/include/cglm/cglm.h" 

PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLVALIDATEPROGRAMPROC glValidateProgram;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM3FPROC glUniform3f;
PFNGLUNIFORM4FPROC glUniform4f;
PFNGLCLEARBUFFERFVPROC glClearBufferfv;
PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D;
PFNGLTEXIMAGE3DPROC glTexImage3D;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
PFNGLVERTEXATTRIB4FVPROC glVertexAttrib4fv;
PFNGLPATCHPARAMETERIPROC glPatchParameteri;
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

void initOpenGL4()
{
    glGenBuffers = (PFNGLGENBUFFERSPROC)state->memory->openGL4LoadFunction("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)state->memory->openGL4LoadFunction("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)state->memory->openGL4LoadFunction("glBufferData");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)state->memory->openGL4LoadFunction("glEnableVertexAttribArray");
    glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)state->memory->openGL4LoadFunction("glDisableVertexAttribArray");
    glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)state->memory->openGL4LoadFunction("glBindAttribLocation");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)state->memory->openGL4LoadFunction("glVertexAttribPointer");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)state->memory->openGL4LoadFunction("glDeleteBuffers");
    glCreateShader = (PFNGLCREATESHADERPROC)state->memory->openGL4LoadFunction("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)state->memory->openGL4LoadFunction("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)state->memory->openGL4LoadFunction("glCompileShader");
    glAttachShader = (PFNGLATTACHSHADERPROC)state->memory->openGL4LoadFunction("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)state->memory->openGL4LoadFunction("glLinkProgram");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)state->memory->openGL4LoadFunction("glCreateProgram");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)state->memory->openGL4LoadFunction("glDeleteProgram");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)state->memory->openGL4LoadFunction("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)state->memory->openGL4LoadFunction("glGetShaderInfoLog");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)state->memory->openGL4LoadFunction("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)state->memory->openGL4LoadFunction("glGetProgramInfoLog");
    glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)state->memory->openGL4LoadFunction("glValidateProgram");
    glDeleteShader = (PFNGLDELETESHADERPROC)state->memory->openGL4LoadFunction("glDeleteShader");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)state->memory->openGL4LoadFunction("glGenVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)state->memory->openGL4LoadFunction("glBindVertexArray");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)state->memory->openGL4LoadFunction("glGetUniformLocation");
    glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)state->memory->openGL4LoadFunction("glGetAttribLocation");
    glBufferSubData = (PFNGLBUFFERSUBDATAPROC)state->memory->openGL4LoadFunction("glBufferSubData");
    glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)state->memory->openGL4LoadFunction("glBlendFuncSeparate");
    glUseProgram = (PFNGLUSEPROGRAMPROC)state->memory->openGL4LoadFunction("glUseProgram");
    glUniform1i = (PFNGLUNIFORM1IPROC)state->memory->openGL4LoadFunction("glUniform1i");
    glUniform1f = (PFNGLUNIFORM1FPROC)state->memory->openGL4LoadFunction("glUniform1f");
    glUniform2f = (PFNGLUNIFORM2FPROC)state->memory->openGL4LoadFunction("glUniform2f");
    glUniform3f = (PFNGLUNIFORM3FPROC)state->memory->openGL4LoadFunction("glUniform3f");
    glUniform4f = (PFNGLUNIFORM4FPROC)state->memory->openGL4LoadFunction("glUniform4f");
    glClearBufferfv = (PFNGLCLEARBUFFERFVPROC)state->memory->openGL4LoadFunction("glClearBufferfv");
    glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC)state->memory->openGL4LoadFunction("glTexSubImage3D");
    glTexImage3D = (PFNGLTEXIMAGE3DPROC)state->memory->openGL4LoadFunction("glTexImage3D");
    glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)state->memory->openGL4LoadFunction("glGenerateMipmap");
    glVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC)state->memory->openGL4LoadFunction("glVertexAttrib4fv");
    glPatchParameteri = (PFNGLPATCHPARAMETERIPROC)state->memory->openGL4LoadFunction("glPatchParameteri");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)state->memory->openGL4LoadFunction("glUniformMatrix4fv");
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)state->memory->openGL4LoadFunction("glActiveTexture");
    
}

void clear(v4 col) 
{
    glClearColor(col.r, col.g, col.b, col.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


////////////////////////////////////
//~ NOTE: Shaders

typedef struct Shader
{
    //how many attributes to use ->0 = position, 1 = color etc.
    u32 numAttributes;
    u32 programID;
    u32 vertexShaderID;
    u32 fragmentShaderID;
} Shader;

void compileShader(char *contents, u32 id)
{
    glShaderSource(id, 1, &contents, 0);
    glCompileShader(id);
    
    i32 success = 0;
    char errorLog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, (int *)&success);
    if (!success)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        //char *errorLog = (char*)alloca(length * sizeof(char));
        
        //glGetShaderInfoLog(id, length, &length, errorLog);
        glGetShaderInfoLog(id, 512, NULL, errorLog);
        glDeleteShader(id);
        quitError("Shader error", errorLog);
    };
}

void shaderLoadFiles(Shader *shader, char *vertexShader, char *fragmentShader)
{
    shader->programID = glCreateProgram();
            
    shader->vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    if (shader->vertexShaderID == 0)
    {
        quitError("Shader error", "Failed to create vertex shader.");
    }
            
    shader->fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    if (shader->fragmentShaderID == 0)
    {
        quitError("Shader error", "Failed to create fragment shader.");
    }
            
    File vs = loadTextFile(vertexShader);
    File fs = loadTextFile(fragmentShader);
    if (vs.text)
        compileShader(vs.text, shader->vertexShaderID);
    else {
        quitError("Shader error", "Failed to load vertex shader.");
    }
    if (fs.text)
        compileShader(fs.text, shader->fragmentShaderID);
    else {
        quitError("Shader error", "Failed to load fragment shader.");
    }
            
    freeFile(&vs);
    freeFile(&fs);
}
    
// this is for creating shaders from string constants
void shaderLoad(Shader *shader, char *vertexShader, char *fragmentShader)
{
    shader->programID = glCreateProgram();
    
    shader->vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    if (shader->vertexShaderID == 0) {
        quitError("Shader error","Failed to create vertex shader.");
    }
    
    shader->fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    if (shader->fragmentShaderID == 0) {
        quitError("Shader error","Failed to create fragment shader.");
    }
    
    compileShader(vertexShader, shader->vertexShaderID);
    compileShader(fragmentShader, shader->fragmentShaderID);
}

void shaderLink(Shader *shader)
{
    //link shaders to make the shaderprogram
    glAttachShader(shader->programID, shader->vertexShaderID);
    glAttachShader(shader->programID, shader->fragmentShaderID);
    glLinkProgram(shader->programID);
    
    //TODO: FIX error handling
    
    //check for linking errors
    i32 success = 0;
    char log[512];
    glGetProgramiv(shader->programID, GL_LINK_STATUS, (int *)&success);
    if (!success)
    {
        glGetProgramInfoLog(shader->programID, 512, NULL, log);
        glDeleteProgram(shader->programID);
        glDeleteShader(shader->vertexShaderID);
        glDeleteShader(shader->fragmentShaderID);
        //printLogError("Error when linking shaders: %s\n", log);
    }
    
    glDeleteShader(shader->vertexShaderID);
    glDeleteShader(shader->fragmentShaderID);
}

void shaderCreateFromFiles(Shader *shader, char *vs, char *fs)
{
    shaderLoadFiles(shader, vs, fs);
    shaderLink(shader);
}

void shaderCreate(Shader *shader, char *vs, char *fs)
{
    shaderLoad(shader, vs, fs);
    shaderLink(shader);
}

void shaderEnable(Shader *shader)
{
    glUseProgram(shader->programID);
    // enable attributes
    for (u32 i = 0; i < shader->numAttributes; i++)
    {
        glEnableVertexAttribArray(i);
    }
}

void shaderDisable(Shader *shader)
{
    glUseProgram(0);
    for (u32 i = 0; i < shader->numAttributes; i++)
    {
        glDisableVertexAttribArray(i);
    }
}

void shaderCleanUp(Shader *shader)
{
    glDeleteProgram(shader->programID);
}


// adds a attribute to the shader - position, colors, textures etc.
void addAttribute(Shader *shader, char *attributeName)
{
    glBindAttribLocation(shader->programID, shader->numAttributes, attributeName);
    shader->numAttributes++;
}

i32 shaderGetUniformLocation(Shader *shader, char* uniformName)
{
    i32 location = glGetUniformLocation(shader->programID, uniformName);
    
    if (location == GL_INVALID_INDEX)
    {
        quitError("Shader error", "Uniform %s not found in shader!", uniformName);
    }
    return location;
}

void shaderSetUniformInt(Shader *shader, char *name, i32 value)
{
    i32 location = shaderGetUniformLocation(shader, name);
    glUniform1i(location, value);
}

void shaderSetUniformFloat(Shader *shader, char *name, f32 value)
{
    i32 location = shaderGetUniformLocation(shader, name);
    glUniform1f(location, value);
}

////////////////////////////////////
//~ NOTE: Vertex Buffers

typedef struct VertexBufferObject
{
    // OpenGL assigned buffer ID
	GLuint bufferID;
    // Buffer type (GL_ARRAY_BUFFER, GL_ELEMENT_BUFFER...)
	i32 bufferType;
    
    v3 *vertices;
	i32 sizeInBytes;
    
} VertexBufferObject;

// creates a VBO with n numner of elements
void vboCreate(VertexBufferObject *vbo, i32 elements)
{
    
	glGenBuffers(1, &vbo->bufferID);
    
    vbo->vertices = 0;//arrayInit(vbo->vertices, elements);
    vbo->sizeInBytes = 0;
}

void vboBind(VertexBufferObject *vbo)
{
    GLenum bufferType = GL_ARRAY_BUFFER;
    
	vbo->bufferType = bufferType;
	glBindBuffer(vbo->bufferType, vbo->bufferID);
}  


void vboAddData(VertexBufferObject *vbo, v3 data[], i32 numVertices)
{
    for (i32 i = 0; i < numVertices; i++)
    {
        stbArrayPush(vbo->vertices, data[i]);
    }
    
    vbo->sizeInBytes += numVertices * 3 * 4;
}

void vboUploadDataToGPU(VertexBufferObject *vbo, GLenum usageHint)
{
    //upload the vertex data to the GPU - send the data to the buffer that is bound to the GL_ARRAY_BUFFER binding point
	/*GL_STATIC_DRAW: the data will most likely not change at all or very rarely.
	GL_DYNAMIC_DRAW : the data is likely to change a lot.
	GL_STREAM_DRAW : the data will change every time it is drawn. */
    glBufferData(vbo->bufferType, vbo->sizeInBytes, vbo->vertices, usageHint);
}

void vboFree(VertexBufferObject *vbo)
{
	glDeleteBuffers(1, &vbo->bufferID);
    stbArrayFree(vbo->vertices);
}


////////////////////////////////////
//~ NOTE: Textures
#if 0
typedef struct Texture
{
    u32 id;
    i32 width;
    i32 height;
    u32 format;
    u32 loadedTextureFormat;
    u32 wrapS; // wrapping mode on s (x) axis
    u32 wrapT; // wrapping mode on t (y) axis
    u32 filterMin; // Filtering mode if texture pixels < screen pixels
    u32 filterMax; // Filtering mode if texture pixels > screen pixels
} Texture;

// Wrap_S(GL_REPEAT), Wrap_T(GL_REPEAT), Filter_Min(GL_LINEAR), Filter_Max(GL_LINEAR)

void textureGenerate(Texture *texture, u32 width, u32 height, u8 *data)
{
    glGenTextures(1, &texture->id);
    texture->width = width;
    texture->height = height;
    texture->format = GL_RGB;
    texture->loadedTextureFormat = GL_RGB;
    
    // Create Texture
    glBindTexture(GL_TEXTURE_2D, texture->id);
    //glTexImage2D(GL_TEXTURE_2D, 0, this->Internal_Format, width, height, 0, this->Image_Format, GL_UNSIGNED_BYTE, data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    // Set Texture wrap and filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->Wrap_S);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->Wrap_T);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->Filter_Min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->Filter_Max);
    
    // create mipmap, a collection of smaller images used in different distances from the object
    glGenerateMipmap(GL_TEXTURE_2D);
    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

void textureBind(Texture *texture)
{
    glBindTexture(GL_TEXTURE_2D, texture->id);
}

void textureLoadFile(Texture *texture, char *file, GLboolean alpha)
{
    // Create Texture object
    Sprite texture;
    if (alpha)
    {
        texture.Internal_Format = GL_RGBA;
        texture.Image_Format = GL_RGBA;
    }
    // Load image
    //unsigned char *image = SOIL_load_image(file, &width, &height, 0, texture.Image_Format == GL_RGBA ? SOIL_LOAD_RGBA : SOIL_LOAD_RGB);
    i32 width, height, bpp;
    unsigned char *image = stbi_load(file, &width, &height, &bpp, 0);
    if (!image)
        quitError("Could't load image file");
    
    //u8* pix = stbi_load(filename, &width, &height, &bpp, 3);
    // Now generate texture
    texture.Generate(width, height, image);
    // And finally free image data
    //SOIL_free_image_data(image);
    stbi_image_free(image);
    return texture;
}

Sprite loadImage(char *filename) 
{
    return loadTextureFromFile(filename, true);
}
#endif