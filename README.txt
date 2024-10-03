This program depicts 2 simple scenes using OpenGL.
The first is a group of metaballs displayed using marching cubes, and the second is a collection of dodecahedrons.

The controls for this program are quite simple:
- click and drag to rotate the scene
- right click and drag to pan the scene
- scroll to zoom in / out
- m to change between the scenes

Logic for the marching cubes algorithm and most of the shader setup code is from the internet; everything should be attributed shortly above in comments. A few things (some setup functions and quat.c/quat.h) are reused from the previous homework. Everything else is original. Dodecahedron faces painstakingly determined by yours truly (although the points themselves are from wikipedia).

Total time: ~10 hours (most of which was bugfixing unfortunately)

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

New to the resubmission:

This resubmission adds different views to the program, since I missed that the first time around. These views are:

- ortho (default from before)
- perspective
- walking (outside)
- walking (inside)

The control to change views is 'p'. When walking, you can use 'wasd' to move around, space to jump, left click and drag to move the camera (note that this behaves like the default minecraft camera, except that you have to drag because glutWarpMouse doesn't work on my machine), and (while walking outside) 'm' to change scene, right click and drag to turn the scene, and middle click and drag to move the scene.

Walking (outside) uses a very simple physics model, so you can clip through all of the objects, while walking (inside) uses some very simple collision detection with the metaballs' influence field. As such, you can get stuck while walking (inside), and if that happens, you'll need to press '0' to reset the scene. This is also the best course of action if the object(s) get lost in any of the other scenes. Finally, this resubmission adds back 'q'/esc to exit.

All of the movement / turning for the walking mode is done to the camera position, rather than to the model, as mentioned in the comment asking me to resubmit. Dragging the scene is slightly less accurate in perspective views than in ortho views, but not by a lot (this is exacerbated if you're moving, though). Due to the way movement was implemented, only one directional button is recognized at a time.