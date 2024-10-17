#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 modelview;
uniform mat4 projection;

out vec4 vertexColor;
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main() {
    vec4 pos = projection * modelview * vec4(aPos, 1.0);
    gl_Position = pos;
    float prod = aPos.y;
    float PI = 3.1415926;
    vertexColor = vec4(mix(vec3(1, 0.75, 0.5), vec3(0.15, 1, 0.3), vec3(clamp(aPos.y + 0.5, 0, 1))), 1.0);
    Normal = aNormal;
    TexCoord = aTexCoord;
    //FragPos = vec3(modelview * vec4(aPos, 1.0));
    FragPos = vec3(aPos.x, -aPos.y, aPos.z);
}