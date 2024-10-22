#version 330 core

in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 eye;
uniform vec3 color;

out vec4 FragColor;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos-FragPos);
    float diffuse = max(dot(norm,lightDir),0.0);

    vec3 viewDir = normalize(eye-FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float specular = pow(max(0.0, dot(norm,reflectDir)),32);

    FragColor = vec4(diffuse * color + specular*color, 1.0f);

} 