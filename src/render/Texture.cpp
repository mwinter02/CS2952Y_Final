#include "Texture.h"


#include "stb_image.h"
#include "../Debug.h"
#include "../Util.h"
#include "assimp/material.h"
#include "assimp/scene.h"
#include "assimp/texture.h"

namespace gl {

    std::unordered_map<std::string, GLuint> Texture::loaded_textures_;

    DrawMaterial Texture::loadMaterial(const aiScene* scene, const aiMaterial* material, const std::string& directory) {
        aiColor3D ambient(0.f, 0.f, 0.f);
        aiColor3D diffuse(0.f, 0.f, 0.f);
        aiColor3D specular(0.f, 0.f, 0.f);
        float shininess = 0.0f;
        float opacity = 1.0f;

        material->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
        material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
        material->Get(AI_MATKEY_SHININESS, shininess);
        material->Get(AI_MATKEY_OPACITY, opacity);

        DrawMaterial draw_material;
        draw_material.ambient = glm::vec3(ambient.r, ambient.g, ambient.b);
        draw_material.diffuse = glm::vec3(diffuse.r, diffuse.g, diffuse.b);
        draw_material.specular = glm::vec3(specular.r, specular.g, specular.b);
        draw_material.shininess = shininess;
        draw_material.opacity = opacity;
        draw_material.textures = loadMaterialTextures(scene, material, directory);

        return draw_material;
    }

    std::unordered_map<std::string, DrawMaterial> Texture::loadSceneMaterials(const aiScene* scene,const std::string& directory) {
        std::unordered_map<std::string, DrawMaterial> materials;
        for (size_t i = 0; i < scene->mNumMaterials; i++) {
            const aiMaterial* material = scene->mMaterials[i];
            const auto material_name = material->GetName().C_Str();
            materials[material_name] = loadMaterial(scene, material, directory);
        }
        return materials;
    }

    GLint Texture::loadTexture(const aiScene* scene,const aiString* tex_string, const std::string& directory) {
        // Check if texture is embedded
        if (const auto texture = scene->GetEmbeddedTexture(tex_string->C_Str())) {
            return loadEmbedded(texture);
        }
        return loadFromFile(tex_string, directory);

    }

    GLuint Texture::loadEmbedded(const aiTexture* texture) {
        const auto tex_name = texture->mFilename.C_Str();
        if (loaded_textures_.contains(tex_name)) {
            return loaded_textures_[tex_name];
        }
        int width, height, channels;
        void* image = stbi_load_from_memory((const stbi_uc*)texture->pcData, texture->mWidth, &width, &height, &channels, 0);
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        GLenum format = GL_RGB;
        if (channels == 1) format = GL_RED;
        else if (channels == 2) format = GL_RG;
        else if (channels == 4) format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(image);

        loaded_textures_[tex_name] = textureID;
        return loaded_textures_[tex_name];
    }

    GLuint Texture::loadFromFile(const aiString* ai_string, const std::string& directory) {
        std::string tex_name = ai_string->C_Str();
        if (tex_name.empty()) {
            return 0; // GL null texture
        }
        util::fixPath(tex_name);
        auto tex_path = directory + "/" + tex_name;
        util::fixPath(tex_path);

        if (loaded_textures_.contains(tex_path)) {
            return loaded_textures_[tex_path];
        }

        const std::string full_path = util::getPath(tex_path);
        GLuint texture_id;
        int w, h, comp;
        unsigned char* image = stbi_load(full_path.c_str(), &w, &h, &comp, STBI_default);
        if (!image) {
            debug::error("Unable to load texture at: " + full_path);
            return 0; // GL null texture
        }

        debug::print("Loaded texture: " + tex_path);

        GLint format;
        if (comp == 1) format = GL_RED;
        else if (comp == 2) format = GL_RG;
        else if (comp == 3) format = GL_RGB;
        else if (comp == 4) format = GL_RGBA;
        else {
            debug::error("Unsupported texture format for: " + directory + ", format: " + std::to_string(comp));
            return 0; // GL null texture
        }

        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, image);

        glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture
        stbi_image_free(image); // Free image memory

        loaded_textures_[tex_path] = texture_id;
        return texture_id;
    }

    Textures Texture::loadMaterialTextures(const aiScene* scene, const aiMaterial* material, const std::string& directory) {
        aiString ambient_texture;
        material->GetTexture(aiTextureType_AMBIENT, 0, &ambient_texture);
        aiString diffuse_texture;
        material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuse_texture);
        aiString specular_texture;
        material->GetTexture(aiTextureType_SPECULAR, 0, &specular_texture);

        Textures textures;
        textures.ambient = loadTexture(scene, &ambient_texture, directory);
        textures.diffuse = loadTexture(scene, &diffuse_texture, directory);
        textures.specular = loadTexture(scene, &specular_texture, directory);
        setTextureFlags(textures);
        return textures;
    }

    void Texture::setTextureFlags(Textures& texture) {
        if (texture.ambient != 0) {
            texture.flags |= TEXTURE_FLAG_AMBIENT;
        }
        if (texture.diffuse != 0) {
            texture.flags |= TEXTURE_FLAG_DIFFUSE;
        }
        if (texture.specular != 0) {
            texture.flags |= TEXTURE_FLAG_SPECULAR;
        }
    }
}
