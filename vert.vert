#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 modelview;
uniform mat4 projection;

out vec4 vertexColor;

float msin(float a) {
    return (sin(a) + 1) / 2;
}

void main() {
    vec4 pos = projection * modelview * vec4(aPos, 1.0);
    gl_Position = pos;
    float prod = aPos.y;
    float PI = 3.1415926;
    vertexColor = vec4(msin(prod), msin(prod + 2 * PI / 3), msin(prod + 4 * PI / 3), 1.0);
}