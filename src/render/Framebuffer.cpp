#include "Framebuffer.h"

#include <glad/glad.h>
#include <cstdio>
#include <utility>

Framebuffer::~Framebuffer()
{
    Destroy();
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
{
    *this = std::move(other);
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
    if (this == &other) return *this;
    Destroy();

    m_Fbo = other.m_Fbo;
    m_ColorTex = other.m_ColorTex;
    m_DepthStencilRbo = other.m_DepthStencilRbo;
    m_Width = other.m_Width;
    m_Height = other.m_Height;

    other.m_Fbo = 0;
    other.m_ColorTex = 0;
    other.m_DepthStencilRbo = 0;
    other.m_Width = 0;
    other.m_Height = 0;
    return *this;
}

bool Framebuffer::Create(int width, int height)
{
    Destroy();
    if (width <= 0 || height <= 0) return false;

    glGenFramebuffers(1, &m_Fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo);

    if (!CreateAttachments(width, height)) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        Destroy();
        return false;
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::fprintf(stderr, "[Framebuffer] Incomplete FBO: 0x%x\n", (unsigned)status);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        Destroy();
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_Width = width;
    m_Height = height;
    return true;
}

bool Framebuffer::CreateAttachments(int width, int height)
{
    glGenTextures(1, &m_ColorTex);
    glBindTexture(GL_TEXTURE_2D, m_ColorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorTex, 0);

    glGenRenderbuffers(1, &m_DepthStencilRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_DepthStencilRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthStencilRbo);

    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    return true;
}

void Framebuffer::Destroy()
{
    if (m_DepthStencilRbo) {
        glDeleteRenderbuffers(1, &m_DepthStencilRbo);
        m_DepthStencilRbo = 0;
    }
    if (m_ColorTex) {
        glDeleteTextures(1, &m_ColorTex);
        m_ColorTex = 0;
    }
    if (m_Fbo) {
        glDeleteFramebuffers(1, &m_Fbo);
        m_Fbo = 0;
    }
    m_Width = 0;
    m_Height = 0;
}

bool Framebuffer::Resize(int width, int height)
{
    if (width <= 0 || height <= 0) return false;
    if (!m_Fbo) return Create(width, height);
    if (m_Width == width && m_Height == height) return true;

    glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo);

    if (m_ColorTex) glDeleteTextures(1, &m_ColorTex);
    if (m_DepthStencilRbo) glDeleteRenderbuffers(1, &m_DepthStencilRbo);
    m_ColorTex = 0;
    m_DepthStencilRbo = 0;

    if (!CreateAttachments(width, height)) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::fprintf(stderr, "[Framebuffer] Incomplete FBO after resize: 0x%x\n", (unsigned)status);
        return false;
    }

    m_Width = width;
    m_Height = height;
    return true;
}

void Framebuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo);
}

void Framebuffer::BindDefault()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

