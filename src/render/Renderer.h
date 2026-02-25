#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "Light.h"
#include "../Object.h"

class Renderer
{
public:
    void SetPointLights(const std::vector<PointLight>& lights);

    void BeginFrame(const glm::mat4& view,
                    const glm::mat4& proj,
                    const glm::vec3& viewPos);

    void DrawObject(const Object& obj, unsigned int vao, int vertexCount);

private:
    std::vector<PointLight> m_PointLights;

    // per-frame cache
    glm::mat4 m_View{1.0f};
    glm::mat4 m_Proj{1.0f};
    glm::vec3 m_ViewPos{0.0f};
};
