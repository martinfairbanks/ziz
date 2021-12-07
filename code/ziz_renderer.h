#ifndef ZIZ_RENDERER_H
#define ZIZ_RENDERER_H
/* NOTE: Set render target here

    RENDERER_SOFTWARE_GDI
        Windows GDI software rendering. Uses StretchDIBits to blit the pixel buffer to the screen.
    RENDERER_SOFTWARE_OGL1
        Software rendering that uses a texture to blit the pixel buffer to the screen.            
    RENDERER_OGL1_2D
        OpenGl 1.1 2D hardware rendering.    
    RENDERER_OGL1_3D
        OpenGl 1.1 3D hardware rendering.
    RENDERER_OGL4
        OpenGl 4.6 hardware rendering.
*/

//#define RENDERER_SOFTWARE_GDI
#define RENDERER_SOFTWARE_OGL1
//#define RENDERER_OGL1_2D
//#define RENDERER_OGL1_3D
//#define RENDERER_OGL4

#endif