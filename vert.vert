#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 modelview;
uniform mat4 projection;

out vec4 vertexColor;
out vec3 FragPos;
out vec3 Normal;

float msin(float a) {
    return (sin(a) + 1) / 2;
}

void main() {
    vec4 pos = projection * modelview * vec4(aPos, 1.0);
    gl_Position = pos;
    float prod = aPos.y;
    float PI = 3.1415926;
    vertexColor = vec4(msin(prod), msin(prod + 2 * PI / 3), msin(prod + 4 * PI / 3), 1.0);
    Normal = aNormal;
    //FragPos = vec3(modelview * vec4(aPos, 1.0));
    FragPos = vec3(aPos.x, -aPos.y, aPos.z);
}