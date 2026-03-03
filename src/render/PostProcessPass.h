#pragma once
#include <cstdint>

class Shader;

class PostProcessPass
{
public:
    void Init(Shader* shader);
    void Shutdown();

    void Execute(std::uint32_t sceneColorTex,
                 int width,
                 int height,
                 int mode,
                 float vignetteStrength);

private:
    Shader* m_Shader = nullptr;
    std::uint32_t m_Vao = 0;
};

