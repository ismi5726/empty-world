#version 330 core
in vec4 vertexColor;
in vec4 snowValues;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightPos;

void main() {
    // lighting code from https://learnopengl.com/Lighting/Basic-Lighting,
    // except for attenuation, which is from another page on this site
    vec4 color = vertexColor;
    float rx = fract(FragPos.x);
    /*float snow = mix(
        mix(snowValues.x, snowValues.y, rx),
        mix(snowValues.w, snowValues.z, rx),
        fract(FragPos.z)
    );*/
    float snow = snowValues.x;
    //vec4 groundColor = color;
    vec4 groundColor = vec4(1.0);
    vec4 snowMixedColor = mix(vertexColor, vec4(1.0), step(0, snow));
    //vec4 snowMixedColor = vec4(1.0);
    //vec4 groundColor = mix(dirtColor, snowColor, smoothstep(0.7, 0.9, vertexColor.b));
    //vec4 groundColor = texe
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos + FragPos);
    float specularFactor = 1.15 - vertexColor.r;

    float diff = max(dot(norm, lightDir), 0.0) * specularFactor;
    vec3 diffuse = vec3(diff);
    vec3 ambient = vec3(0.4);

    float specularStrength = specularFactor / 2;
    vec3 viewDir = lightDir;
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2);
    vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);

    float distance = length(lightPos + FragPos);
    float attenuation = 1.0 / (1 + 0.022 * distance + 0.0019 * distance * distance);

    vec3 result = (attenuation * (diffuse + specular + ambient) + ambient) * snowMixedColor.xyz * groundColor.xyz;
    //vec3 result = vec3(groundColor.x, TexCoord.xy);
    FragColor = vec4(result, 1.0);
    //FragColor = color;
}