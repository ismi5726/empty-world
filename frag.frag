#version 330 core
in vec4 vertexColor;
in vec4 snowValues;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in float dist;

out vec4 FragColor;

uniform vec3 lightPos;
uniform sampler2D groundSampler;
uniform sampler2D snowSampler;

void main() {
    // lighting code from https://learnopengl.com/Lighting/Basic-Lighting,
    // except for attenuation, which is from another page on this site
    vec4 dirtColor = texture(groundSampler, TexCoord);
    vec4 snowColor = texture(snowSampler, TexCoord);
    float rx = fract(FragPos.x);
    float snow = snowValues.x;
    vec4 groundColor = mix(dirtColor, snowColor, step(0, snow));
    vec4 snowMixedColor = mix(vertexColor, vec4(1.0), step(0, snow));
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos + FragPos);
    float specularFactor = 1.15 - vertexColor.r;

    float diff = max(dot(norm, lightDir), 0.0) * specularFactor;
    vec3 diffuse = vec3(diff);
    vec3 ambient = vec3(0.4);

    float specularStrength = specularFactor / 2;
    vec3 viewDir = lightDir;
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);

    float distance = length(lightPos + FragPos);
    float attenuation = 1.0 / (1 + 0.022 * distance + 0.0019 * distance * distance);

    vec4 fogColor = vec4(mix(vec4(0, 0.03, 0.5, 1.0), vec4(1.0, 1.0, 1.0, 1.0), step(0, lightPos.y)));
    float fogStart = mix(10, 240, step(0, lightPos.y));
    float f = clamp((320 - dist) / (320 - fogStart), 0.0, 1.0);

    vec3 result = (attenuation * (diffuse + specular + ambient) + ambient) * snowMixedColor.xyz * groundColor.xyz;
    FragColor = vec4(result, 1.0);
}