#include "TextureHDR.h"
#include <glad/glad.h>
#include <cstdio>

// stb_image 支持 HDR 加载
#include <stb_image.h>

TextureHDR::~TextureHDR()
{
    Destroy();
}

bool TextureHDR::Load(const std::string& path)
{
    Destroy();

    // stbi_loadf 返回 float* 数组（每像素 RGB 三个 float）
    stbi_set_flip_vertically_on_load(true);
    int w, h, channels;
    float* data = stbi_loadf(path.c_str(), &w, &h, &channels, 3); // 强制 RGB
    if (!data) {
        std::fprintf(stderr, "[TextureHDR] Failed to load: %s\n", path.c_str());
        return false;
    }

    glGenTextures(1, &m_TexID);
    glBindTexture(GL_TEXTURE_2D, m_TexID);

    // 内部格式 GL_RGB16F（16bit 浮点），数据类型 GL_FLOAT
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    m_Width = w;
    m_Height = h;
    std::printf("[TextureHDR] Loaded: %s (%dx%d)\n", path.c_str(), w, h);
    return true;
}

void TextureHDR::Destroy()
{
    if (m_TexID) {
        glDeleteTextures(1, &m_TexID);
        m_TexID = 0;
    }
    m_Width = 0;
    m_Height = 0;
}
