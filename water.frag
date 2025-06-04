#version 330 core
// https://medium.com/@vincehnguyen/simplest-way-to-render-pretty-water-in-opengl-7bce40cbefbe
// https://github.com/moomoo02/Voxel-Engine/blob/main/Shaders/WaterShader.GLSL
in vec4 fullPos;
in vec4 halfPos;
in vec3 eye;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform float time;

out vec4 FragColor;

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

void main() {
    vec2 modifiedCoords = (fullPos.xy / fullPos.w) / 2.0 + 0.5;

    vec2 reflectCoords = vec2(modifiedCoords.x, -modifiedCoords.y);
    vec2 refractCoords = vec2(modifiedCoords.x, modifiedCoords.y);

    vec2 distortion = vec2(noise(halfPos.zxy / 3 + vec3(time / 300.0, 0, 0)), noise(halfPos.yzx)) * 0.01;

    reflectCoords += distortion;
    reflectCoords.x = clamp(reflectCoords.x, 0.001, 0.999);
    reflectCoords.y = clamp(reflectCoords.y, -0.999, -0.001);

    refractCoords += distortion;
    refractCoords = clamp(refractCoords, 0.001, 0.999);

    vec4 reflectColor = texture(reflectionTexture, reflectCoords);
    vec4 refractColor = texture(refractionTexture, refractCoords);

    vec3 viewVector = normalize(eye);
    float refractFactor = pow(dot(viewVector, vec3(0.0, 1.0, 0.0)), 2.0);

    FragColor = mix(reflectColor, refractColor, refractFactor);
    FragColor = mix(FragColor, vec4(0.0, 0.03, 0.5, 1.0), 0.1);
}