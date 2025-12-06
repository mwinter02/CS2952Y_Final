#pragma once
#include "Graphics.h"


struct DecompParameters;
struct aiMaterial;
struct aiScene;
struct aiTexture;

namespace gl {
    struct DrawShape;





    class Mesh {
    public:
        static DrawMesh loadStaticMesh(const char* filename);
        static DrawShape loadStaticShape(const std::vector<float> &data);
        static DrawMesh decomposeObj(const char* file_name, const DecompParameters& parameters);
        static DrawMesh loadColliderMeshObj(const char* filename);

    private:
        static bool generateObjPyScript(
            const char* obj_path,
            const char* output_path,
            const DecompParameters& parameters
        );

    };
}
