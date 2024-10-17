#version 330 core
in vec4 vertexColor;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 lightPos;
uniform sampler2D smplr;

void main() {
    // lighting code from https://learnopengl.com/Lighting/Basic-Lighting,
    // except for attenuation, which is from another page on this site
    vec4 pixelColor = texture(smplr, TexCoord);
    //vec4 pixelColor = texe
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos + FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = vec3(diff);
    vec3 ambient = vec3(0.1);

    float specularStrength = 0.5;
    vec3 viewDir = lightDir;
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);

    float distance = length(lightPos + FragPos);
    float attenuation = 1.0 / (1 + 0.022 * distance + 0.0019 * distance * distance);

    vec3 result = (attenuation * (diffuse + specular + ambient) + ambient) * vertexColor.xyz * pixelColor.xyz;
    //vec3 result = vec3(pixelColor.x, TexCoord.xy);
    FragColor = vec4(result, 1.0);
}