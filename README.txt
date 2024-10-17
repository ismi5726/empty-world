This program is an extension of the previous one, adding lighting to the scene. Controls are as follows:

Full controls list:
- scroll to zoom in / out
- p to change between the perspectives (ortho, perspective, walking)
- o to pause the rotation of the lighting
- 0 to reset scene
- q / esc to exit
normal controls:
- click and drag to rotate the camera around the viewing point
walking controls:
- wasd to move
- space to jump
- click and drag to turn camera

Time taken: 7 hours.

~~~~~ RESUBMISSION NOTES ~~~~~

I think in the original version, the light should've moved, but perhaps I made it too subtle (through things like moving its position between objects so that it had a consistent global position and placing it right near the middle, but not drawing anything to indicate its position), making it very hard to notice and thus ineffective at accomplishing its goal. In an effort to fix this, I've moved it significantly further out (I kept the global positioning, but now it has a lot more of an effect on all of the dodecahedra) and added another dodecahedron to indicate its position. It just spins around the scene, which isn't super interesting, but I feel like that gets the job done.

For something more interesting, you can look at the walking perspective (press 'p' twice from starting the program), which had a light that moved with the player before, but I've added attenuation to make it more apparent that things are going on. Unfortunately, I realized during the course of testing everything that it's hard to see the specular highlights on shapes with so few triangles in the dodecahedrons and the OpenGL built-in lighting model being per-vertex / per-triangle, rather than per-fragment, so I found a tutorial for the Phong lighting model online (linked in frag.frag) and used it to add really nice specular highlights to the terrain. Honestly, considering that the terrain and the dodecahedra are drawn with completely different systems (especially now that the terrain actually uses buffers to make normals work), it's impressive how adequately they fit together stylistically (just pretend that the dodecahedra are really shiny!).

None of the controls have changed.