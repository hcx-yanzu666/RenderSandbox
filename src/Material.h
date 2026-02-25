// Material.h
#pragma once
#include <glm/glm.hpp>

class Shader;
class Texture2D;

struct Material
{
    Shader*    shader = nullptr;
    Texture2D* albedo = nullptr;

    glm::vec4  color = glm::vec4(1.0f);
    float      shininess = 32.0f;
    float      ambientStrength = 0.08f;

    void Bind(const glm::vec3& viewPos) const;
};
