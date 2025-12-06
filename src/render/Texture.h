#pragma once
#include "GL/glew.h"
#include <string>
#include <memory>

struct aiScene;
struct aiString;
struct aiMaterial;
struct aiTexture;

namespace gl {

    constexpr int TEXTURE_UNIT_AMBIENT  = 0;
    constexpr int TEXTURE_UNIT_DIFFUSE  = 1;
    constexpr int TEXTURE_UNIT_SPECULAR = 2;

    constexpr  int TEXTURE_FLAG_AMBIENT  = 0x1;  // Bit 0
    constexpr  int TEXTURE_FLAG_DIFFUSE  = 0x2;  // Bit 1
    constexpr  int TEXTURE_FLAG_SPECULAR = 0x4;  // Bit 2


    struct Textures {

        GLuint ambient = 0;
        GLuint diffuse = 0;
        GLuint specular = 0;
        int flags = 0;
    };

    struct DrawMaterial {

        // Color
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
        float shininess;
        float opacity;
        Textures textures;
    };

    static const DrawMaterial defaultMaterial = {
        .ambient = glm::vec3(1.f, 0.f, 1.f),
        .diffuse = glm::vec3(0.8f, 0.0f, 0.8f),
        .specular = glm::vec3(1.0f, 0.5f, 1.0f),
        .shininess = 32.0f,
        .opacity = 1.0f,
        .textures = {}
    };

    static const DrawMaterial colliderMaterial = {
        .ambient = glm::vec3(1.f, 0.3f, 0.3f),
        .diffuse = glm::vec3(1.0f, 0.3f, 0.3f),
        .specular = glm::vec3(0.0f, 0.0f, 0.0f),
        .shininess = 0.0f,
        .opacity = 0.3f,
        .textures = {}
    };

    static DrawMaterial getColliderMaterial(glm::vec3 color) {
        return {
            .ambient = color * 0.5f,
            .diffuse = color,
            .specular = glm::vec3(0.0f, 0.0f, 0.0f),
            .shininess = 0.0f,
            .opacity = 0.3f,
            .textures = {}
        };
    }

    static glm::vec3 getHue(float hue) {
        hue = std::fmod(hue, 1.0f);
        if (hue < 0.0f) hue += 1.0f;

        float h = hue * 6.0f;          // [0,6)
        int   i = static_cast<int>(std::floor(h));
        float f = h - static_cast<float>(i);

        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;

        switch (i) {
        case 0: // red -> yellow
            r = 1.0f;
            g = f;
            b = 0.0f;
            break;
        case 1: // yellow -> green
            r = 1.0f - f;
            g = 1.0f;
            b = 0.0f;
            break;
        case 2: // green -> cyan
            r = 0.0f;
            g = 1.0f;
            b = f;
            break;
        case 3: // cyan -> blue
            r = 0.0f;
            g = 1.0f - f;
            b = 1.0f;
            break;
        case 4: // blue -> magenta
            r = f;
            g = 0.0f;
            b = 1.0f;
            break;
        case 5: // magenta -> red
        default:
            r = 1.0f;
            g = 0.0f;
            b = 1.0f - f;
            break;
        }

        return {r, g, b};
    }

    static std::vector<glm::vec3> getRainbow(int number) {
        std::vector<glm::vec3> rainbow;
        for (int i=0; i<number; i++) {
            float val = float(i) / float(number);
            rainbow.emplace_back(getHue(val));
        }
        return rainbow;
    }



    class Texture {
    public:
        static std::unordered_map<std::string, DrawMaterial> loadSceneMaterials(const aiScene* scene, const std::string& directory);

    private:
        static DrawMaterial loadMaterial(const aiScene* scene, const aiMaterial* material, const std::string& directory);
        static GLint loadTexture(const aiScene* scene, const aiString* tex_string, const std::string& directory);
        static GLuint loadEmbedded(const aiTexture* texture);
        static GLuint loadFromFile(const aiString* ai_string, const std::string& directory);
        static Textures loadMaterialTextures(const aiScene* scene, const aiMaterial* material, const std::string& directory);
        static void setTextureFlags(Textures& texture);
        static std::unordered_map<std::string, GLuint> loaded_textures_;

    };
}
