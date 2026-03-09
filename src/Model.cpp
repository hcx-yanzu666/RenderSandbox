#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <cstdio>
#include <algorithm>

bool Model::Load(const std::string& path)
{
    Assimp::Importer importer;
    // aiProcess_Triangulate :	把四边形/多边形面拆成三角形
    // aiProcess_GenNormals : 如果模型没有法线，自动生成
    // aiProcess_FlipUVs : 把 UV 的 V 坐标翻转
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_FlipUVs |
        aiProcess_PreTransformVertices);

    // 检查是否加载成功
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::fprintf(stderr, "[Model] Failed to load: %s\n  Error: %s\n",
            path.c_str(), importer.GetErrorString());
        return false;
    }

    m_Meshes.clear();
    m_HasBounds = false;
    ProcessNode(scene->mRootNode, scene);
    std::fprintf(stdout, "[Model] Loaded: %s | Meshes: %zu\n", path.c_str(), m_Meshes.size());
    return true;
}

void Model::Draw() const
{
    for (const auto& mesh : m_Meshes)
    {
        mesh.Draw();
    }
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
    //处理当前节点上挂的所有mesh
    for (unsigned int i = 0; i<node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_Meshes.push_back(ProcessMesh(mesh));
    }
    //递归处理子节点
    for (unsigned int i = 0; i < node->mNumChildren; i++)
        ProcessNode(node->mChildren[i], scene);
}

Mesh Model::ProcessMesh(aiMesh* mesh)
{
    std::vector<MeshVertex> vertices;
    std::vector<unsigned int> indices;

    //遍历所有顶点
    for (unsigned int i = 0; i<mesh->mNumVertices; i++)
    {
        MeshVertex vertex;

        vertex.position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
        vertex.normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
        // UV：Assimp 支持多套 UV，只取第 0 套
        if (mesh->mTextureCoords[0])
        {
            vertex.uv = {
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            };
        }
        vertices.push_back(vertex);

        if (!m_HasBounds)
        {
            m_BoundsMin = vertex.position;
            m_BoundsMax = vertex.position;
            m_HasBounds = true;
        }
        else
        {
            m_BoundsMin.x = std::min(m_BoundsMin.x, vertex.position.x);
            m_BoundsMin.y = std::min(m_BoundsMin.y, vertex.position.y);
            m_BoundsMin.z = std::min(m_BoundsMin.z, vertex.position.z);

            m_BoundsMax.x = std::max(m_BoundsMax.x, vertex.position.x);
            m_BoundsMax.y = std::max(m_BoundsMax.y, vertex.position.y);
            m_BoundsMax.z = std::max(m_BoundsMax.z, vertex.position.z);
        }
    }
    //遍历所有面 取出索引
    for (unsigned int i = 0; i< mesh->mNumFaces; i++)
    {
        const aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }
    return Mesh(std::move(vertices), std::move(indices));
}

float Model::GetRadius() const
{
    if (!m_HasBounds) return 1.0f;
    glm::vec3 ext = (m_BoundsMax - m_BoundsMin) * 0.5f;
    return std::max(ext.x, std::max(ext.y, ext.z));
}
