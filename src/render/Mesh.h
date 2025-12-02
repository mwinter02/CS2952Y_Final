#pragma once
#include "Graphics.h"


struct aiMaterial;
struct aiScene;
struct aiTexture;

namespace gl {
    struct DrawShape;





    class Mesh {
    public:


        static DrawShape loadStaticShape(const std::vector<float> &data);
        static DrawMesh loadStaticMesh(const char* filename);

        static unsigned int getImportPreset();

    private:


        static void setTextureFlags(Textures& texture);
    };
}
