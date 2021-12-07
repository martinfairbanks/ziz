# ZiZ Framework
A creative framework/game engine written in C.
Supports hot-reload and input playback.


## Quick Start

Install [Visual Studio Community 2022](https://visualstudio.microsoft.com/).
Run init_vs.bat to setup the Visual Studio build tools.
Note: Change init_vs.bat if you have a different version of VS or if you installed it in a different location.

### Build the sketch

```console
$ build.bat
```

### Build all examples
```console
$ build_all.bat
```

### Hotload
- Build and run your app
- Edit source code and run build.bat again to see the changes.


## Dependencies
Windows and Visual Studio compiler.

## Notes on the code
All files prefixed with win64_ is win64 platform only! The rest of the code is platform independent.
Functions prefixed with win64 is internal to the windows platform layer. 

The hot reload and input playback I got from Casey Muratori from his Handmade Hero series.

### Compilation

	To get a console window, compile with subsystem console.

	Build switches:
		-DNOCRT
 			For no c runtime library.
        -DDEVELOPER=1
			Debug code allowed.

### Renderers
Currently supported graphics APIs are:

- Windows GDI - softwarte rendering
- OpenGL 1.1 - software and hardware rendering
- OpenGL 4.6 - hardware rendering

You can specify the render terget in the file ziz_renderer.h using the following defines:
```
RENDERER_SOFTWARE_GDI
RENDERER_OGL1
RENDERER_OGL4
```

## Controls
- `esc` - terminate
- `alt+f4` - terminate
- `f1` - start input recording/playback
- `f4` - pause
- `alt+return` - toggle fullscreen

## **Framework API**

## Vectors

### Vector initializiation
```
v2i vec = v2i(1, 1);
v2 vec = { 1.f, 1.f };
v2 vec = v2(2.f, 2.f);
v3 vec = v3(2.f, 2.f, 2.f);
v4 vec = v4(2.f, 2.f, 2.f,, 2.f);
v2 vec{ 1.f, 1.f };

// swizzle
vec2 = vec3.xy;
vec2 = vec3.yz;
    
```