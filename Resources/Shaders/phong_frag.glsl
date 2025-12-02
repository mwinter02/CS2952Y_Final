#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

// Color
uniform vec3 ambient; // Ambient color/texture (Ka)
uniform vec3 diffuse; // Diffuse color/texture (Kd)
uniform vec3 specular; // Specular color/texture (Ks)

// Textures
uniform sampler2D texture_ambient;
uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform int texture_flags;

// Specular exponent
uniform float shininess;
uniform float opacity;

// Light properties
uniform vec3 light_position;
uniform vec3 light_color;
uniform vec3 camera_pos;

uniform vec3 ambient_light;

// Texture flags (bitwise)
const int TEXTURE_FLAG_AMBIENT  = 0x1;  // Bit 0
const int TEXTURE_FLAG_DIFFUSE  = 0x2;  // Bit 1
const int TEXTURE_FLAG_SPECULAR = 0x4;  // Bit 2


void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light_position - FragPos);
    vec3 viewDir = normalize(camera_pos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    // Determine which textures are present using bitwise operators
    bool has_ambient_tex = bool(texture_flags & TEXTURE_FLAG_AMBIENT);
    bool has_diffuse_tex = bool(texture_flags & TEXTURE_FLAG_DIFFUSE);
    bool has_specular_tex = bool(texture_flags & TEXTURE_FLAG_SPECULAR);

    // Get material colors from textures or fallback to vertex colors
    vec3 ambient = has_ambient_tex
        ? ambient * texture(texture_ambient, TexCoord).rgb
        : ambient;

    vec3 diffuse = has_diffuse_tex
        ? diffuse * texture(texture_diffuse, TexCoord).rgb
        : diffuse;

    vec3 specular = has_specular_tex
        ? specular * texture(texture_specular, TexCoord).rgb
        : specular;

    // Ambient component
    vec3 ambientResult = ambient * ambient_light;

    // Diffuse component
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuseResult = light_color * diff * diffuse;
    diffuseResult = clamp(diffuseResult,0.0 , 1.0);

    // Specular component
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specularResult = light_color * spec * specular;
    specularResult = clamp(specularResult,0.0 , 1.0);

    vec3 result = ambientResult + diffuseResult + specularResult;
    FragColor = vec4(result, opacity);
}