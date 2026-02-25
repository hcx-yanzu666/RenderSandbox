#pragma once
#include <cstdint>

class Framebuffer
{
public:
    Framebuffer() = default;
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    Framebuffer(Framebuffer&& other) noexcept;
    Framebuffer& operator=(Framebuffer&& other) noexcept;

    bool Create(int width, int height);
    void Destroy();
    bool Resize(int width, int height);

    void Bind() const;
    static void BindDefault();

    int Width() const { return m_Width; }
    int Height() const { return m_Height; }

    std::uint32_t Fbo() const { return m_Fbo; }
    std::uint32_t ColorTex() const { return m_ColorTex; }

private:
    bool CreateAttachments(int width, int height);

    std::uint32_t m_Fbo = 0;
    std::uint32_t m_ColorTex = 0;
    std::uint32_t m_DepthStencilRbo = 0;
    int m_Width = 0;
    int m_Height = 0;
};
