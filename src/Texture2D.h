#pragma once
#include <string>

// Texture2D: 封装 OpenGL 2D 纹理对象（GL_TEXTURE_2D）
// 目标：
// 1) 把“加载图片 -> 上传 GPU -> 设置采样参数”封装起来
// 2) 用 RAII 管理 GPU 资源，避免泄漏和 double-delete
// 3) 统一 Bind(slot) 接口，减少 OpenGL 状态机污染
class Texture2D
{
public:
    // path: 图片路径（jpg/png等）
    // srgb: 是否以 sRGB 内部格式存储（颜色贴图通常 true；数据贴图必须 false）
    // flipY: 是否在加载时上下翻转（解决图片坐标原点与 UV 约定差异）
    explicit Texture2D(const std::string& path, bool srgb = false, bool flipY = true);
    ~Texture2D();

    // 禁用拷贝：避免两个对象持有同一个 OpenGL texture id 导致重复释放
    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    // 允许移动：支持容器/函数返回等所有权转移
    Texture2D(Texture2D&& other) noexcept;
    Texture2D& operator=(Texture2D&& other) noexcept;

    // slot: 纹理单元编号（0,1,2...）
    // shader sampler2D 的 uniform 值就是这个 slot
    void Bind(unsigned int slot = 0) const;

    // 一般不太需要 Unbind，但保留给调试用
    void Unbind() const;

    unsigned int ID() const { return m_ID; }
    int Width() const { return m_Width; }
    int Height() const { return m_Height; }
    int Channels() const { return m_Channels; }

private:
    unsigned int m_ID = 0; // OpenGL 纹理对象句柄（glGenTextures 得到）
    int m_Width = 0;
    int m_Height = 0;
    int m_Channels = 0;
};
