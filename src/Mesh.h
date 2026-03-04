#pragma once

#include <vector>

#include <glm/glm.hpp>

struct MeshVertex
{
    glm::vec3 position{0.0f};
    glm::vec3 normal{0.0f, 1.0f, 0.0f};
    glm::vec2 uv{0.0f};
};

class Mesh
{
public:
    Mesh() = default;
    Mesh(std::vector<MeshVertex> vertices, std::vector<unsigned int> indices);
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    bool IsValid() const;
    void Draw() const;

private:
    void Setup();
    void Destroy();

    std::vector<MeshVertex> m_Vertices;
    std::vector<unsigned int> m_Indices;

    unsigned int m_VAO = 0;
    unsigned int m_VBO = 0;
    unsigned int m_EBO = 0;
};

