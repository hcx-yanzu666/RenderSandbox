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

    // PBR 参数（pbr.frag 用；basic.frag 会忽略不认识的 uniform，不会报错）
    shader->setUniform1f("u_Metallic",  metallic);
    shader->setUniform1f("u_Roughness", roughness);
    shader->setUniform1f("u_AO",        ao);

    shader->setUniform3f("u_ViewPos",
        viewPos.x, viewPos.y, viewPos.z);
}
