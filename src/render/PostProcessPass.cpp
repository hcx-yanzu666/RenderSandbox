#include "PostProcessPass.h"

#include "../Shader.h"

#include <glad/glad.h>

void PostProcessPass::Init(Shader* shader)
{
    m_Shader = shader;
    if (!m_Vao) {
        glGenVertexArrays(1, &m_Vao);
    }
}

void PostProcessPass::Shutdown()
{
    if (m_Vao) {
        glDeleteVertexArrays(1, &m_Vao);
        m_Vao = 0;
    }
    m_Shader = nullptr;
}

void PostProcessPass::Execute(std::uint32_t sceneColorTex,
                              int width,
                              int height,
                              int mode,
                              float vignetteStrength)
{
    if (!m_Shader || !m_Shader->GetRendererID()) return;
    if (width <= 0 || height <= 0) return;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    glDisable(GL_DEPTH_TEST);

    m_Shader->Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneColorTex);
    m_Shader->setUniform1i("u_SceneTex", 0);
    m_Shader->setUniform1i("u_Mode", mode);
    m_Shader->setUniform1f("u_VignetteStrength", vignetteStrength);

    glBindVertexArray(m_Vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

