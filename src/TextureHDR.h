#ifndef RENDERSANDBOX_TEXTUREHDR_H
#define RENDERSANDBOX_TEXTUREHDR_H

#pragma once
#include <cstdint>
#include <string>

class TextureHDR
{
public:
    TextureHDR() = default;
    ~TextureHDR();

    //加载 .hdr 文件 (Equirectangular 格式)
    bool Load(const std::string& path);
    void Destroy();

    std::uint32_t GetID() const {return m_TexID;}
    int GetWidth() const {return m_Width;}
    int GetHeight() const {return m_Height;}

private:
    std::uint32_t m_TexID = 0;
    int m_Width = 0;
    int m_Height = 0;
};


#endif //RENDERSANDBOX_TEXTUREHDR_H