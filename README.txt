This program is an extension of the previous ones, now with textures! This time, I tried to play around with how to get texture coordinates for different shapes, with the terrain being fairly simply (just plastering the texture on every marched cell, but in a way that's coherent even with large vertical changes), but the dodecahedra being slightly more complicated. The default mode is a sort of naive attempt to force a square texture onto a pentagon, which sort of works, but is missing close to half of the texture, while the second mode changes the way the dodecahedra are rendered to have 5 triangles instead of 3, which allows me to also put the texture on there slightly better. Unfortunately, neither of these modes do a great job of preserving the full texture or visual accuracy with angles, so I've also included a mode that just clips the texture to the pentagon, but especially for something like the crate texture (taken from ex14), where the edges are very important, this isn't ideal, either (although it'd be fine if I was using textures meant for pentagons).

This version also features some other little fun things like landscape made from perlin noise instead of just a sine wave (original sources for the perlin noise listed at the top of perlin.c), which involved significantly rewriting my movement code due to it apparently being upside down and inverted (as in you were somehow walking on the underside of the function - I don't really get it either), and grassy / dirt terrain as a sort of callback to hw2.

Full controls list:
- scroll to zoom in / out
- p to change between the perspectives (ortho, perspective, walking)
- m to change dodecahedron texture mode
- t to change dodecahedron texture
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