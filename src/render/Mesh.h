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



        static DrawMesh decomposeObj(const char* file_name, float quality = 0.8);
        static void coacdJsonToObj(const char* json_full_path, const char* collider_output_obj);
        static DrawMesh loadCoacdJson(const char* json_filename, const char* collider_output_obj);
        static bool generateCoacdJson(const char* obj_path, const char* json_path, float threshold);


    private:


    };
}
