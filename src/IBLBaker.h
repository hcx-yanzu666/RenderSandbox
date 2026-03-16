#pragma once
#include <cstdint>

class IBLBaker
{
public:
    // 程序启动时调用一次，传入 HDR 纹理 ID
    void Bake(uint32_t hdrTexID);
    void Destroy();

    uint32_t GetEnvCubemap()    const { return m_EnvCubemap; }
    uint32_t GetIrradianceMap() const { return m_IrradianceMap; }
    uint32_t GetPrefilterMap()  const { return m_PrefilterMap; }
    uint32_t GetBRDFLUT()       const { return m_BrdfLUT; }

    void RenderCube();   // 渲染单位立方体（6面共用）


private:
    //输入：HDR 全景图（2D 纹理）
    //输出：m_EnvCubemap（Cubemap，512×512×6）
    //作用：把球面全景图转成立方体贴图，方便用方向向量采样
    void BakeCubemap(uint32_t hdrTexID);
    //输入：m_EnvCubemap
    //输出：m_IrradianceMap（Cubemap，32×32×6）
    //作用：把环境图模糊成"各方向的平均光照"，给漫反射用
    void BakeIrradiance();
    //输入：m_EnvCubemap
    //输出：m_PrefilterMap（Cubemap，128×128×6，5 级 mipmap）
    //作用：按不同 roughness 分级模糊，给镜面反射用
    void BakePrefilter();
    //输入：无（纯数学计算）
    //输出：m_BrdfLUT（2D 纹理，512×512）
    //作用：预计算 Fresnel 和 roughness 对镜面反射的影响
    void BakeBRDFLUT();

    void RenderQuad();   // 渲染全屏四边形（BRDF LUT 用）

    uint32_t m_EnvCubemap    = 0;
    uint32_t m_IrradianceMap = 0;
    uint32_t m_PrefilterMap  = 0;
    uint32_t m_BrdfLUT       = 0;

    // 渲染用的临时几何体
    uint32_t m_CubeVAO = 0, m_CubeVBO = 0;
    uint32_t m_QuadVAO = 0, m_QuadVBO = 0;

    // 离屏渲染用的 FBO
    uint32_t m_CaptureFBO = 0, m_CaptureRBO = 0;
};
