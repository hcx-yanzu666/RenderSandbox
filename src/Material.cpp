// Material.cpp
#include "Material.h"
#include "Shader.h"
#include "Texture2D.h"

void Material::Bind(const glm::vec3& viewPos) const
{
    shader->Bind();

    if (albedo)
    {
        albedo->Bind(0);
        shader->setUniform1i("u_Texture0", 0);
    }

    shader->setUniform4f("u_Color",
        color.r, color.g, color.b, color.a);

    shader->setUniform1f("u_Shininess", shininess);
    shader->setUniform1f("u_AmbientStrength", ambientStrength);

    shader->setUniform3f("u_ViewPos",
        viewPos.x, viewPos.y, viewPos.z);
}
