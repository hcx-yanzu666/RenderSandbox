#include "IBLBaker.h"
#include "Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//6个面的朝向 从原看向 +x/-x/+y/-y/+z/-z
static const glm::mat4 s_CaptureViews[6] = {
    glm::lookAt(glm::vec3(0), glm::vec3( 1, 0, 0), glm::vec3(0,-1, 0)), // +X
    glm::lookAt(glm::vec3(0), glm::vec3(-1, 0, 0), glm::vec3(0,-1, 0)), // -X
    glm::lookAt(glm::vec3(0), glm::vec3( 0, 1, 0), glm::vec3(0, 0, 1)), // +Y
    glm::lookAt(glm::vec3(0), glm::vec3( 0,-1, 0), glm::vec3(0, 0,-1)), // -Y
    glm::lookAt(glm::vec3(0), glm::vec3( 0, 0, 1), glm::vec3(0,-1, 0)), // +Z
    glm::lookAt(glm::vec3(0), glm::vec3( 0, 0,-1), glm::vec3(0,-1, 0)), // -Z
};

void IBLBaker::Bake(uint32_t hdrTexID)
{
    std::printf("[IBLBaker] Starting bake...\n");
    BakeCubemap(hdrTexID);
    BakeIrradiance();
    // BakePrefilter();     // 下一步
    // BakeBRDFLUT();       // 下一步
    std::printf("[IBLBaker] Bake complete.\n");
}

// 90° FOV，1:1 宽高比（每个面都是正方形）
static const glm::mat4 s_CaptureProj =
    glm::perspective(glm::radians(90.0f),1.0f,0.1f,10.0f);

void IBLBaker::BakeCubemap(uint32_t hdrTexID)
{
    // 1) 创建 512×512 的 Cubemap 纹理（6 个面） 生成一个纹理对象 ID，存到 m_EnvCubemap 分配句柄
    glGenTextures(1, &m_EnvCubemap);
    //绑定到GL_TEXTURE_CUBE_MAP 目标 后续的操作都针对这个 Cubemap
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);
    for (int i = 0; i < 6; i++)
    {
     // GL_TEXTURE_CUBE_MAP_POSITIVE_X + i 依次对应 6 个面
     glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 2) 创建离屏 FBO（只需要颜色附件，不需要深度）
    glGenFramebuffers(1, &m_CaptureFBO);
    glGenRenderbuffers(1, &m_CaptureRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, m_CaptureRBO);

    // 用转换shader渲染六次
    Shader convShader("assets/shaders/cubemap.vert",
                  "assets/shaders/equirect_to_cubemap.frag");

    convShader.Bind();
    convShader.setUniform1i("u_EquirectMap", 0);
    convShader.setUniformMat4("u_Projection", s_CaptureProj);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexID);

    glViewport(0, 0, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        std::printf("[IBLBaker] FBO incomplete! Status: 0x%X\n", status);
    else
        std::printf("[IBLBaker] FBO complete. EnvCubemap ID: %u\n", m_EnvCubemap);

    glDisable(GL_CULL_FACE);   // ← 烘焙前禁用剔除

    for (int i = 0; i < 6; i++)
    {
        //切换到离屏fbo
        convShader.setUniformMat4("u_View", s_CaptureViews[i]);
        // 把 Cubemap 的第 i 个面挂到 FBO 的颜色附件上
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               m_EnvCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderCube();  // 画单位立方体
    }

    glEnable(GL_CULL_FACE);    // ← 烘焙后恢复（可选，主循环里也会设置）



    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void IBLBaker::BakeIrradiance()
{
    // 1) 创建 32×32 的 IrradianceMap（低分辨率就够，因为是模糊结果）
    glGenTextures(1, &m_IrradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_IrradianceMap);
    for (int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 2) 复用已有的 FBO，但 RBO 要改成 32×32
    glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

    // 3) 卷积 shader（顶点复用 cubemap.vert，只需要位置+方向）
    Shader irrShader("assets/shaders/cubemap.vert",
                     "assets/shaders/irradiance_convolution.frag");
    irrShader.Bind();
    irrShader.setUniform1i("u_EnvMap", 0);
    irrShader.setUniformMat4("u_Projection", s_CaptureProj);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap); // 输入：上一步烘好的 Cubemap

    glViewport(0, 0, 32, 32);
    glDisable(GL_CULL_FACE);

    for (int i = 0; i < 6; i++)
    {
        irrShader.setUniformMat4("u_View", s_CaptureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               m_IrradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderCube();
    }

    glEnable(GL_CULL_FACE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void IBLBaker::BakePrefilter()
{
    // 下一步实现
}

void IBLBaker::BakeBRDFLUT()
{
    // 下一步实现
}

void IBLBaker::RenderCube()
{
    // 懒加载：第一次调用时创建 VAO
    if (m_CubeVAO == 0)
    {
        // 单位立方体顶点数据（36 个顶点，每个 3 个 float）
        float vertices[] = {
            // 后面 (-Z)
            -1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            // 前面 (+Z)
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,
            // 左面 (-X)
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            // 右面 (+X)
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            // 下面 (-Y)
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            // 上面 (+Y)
            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f
        };

        glGenVertexArrays(1, &m_CubeVAO);
        glGenBuffers(1, &m_CubeVBO);
        glBindVertexArray(m_CubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_CubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glBindVertexArray(0);
    }

    glBindVertexArray(m_CubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void IBLBaker::RenderQuad()
{
    // 下一步实现（BRDF LUT 用）
}



void IBLBaker::Destroy()
{
    if (m_EnvCubemap)    glDeleteTextures(1, &m_EnvCubemap);
    if (m_IrradianceMap) glDeleteTextures(1, &m_IrradianceMap);
    if (m_PrefilterMap)  glDeleteTextures(1, &m_PrefilterMap);
    if (m_BrdfLUT)       glDeleteTextures(1, &m_BrdfLUT);
    if (m_CubeVAO)       glDeleteVertexArrays(1, &m_CubeVAO);
    if (m_CubeVBO)       glDeleteBuffers(1, &m_CubeVBO);
    if (m_QuadVAO)       glDeleteVertexArrays(1, &m_QuadVAO);
    if (m_QuadVBO)       glDeleteBuffers(1, &m_QuadVBO);
    if (m_CaptureFBO)    glDeleteFramebuffers(1, &m_CaptureFBO);
    if (m_CaptureRBO)    glDeleteRenderbuffers(1, &m_CaptureRBO);

    m_EnvCubemap = m_IrradianceMap = m_PrefilterMap = m_BrdfLUT = 0;
    m_CubeVAO = m_CubeVBO = m_QuadVAO = m_QuadVBO = 0;
    m_CaptureFBO = m_CaptureRBO = 0;
}
