#include "Mesh.h"

#include <cstddef>
#include <utility>

#include <glad/glad.h>

Mesh::Mesh(std::vector<MeshVertex> vertices, std::vector<unsigned int> indices)
    : m_Vertices(std::move(vertices)),
      m_Indices(std::move(indices))
{
    Setup();
}

Mesh::~Mesh()
{
    Destroy();
}

Mesh::Mesh(Mesh&& other) noexcept
{
    *this = std::move(other);
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if (this == &other) return *this;
    Destroy();

    m_Vertices = std::move(other.m_Vertices);
    m_Indices = std::move(other.m_Indices);
    m_VAO = other.m_VAO;
    m_VBO = other.m_VBO;
    m_EBO = other.m_EBO;

    other.m_VAO = 0;
    other.m_VBO = 0;
    other.m_EBO = 0;
    return *this;
}

bool Mesh::IsValid() const
{
    return m_VAO != 0 && !m_Indices.empty();
}

void Mesh::Setup()
{
    if (m_Vertices.empty() || m_Indices.empty()) return;

    // 1) 创建 VAO/VBO/EBO
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    // 2) 绑定 VAO，后续配置都记录到它
    glBindVertexArray(m_VAO);

    // 3) 上传顶点数据
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_Vertices.size() * sizeof(MeshVertex)),
                 m_Vertices.data(),
                 GL_STATIC_DRAW);

    // 4) 上传索引数据
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_Indices.size() * sizeof(unsigned int)),
                 m_Indices.data(),
                 GL_STATIC_DRAW);

    // 5) 顶点布局：location 0/1/2 -> position/normal/uv
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, uv));
    glEnableVertexAttribArray(2);

    // 6) 解绑 VAO，防止后续状态污染
    glBindVertexArray(0);
}

void Mesh::Destroy()
{
    if (m_EBO) {
        glDeleteBuffers(1, &m_EBO);
        m_EBO = 0;
    }
    if (m_VBO) {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }
    if (m_VAO) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
}

void Mesh::Draw() const
{
    if (!IsValid()) return;

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_Indices.size()), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

