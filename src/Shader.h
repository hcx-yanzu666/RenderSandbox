#pragma once
#include <string>
#include <glm/glm.hpp>

/**
* Shader 类的作用是将繁琐且易出错的 OpenGL 着色器创建、编译和链接流程封装起来，
* 统一管理 GPU program 的生命周期。
在渲染阶段，通过 Bind 切换当前使用的 shader program，
从而对同一套 VAO/VBO 的几何数据应用不同的渲染逻辑，实现不同的视觉效果。
 */
class Shader
{
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    void Bind() const;
    void Unbind() const;

    unsigned int GetRendererID() const { return m_RendererID; }
    void setUniformMat4(const std::string& name, const glm::mat4& matrix);
    void setUniform4f(const std::string& name,float v0,float v1,float v2,float v3);
private:
    //program object的句柄 一个可执行的着色器程序
    unsigned int m_RendererID = 0;

    static std::string ReadFile(const std::string& filepath);
    //把GLSL文本->gpu能理解的shader object
    static unsigned int CompileShader(unsigned int type, const std::string& source);
    static unsigned int CreateShaderProgram(const std::string& vertexSrc, const std::string& fragmentSrc);
    int GetUniformLocation(const std::string& name) const;
};
