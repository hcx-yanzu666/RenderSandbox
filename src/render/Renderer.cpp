#include "Renderer.h"

#include "../Material.h"
#include "../Shader.h"

#include <glad/glad.h>
#include <algorithm>

void Renderer::SetPointLights(const std::vector<PointLight>& lights)
{
    m_PointLights = lights;
}

void Renderer::BeginFrame(const glm::mat4& view,
                          const glm::mat4& proj,
                          const glm::vec3& viewPos)
{
    m_View = view;
    m_Proj = proj;
    m_ViewPos = viewPos;
}

void Renderer::DrawObject(const Object& obj, unsigned int vao, int vertexCount)
{
    if (!obj.material || !obj.material->shader) return;

    // 1) 绑定材质（内部会 Bind shader / texture，并写入材质参数）
    obj.material->Bind(m_ViewPos);

    Shader* shader = obj.material->shader;

    // 2) 每帧/每物体 uniform（矩阵）
    glm::mat4 modelMat = obj.transform.ToMatrix();
    shader->SetMatrices(modelMat, m_View, m_Proj);

    // 3) 灯光（先按你现有 shader 的写法：PointLights 数组）
    const int MAX_POINT_LIGHTS = 8;
    int count = (int)std::min<size_t>(m_PointLights.size(), MAX_POINT_LIGHTS);

    shader->setUniform1i("u_PointLightCount", count);

    for (int i = 0; i < count; ++i)
    {
        const auto& L = m_PointLights[i];

        // u_PointLights[i].position / color
        // 注意：你 Shader 类现在是 setUniform3f(string, x,y,z) ——你要确保它存在
        shader->setUniform3f(("u_PointLights[" + std::to_string(i) + "].position").c_str(),
                             L.position.x, L.position.y, L.position.z);

        shader->setUniform3f(("u_PointLights[" + std::to_string(i) + "].color").c_str(),
                             L.color.x, L.color.y, L.color.z);
    }

    // 4) draw
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}
