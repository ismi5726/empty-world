#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 plane;

out vec4 vertexColor;
out vec4 snowValues;
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

float mod289(float x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 mod289(vec4 x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 perm(vec4 x){return mod289(((x * 34.0) + 1.0) * x);}

float noise(vec3 p){
    vec3 a = floor(p);
    vec3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);

    vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(b.xyxy);
    vec4 k2 = perm(k1.xyxy + b.zzww);

    vec4 c = k2 + a.zzzz;
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + 1.0);

    vec4 o1 = fract(k3 * (1.0 / 41.0));
    vec4 o2 = fract(k4 * (1.0 / 41.0));

    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

    return o4.y * d.y + o4.x * (1.0 - d.y);
}

float snowValue(vec3 pos) {
    return (smoothstep(35, 60, pos.y) - pow(noise(vec3(pos.x / 4.0, pos.y - 45, pos.z / 4.0)), 2.0));
}

void main() {
    float snow = snowValue(aPos);
    vec3 snowPos = aPos + vec3(0, 1, 0) * smoothstep(0, 1, snow) * 1.5 * aNormal.y;
    vec4 pos = projection * modelview * vec4(snowPos, 1.0);
    gl_Position = pos;
    float prod = aPos.y;
    float PI = 3.1415926;
    vec3 r = (vec3(noise(aPos.xyz), noise(aPos.zxy), noise(aPos.yzx)) - vec3(0.5));
    //vec3 grassColor = mix(vec3(0.15, 0.8, 0.3) * (vec3(1) + r * vec3(0.2, 1, 0.5)), vec3(1, 1, 1), step(0, snow));
    vec3 grassColor = vec3(0.15, 0.8, 0.3) * (vec3(1) + r * vec3(0.2, 1, 0.5));
    vec3 materialColor = mix(vec3(0.76, 0.7, 0.5) * (vec3(1) + r / 8), grassColor, vec3(smoothstep(0, 1, aPos.y + 0.5)));
    materialColor = mix(vec3(0.5, 0.5, 0.5), materialColor, vec3(smoothstep(0, 1, aPos.y + 3.5)));
    vertexColor = vec4(materialColor, 1.0);
    Normal = aNormal;
    TexCoord = aTexCoord;
    //vec3 floorPos = floor(aPos);
    vec3 floorPos = aPos - vec3(0.5, 0, 0.5);
    snowValues = vec4(snowValue(floorPos), snowValue(floorPos + vec3(1, 0, 0)), snowValue(floorPos + vec3(1, 0, 1)), snowValue(floorPos + vec3(0, 0, 1)));
    //FragPos = vec3(modelview * vec4(aPos, 1.0));
    FragPos = vec3(aPos.x, -aPos.y, aPos.z);
    //gl_ClipDistance[0] = mix((aPos.y + plane.w) * plane.y, 1.0, step(0.1, abs(plane.w)));
    gl_ClipDistance[0] = mix(1, (aPos.y + plane.w) * plane.y, step(-1, abs(plane.w)));
}