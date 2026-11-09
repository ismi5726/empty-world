This project is an extension and addition to a number of things that I have done previously during the assignments. I ended up diverging somewhat from the proposal to focus on things that were more graphics-focused and less "game"-ey.

Here are the controls:
- WASD to move forward, left, backward, or right, respectively
- Space to jump
- Mouse to move (click to focus on the window first)
- Q to quit
- Esc to defocus the mouse, or quit if the mouse is not focused

Build instructions:
Required dependencies: opengl, glu, sdl2
To run:
make
./project

To delete:
make clean
(only tested on linux)

Features I've added:
- Procedurally generated trees, which (as far as I can tell?) use an original, recursive algorithm that seems to produce fairly reasonable results. It's also fast enough to have trees be generated and re-generated at higher detail levels within the level-of-detail generation process, but unfortunately it uses rand(), so the trees will visibly change when chunks are regenerated.

- Water, based on this article: https://medium.com/@vincehnguyen/simplest-way-to-render-pretty-water-in-opengl-7bce40cbefbe, with shader code adapted from https://github.com/moomoo02/Voxel-Engine/blob/main/Shaders/WaterShader.GLSL. It uses 2 framebuffers to render reflections and refractions, respectively, followed by some clever math to get them onto the water plane. I ended up not applying water fog because I couldn't get it to look pretty with all of the gaps in the sides of the world, unfortunately.

- Snow at higher elevations, which is applied per-fragment rather than per-vertex, is able to manipulate the actual terrain in what resembles clumping, and has what I would consider to be a fairly convincing transition from grass.

- Other land types: sand (at lower elevations as the grass goes near the water), stone (underwater)

- Infinite terrain, which additionally has a lot of height variation and should never show any gaps, even between different levels of detail (this is a big improvement from how it used to be)

- A skybox, because it gets kinda boring just looking at a completely black background color all of the time and the water needs something to reflect to look pretty

Although this project doesn't contain everything mentioned in the proposal, I believe I've done a good job of choosing things to implement that demonstrate a mastery of the course material and polishing the things that are here, rather than throwing a bunch of half-baked ideas together.

This project contains a fixed scene, as that best demonstrates the features mentioned above (snow is behind the starting area, up the mountain). It is possible to change this by changing "srand(0)" to "srand(time(NULL))" in the project-1.c file, but that's perhaps more work than you want.

Skybox taken from https://opengameart.org/content/sky-box-sunny-day (CC-BY 3.0)
