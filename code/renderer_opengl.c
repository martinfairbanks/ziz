
//
//~ OpenGL1.1 Renderer
//

#pragma comment(lib, "opengl32.lib")
//#pragma comment(lib, "glu32.lib")
//#pragma comment(lib, "ext/lib/glew32.lib")
//#include <windows.h>

#pragma warning(push, 3)
    #include <windows.h>
    #include <gl/gl.h>
    //#include <GL/glu.h>
#pragma warning(pop)

void clear(v4 col) 
{
    glClearColor(col.r, col.g, col.b, col.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void loadIdentity()
{
	glLoadIdentity();
}

void translate(f32 x, f32 y, f32 z)
{
	glTranslatef(x, y, z);
}

//~ NOTE: 2D Rendering

inline void
drawPoint(f32 x, f32 y, v4 color)
{
    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
}

inline void
drawLine(f32 x0, f32 y0, f32 x1, f32 y1, v4 color)
{
    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_LINES);
    glVertex2f(x0, y0);
    glVertex2f(x1, y1);
    glEnd();    
}

inline void
drawRect(f32 x, f32 y, f32 width, f32 height, v4 color)
{
    glColor4f(color.r, color.g, color.b, color.a);
        
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

inline void
drawFilledRect(f32 x, f32 y, f32 width, f32 height, v4 color)
{
    glColor4f(color.r, color.g, color.b, color.a);
    
    //glRectf(x, y, x+width, y+height);
    
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

inline void
drawFilledRectOutlined(f32 x, f32 y, f32 width, f32 height, v4 colorFill, v4 colorOutline)
{
    drawFilledRect(x, y, width, height, colorFill);
    drawRect(x, y, width, height, colorOutline);
    
    /*glColor4f(colorOutline.r, colorOutline.g, colorOutline.b, 1.0f);
    
    glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);
    glEnd();*/
}

inline void
drawCircle(f32 x, f32 y, f32 radius, v4 color)
{
    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_LINE_STRIP);
    for (f32 angle = 0; angle < PI * 4; angle += (PI / 50.0f))
    {
        glVertex2f((f32)(x + sinf32(angle) * radius), (f32)(y + cosf32(angle) * radius));
    }
    glEnd();
}

inline void
drawFilledCircle(f32 x, f32 y, f32 radius, v4 color)
{
    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (f32 angle = 0.f; angle < TWO_PI; angle += 0.1f)
    {
        glVertex2f((f32)(x + cosf32(angle) * radius), (f32)(y + sinf32(angle) * radius));
    }
            
    // close the circle
    glVertex2f((f32)x + (f32)cosf32(TWO_PI) * (f32)radius, (f32)y + (f32)sinf32(TWO_PI) * (f32)radius);
    glEnd();
}

inline void
drawFilledCircleOutline(f32 x, f32 y, f32 radius,  v4 colorFill, v4 colorOutline)
{
    drawFilledCircle(x, y, radius, colorFill);
    drawCircle(x, y, radius, colorOutline);
}


//~ NOTE: Projections

// set orthographic projection
// objects with the same dimension appear the same size, regardless of 
// whether they are near or far from the camera
void ortho(f32 left, f32 right, f32 bottom, f32 top, f32 nearZ, f32 farZ)
{
    //ortho(0.f, zizState.windowWidth, zizState.windowHeight, 0.f, f32 nearZ = 0.0f, f32 farZ = 500.f)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    
	glOrtho(left, right, bottom, top, nearZ, farZ);
    
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    
	glDisable(GL_DEPTH_TEST);
//	renderer.projection3DFlag = false;
}

//~ NOTE: Textures

typedef union
{
    //struct {
    //v2 min, max;
    //};
    
    struct
    {
        f32 x, y, w, h;
    };
} Rect;

typedef struct Texture
{
    u32 id;
    i32 width;
    i32 height;
    b32 isValid;
} Texture;

typedef struct TextureFrame 
{
    Rect srcRect;
    Texture texture;
    
    int numFrames;
    int currentFrame;
    
} TextureFrame;


Texture loadTextureEx(char *filename, int channelCount)
{
    Texture texture = {0};
    i32 bpp;
    u8 *pix = stbi_load(filename, &texture.width, &texture.height, &bpp, channelCount);
    
    if (pix)
        texture.isValid = true;
    else
        return texture;
    
    u32 id;
    // generate 1 texture
    glGenTextures(1, &id);
    
    // bind the texture id to a texture target
    glBindTexture(GL_TEXTURE_2D, id);
    
    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //GL_LINEAR);	// linear min filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //GL_LINEAR);	// linear mag filter
    
    if (channelCount == 3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.width, texture.height, 0, GL_RGB, GL_UNSIGNED_BYTE, pix);
    else if (channelCount == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pix);
    
    stbi_image_free(pix);
    
    texture.id = id;
    
    return texture;
}

// Loads RGB texture
Texture loadTexture(char *filename)
{
    Texture tex = loadTextureEx(filename, 3);
    return tex;
}

TextureFrame loadTextureFrame(char *filename, int channelCount, Rect frame, int numFrames)
{
    TextureFrame texFrame = {0};
    texFrame.srcRect = frame;
    texFrame.texture = loadTextureEx(filename, channelCount);
    texFrame.numFrames = numFrames;
    return texFrame;
}

void drawTextureEx(Texture texture, f32 x, f32 y, f32 w, f32 h, f32 rotation, v4 color)
{
    glEnable(GL_TEXTURE_2D);
    glColor4f(color.r, color.g, color.b, color.a);
    
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(x, y, 0.0f);
    glRotatef(rotation, 0, 0, 1);
    
    glBindTexture(GL_TEXTURE_2D, texture.id);
    
    if (w == 0)
        w = (f32)texture.width;
    if (h == 0)
        h = (f32)texture.height;
    
    // place texture on quad
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(w, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(w, h);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(0.0f, h);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
    //glLoadIdentity();
    
    glPopMatrix();
    // change back the stroke color to the current selected
    //glColor4f(platformState.strokeColor.r, platformState.strokeColor.g, platformState.strokeColor.b, platformState.strokeColor.a);
}

// Draw a part of a texture (defined by a rectangle)
// NOTE: origin is relative to destination rectangle size
void drawEx(Texture texture, Rect sourceRec, Rect destRec, v2 origin, f32 rotation, v4 color, b32 flip)
{
    if (texture.id != 0)
    {
        if (sourceRec.w < 0)
            sourceRec.x -= sourceRec.w;
        if (sourceRec.h < 0)
            sourceRec.y -= sourceRec.h;
        
        // enable texturing
        glEnable(GL_TEXTURE_2D);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        
        
        glPushMatrix();
     //   glLoadIdentity();
        glTranslatef((f32)destRec.x, (f32)destRec.y, 0);
       // glRotatef(rotation, 0, 0, 1);
        //glTranslatef(-origin.x, -origin.y, 0);
        glBindTexture(GL_TEXTURE_2D, texture.id);
        
        glBegin(GL_QUADS);
        //glColor4ub((u8)tint.r, (u8)tint.g, (u8)tint.b, (u8)tint.a);
        
        // normal vector pointing towards viewer
//        glNormal3f(0.0f, 0.0f, 1.0f);
        
        if (!flip)
        {
            // bottom-left
            glTexCoord2f((f32)sourceRec.x / texture.width, (f32)sourceRec.y / texture.height);
            glVertex2f(0.0f, 0.0f);
                    
            // bottom-right
            glTexCoord2f((f32)sourceRec.x / texture.width, (f32)(sourceRec.y + sourceRec.h) / texture.height);
            glVertex2f(0.0f, (f32)destRec.h);
                    
            // top-right
            glTexCoord2f((f32)(sourceRec.x + sourceRec.w) / texture.width, (f32)(sourceRec.y + sourceRec.h) / texture.height);
            glVertex2f((f32)destRec.w, (f32)destRec.h);
                    
            // top-left
            glTexCoord2f((f32)(sourceRec.x + sourceRec.h) / texture.width, (f32)sourceRec.y / texture.height);
            glVertex2f((f32)destRec.w, 0.0f);
                    
        }
        else
        {
            
            // bottom-left
            glTexCoord2f((f32)sourceRec.x / texture.width, (f32)sourceRec.y / texture.height);
            glVertex2f((f32)destRec.w, 0.0f);
            
            // bottom-right
            glTexCoord2f((f32)sourceRec.x / texture.width, (f32)(sourceRec.y + sourceRec.h) / texture.height);
            glVertex2f((f32)destRec.w, (f32)destRec.h);
            
            // top-right
            glTexCoord2f((f32)(sourceRec.x + sourceRec.w) / texture.width, (f32)(sourceRec.y + sourceRec.h) / texture.height);
            glVertex2f(0, (f32)destRec.h);
            
            // top-left
            glTexCoord2f((f32)(sourceRec.x + sourceRec.h) / texture.width, (f32)sourceRec.y / texture.height);
            glVertex2f(0.0f, 0.0f);
        }
        
        glEnd();
        glPopMatrix();
        
        glDisable(GL_TEXTURE_2D);
    }
}


// Draw a part of a texture (defined by a rectangle)
// NOTE: origin is relative to destination rectangle size
void drawTileTexture(TextureFrame tileTexture, f32 x, f32 y, b32 flip)
{
    Rect destRect = {x, y, 120, 120};
    drawEx(tileTexture.texture, tileTexture.srcRect, destRect, v2(0.0f,0.0f), 0.0f, v4(1.0f, 1.0f, 1.0f, 1.0f), flip); 
}

void drawTextureFrame(TextureFrame tileTexture, f32 x, f32 y, i32 currentFrame, b32 flip)
{
    Rect srcRect = (Rect){
        .x = (f32)(currentFrame * tileTexture.srcRect.w),
        .y = 0,
        .w = tileTexture.srcRect.w,
        .h = tileTexture.srcRect.h
    };
    
    Rect destRect = {x, y, tileTexture.srcRect.w, tileTexture.srcRect.h};
    drawEx(tileTexture.texture, srcRect, destRect, v2(0.0f,0.0f), 0.0f, v4(1.0f, 1.0f, 1.0f, 1.0f), flip); 
}

void drawTexture(Texture texture, f32 x, f32 y, f32 width, f32 height)
{
    drawTextureEx(texture, x, y, width, height, 0.0f, v4(1.0f, 1.0f, 1.0f, 1.0f)); 
}

//~ NOTE: Fonts

void drawText(int xPos, int yPos, const char *str, ...)
{
    u32 base = 1;
    if ((base == 0) || (!str))
        return;
    
    va_list args;
    char buffer[256];
    
    va_start(args, str);
    vsprintf(buffer, str, args);
    va_end(args);
    
    glRasterPos2i(xPos, yPos);
    
    glPushAttrib(GL_LIST_BIT);
    
    // starting at character 32
    glListBase(base - 32);
    glCallLists((int)strlen(buffer), GL_UNSIGNED_BYTE, buffer);
    glPopAttrib();
}
