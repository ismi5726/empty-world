#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 camera;
uniform vec3 light;

out vec4 fullPos;
out vec4 halfPos;
out vec3 eye;

void main() {
    vec4 pos = vec4(aPos, 1.0);
    halfPos = view * pos;
    fullPos = projection * view * pos;
    gl_Position = fullPos;
    eye = -camera - pos.xyz;
}