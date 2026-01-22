#include "Texture2D.h"
#include <glad/glad.h>
#include <cstdio>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// channels -> OpenGL format（源数据格式）
// format 表示你传进 glTexImage2D 的 data 是怎么排列的：RGB / RGBA / RED ...
static GLenum ChannelsToFormat(int channels)
{
    switch (channels) {
        case 1: return GL_RED;
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
        default: return GL_RGB; // 兜底
    }
}

Texture2D::Texture2D(const std::string& path, bool srgb, bool flipY)
{
    // 1) 设置 stb_image 是否翻转
    // OpenGL 的 UV 约定通常认为 v=0 在底部；很多图片数据第一行是“顶部”
    stbi_set_flip_vertically_on_load(flipY ? 1 : 0);

    // 2) CPU 端读取图片数据
    unsigned char* data = stbi_load(path.c_str(), &m_Width, &m_Height, &m_Channels, 0);
    if (!data) {
        std::fprintf(stderr, "[Texture2D] Failed to load: %s\n", path.c_str());
        return;
    }

    // 3) 源数据格式（format）取决于通道数
    GLenum format = ChannelsToFormat(m_Channels);

    // 4) GPU 内部存储格式（internalFormat）
    // - internalFormat 决定 GPU 用什么格式存这张纹理
    // - format 决定你提供给 OpenGL 的 data 是 RGB 还是 RGBA
    //
    // 颜色贴图建议用 sRGB（例如 albedo/diffuse），这样采样时能自动做 gamma->linear 转换（配合 GL_FRAMEBUFFER_SRGB）
    // 数据贴图（normal/metallic/roughness/AO）必须是线性，不要 srgb
    GLenum internalFormat = format;
    if (srgb) {
        if (format == GL_RGB)  internalFormat = GL_SRGB;
        if (format == GL_RGBA) internalFormat = GL_SRGB_ALPHA;
    }

    // 5) 创建并绑定 OpenGL 纹理对象

    //类似于QPen 创建了一个pen 然后glBindTexture setpen 后续绘制或者操作都基于这个pen 后面的函数都是操作pen的属性 opengl是操作纹理对象的属性
    //拿到一个纹理对象
    glGenTextures(1, &m_ID);//生成(分配)一个纹理对象名字/句柄
    //把他设置成当前操作的纹理对象
    //把 id 对应的纹理对象绑定到 GL_TEXTURE_2D target（在当前 active unit 上），这样后续 glTexParameteri/glTexImage2D 操作的就是它。
    glBindTexture(GL_TEXTURE_2D, m_ID);//把这个纹理对象绑定到 GL_TEXTURE_2D 目标，设为当前纹理

    // 6) 采样参数（wrap/filter）
    // wrap: UV 超出 [0,1] 怎么办
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // filter:
    // - MIN: 纹理被缩小（远处）时用什么过滤（通常配 mipmap）
    // - MAG: 纹理被放大（近处）时用什么过滤（mipmap 对放大无意义）
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 7) 上传像素到 GPU
    // 注意：glTexImage2D 会把 data 拷贝进 GPU，所以之后可以 free CPU 数据
    glTexImage2D(GL_TEXTURE_2D,
                 0,                // mip level 0
                 internalFormat,    // GPU 内部存储格式
                 m_Width, m_Height,
                 0,                // border 必须为 0
                 format,            // 源数据格式（RGB/RGBA）
                 GL_UNSIGNED_BYTE,  // 源数据类型（8bit/通道）
                 data);

    // 8) 生成 mipmap（否则远处会闪烁/摩尔纹）
    glGenerateMipmap(GL_TEXTURE_2D);

    // 9) 解绑 + 释放 CPU 数据
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
}

Texture2D::~Texture2D()
{
    // RAII：对象销毁时释放 GPU 资源
    if (m_ID) glDeleteTextures(1, &m_ID);
}

Texture2D::Texture2D(Texture2D&& other) noexcept
{
    // 移动：偷走对方资源句柄
    m_ID = other.m_ID;
    m_Width = other.m_Width;
    m_Height = other.m_Height;
    m_Channels = other.m_Channels;

    // 对方置空，避免析构时重复 delete
    other.m_ID = 0;
    other.m_Width = other.m_Height = other.m_Channels = 0;
}

Texture2D& Texture2D::operator=(Texture2D&& other) noexcept
{
    if (this == &other) return *this;

    // 先释放自己原来的资源，避免泄漏
    if (m_ID) glDeleteTextures(1, &m_ID);

    // 再接管对方资源
    m_ID = other.m_ID;
    m_Width = other.m_Width;
    m_Height = other.m_Height;
    m_Channels = other.m_Channels;

    other.m_ID = 0;
    other.m_Width = other.m_Height = other.m_Channels = 0;
    return *this;
}

void Texture2D::Bind(unsigned int slot) const
{
    //抽象理解
    //比如bind(0)  就相当于
    //glActiveTexture(GL_TEXTURE0 + 0);      // 选中 0 号纹理单元（坑位） 选了一个坑位  GL_TEXTURE0是枚举常量  + slot 就相当于选slot号纹理单元
    //glBindTexture(GL_TEXTURE_2D, m_ID);    // 把这张纹理对象绑定到该单元的 2D 绑定点上 相当于往0号坑里 对应的2d绑定点上（会有多个绑定点） 填m_id指向的纹理 m_id类比于QImage img(xx); 一个纹理图片的指针


    // 纹理单元（Texture Unit）：
    // shader 里的 sampler2D uniform = slot 编号
    // glActiveTexture 选择“当前操作的纹理单元”
    glActiveTexture(GL_TEXTURE0 + slot);
    // 把该纹理对象(其实是m_ID句柄  类似于qimage 和 qimage的指针  但是效果是绑定句柄代表的对象即纹理对象)绑定到当前纹理单元的 GL_TEXTURE_2D 绑定点
    glBindTexture(GL_TEXTURE_2D, m_ID);
}

void Texture2D::Unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}
