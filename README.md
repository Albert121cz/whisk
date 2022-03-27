# Whisk
This is a school project. The goal of the app is the ability to load and manipulate with objects in 3D space. Objects can be loaded from Wavefront object (*.obj) files. The objects can be wrapped with textures - most of the common formats are supported.

The detailed documentation can be found in the release.

## Building
The app is built on Windows (10 and 11 tested) using MinGW (8.1.0 - standard C++17 needed) with GNU-make.

### Libraries
- wxWidgets 3.1.5
- OpenGL Mathematics (GLM) 0.9.9.8
- OpenGL Extension Wrangler (GLEW) 2.1.0

OpenGL itself is shipped with GPU drivers - to get the latest version just update them.

### Unpacking
Header files from wxWidgets should be put with to `./include/wx/`. Development files should be put to `./lib/gcc810_x64_dll` and release DLLs have to be in `./build/` loosely.

GLM's headers are expected to be in `./include/glm/` - GLM is header only library, so no other files are needed.

GLEW's header files have to be in `./include/GL/`, development files are expected to be in `./lib/glew/`; *glew32.dll* should be again put to `./build/`.

### Building in command line
The release version of the app is built with:
```
make
```

and the debug version (with terminal) with:
```
make debug
```

When switching between these 2 versions the compiled files should be deleted first with:
```
make clean
```
