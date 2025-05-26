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

https://github.com/user-attachments/assets/2bbe93e6-ac79-4c4f-b5f3-9b0f1aa8dc4c
https://github.com/user-attachments/assets/5e890af0-5713-4a09-ad9f-f7c2a5ba4f41
https://github.com/user-attachments/assets/78da90e0-ff73-4195-a0fc-683bfd24bed3
