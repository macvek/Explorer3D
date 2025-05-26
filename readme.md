# Explorer3d

This app is a walker in 3d space. Loads sample texture, renders some fixed font text, has 3 different move types : freespace, xyz, hybrid. 

Features:
* 3 sides + camera view
* Selecting object in space based translating 2d space to a traced line into 3d space. It allows to have a 3d cursor
* Very basic opengl 1.1 support with no shaders, the main idea was to figure out general coordinate system along with FOV, perspective/orthographic rendering and making a 3d trace line for selecting

## Building
Use cmake, create directory `build` and inside it:
* `cmake ../`
* Open solution in `visual studio community` and run it.
* I failed to get it running on Linux using Mesa3D, yet I did not want to debug it, portability is not the point of this project.



