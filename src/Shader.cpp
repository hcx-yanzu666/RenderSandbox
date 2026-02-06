#include "Shader.h"

#include <glad/glad.h>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>
#include <glm/gtc/type_ptr.hpp>

static const char* ShaderTypeToString(unsigned int type)
{
    switch (type) {
        case GL_VERTEX_SHADER:   return "VERTEX";
        case GL_FRAGMENT_SHADER: return "FRAGMENT";
        default:                 return "UNKNOWN";
    }
}

std::string Shader::ReadFile(const std::string& filepath)
{
    std::ifstream stream(filepath, std::ios::in | std::ios::binary);
    if (!stream.is_open()) {
        std::fprintf(stderr, "[Shader] Failed to open file: %s\n", filepath.c_str());
        return "";
    }

    std::stringstream ss;
    ss << stream.rdbuf();
    return ss.str();
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
    //创建shader对象 type为传进来的比如GL_VERTEX_SHADER GL_FRAGMENT_SHADER
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    //源码交给olengl src是字符串指针
    glShaderSource(id, 1, &src, nullptr);
    //触发编译
    glCompileShader(id);

    int result = 0;
    //检查编译结果
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        //失败时拉取日志 打印错误.....
        int length = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

        std::vector<char> message(length);
        glGetShaderInfoLog(id, length, &length, message.data());

        std::fprintf(stderr, "[Shader] %s compilation failed:\n%s\n",
                     ShaderTypeToString(type), message.data());

        glDeleteShader(id);
        return 0;
    }

    return id;
}

//将顶点着色器和片段着色器 组合成一个可用的program 后续添加额外着色器会扩展
unsigned int Shader::CreateShaderProgram(const std::string& vertexSrc, const std::string& fragmentSrc)
{
    //创建program
    unsigned int program = glCreateProgram();

    //编译顶点着色器
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexSrc);
    //编译片段着色器
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);

    if (vs == 0 || fs == 0) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        glDeleteProgram(program);
        return 0;
    }

    //挂载和连接
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int linked = 0;
    //检查并判断是否连接成功  打印错误日志
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
        int length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

        std::vector<char> message(length);
        glGetProgramInfoLog(program, length, &length, message.data());

        std::fprintf(stderr, "[Shader] Program link failed:\n%s\n", message.data());

        glDeleteShader(vs);
        glDeleteShader(fs);
        glDeleteProgram(program);
        return 0;
    }

    //清理
    glDetachShader(program, vs);
    glDetachShader(program, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

int Shader::GetUniformLocation(const std::string& name) const
{
    int location = glGetUniformLocation(m_RendererID,name.c_str());
    if (location == -1)
    {
        std::fprintf(stderr, "[Shader] Warning: uniform '%s' doesn't exist or is not used.\n", name.c_str());
    }
    return location;
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
{
    //构造函数：串联流程
    //1.ReadFile读两个文件
    //2.CreateShaderProgram得到program ID
    std::string vertexSrc = ReadFile(vertexPath);
    std::string fragmentSrc = ReadFile(fragmentPath);

    if (vertexSrc.empty() || fragmentSrc.empty()) {
        std::fprintf(stderr, "[Shader] Empty shader source. Vertex: %s, Fragment: %s\n",
                     vertexPath.c_str(), fragmentPath.c_str());
        m_RendererID = 0;
        return;
    }

    m_RendererID = CreateShaderProgram(vertexSrc, fragmentSrc);
    if (m_RendererID == 0) {
        std::fprintf(stderr, "[Shader] Failed to create shader program.\n");
    }
}

Shader::~Shader()
{
    if (m_RendererID)
        glDeleteProgram(m_RendererID);
}

//渲染时切换当前程序
void Shader::Bind() const
{
    glUseProgram(m_RendererID);
}

void Shader::Unbind() const
{
    glUseProgram(0);
}

void Shader::setUniformMat4(const std::string& name, const glm::mat4& matrix)
{
    int location = GetUniformLocation(name);
    if (location != -1)
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}


void Shader::setUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    int location = GetUniformLocation(name);
    if (location != -1)
        glUniform4f(location,v0,v1,v2,v3);
}

void Shader::setUniform1i(const std::string& name, int v)
{
    //把shader里的 u_Texture0（sampler2D）设置为 v
    //含义是  这个sampler 从 texture unit v 取纹理 （从 v 号坑的 (sampler2D → 找 unitv 的 GL_TEXTURE_2D) 取纹理）
    int location = GetUniformLocation(name);
    if (location != -1)
        glUniform1i(location, v);
}

void Shader::setUniform3f(const std::string& name, float v0, float v1, float v2)
{
    int location = GetUniformLocation(name);
    if (location != -1)
        glUniform3f(location,v0,v1,v2);
}

void Shader::setUniform1f(const std::string& name, float v)
{
    int location = GetUniformLocation(name);
    if (location != -1)
        glUniform1f(location,v);
}

void Shader::SetMatrices(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj)
{
    setUniformMat4("u_Model",model);
    setUniformMat4("u_View", view);
    setUniformMat4("u_Proj", proj);
}
