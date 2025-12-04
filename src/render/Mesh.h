#pragma once
#include "Graphics.h"



struct aiMaterial;
struct aiScene;
struct aiTexture;

namespace gl {
    struct DrawShape;





    class Mesh {
    public:
        static DrawMesh loadStaticMesh(const char* filename);
        static DrawShape loadStaticShape(const std::vector<float> &data);
        static DrawMesh decomposeObj(const char* file_name, float quality = 0.8);

    private:
        static bool generateObjPyScript(const char* obj_path, const char* output_path, float threshold);

    };
}
