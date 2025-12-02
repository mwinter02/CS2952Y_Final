#pragma once

// Material constants - keep in sync with material_constants.glsl

namespace material {
    // Texture flags (bitwise)
    constexpr int TEXTURE_FLAG_AMBIENT  = 0x1;  // Bit 0
    constexpr int TEXTURE_FLAG_DIFFUSE  = 0x2;  // Bit 1
    constexpr int TEXTURE_FLAG_SPECULAR = 0x4;  // Bit 2

    // Texture units
    constexpr int TEXTURE_UNIT_AMBIENT  = 0;
    constexpr int TEXTURE_UNIT_DIFFUSE  = 1;
    constexpr int TEXTURE_UNIT_SPECULAR = 2;
}

