#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 modelview;
uniform mat4 projection;

out vec3 TexCoord;

void main() {
    vec4 pos = projection * modelview * vec4(aPos, 1.0);
    pos.z = pos.w;
    gl_Position = pos;
    TexCoord = aPos;
    TexCoord.y = -TexCoord.y;
}