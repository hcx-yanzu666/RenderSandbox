#pragma once
#ifndef RENDERSANDBOX_MODEL_H
#define RENDERSANDBOX_MODEL_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Mesh.h"

struct aiNode;
struct aiMesh;
struct aiScene;

class Model
{
public:
    Model() = default;

    //加载模型文件
    bool Load(const std::string& path);

    //绘制子Mesh
    void Draw() const;

    bool isValid() const
    {
        return !m_Meshes.empty();
    }

    glm::vec3 GetBoundsMin() const { return m_BoundsMin; }
    glm::vec3 GetBoundsMax() const { return m_BoundsMax; }
    glm::vec3 GetCenter() const { return (m_BoundsMin + m_BoundsMax) * 0.5f; }
    float GetRadius() const;

private:
    std::vector<Mesh> m_Meshes;
    void ProcessNode(aiNode* node, const aiScene* scene);
    Mesh ProcessMesh(aiMesh* mesh);

    glm::vec3 m_BoundsMin{0.0f};
    glm::vec3 m_BoundsMax{0.0f};
    bool m_HasBounds = false;
};


#endif //RENDERSANDBOX_MODEL_H
