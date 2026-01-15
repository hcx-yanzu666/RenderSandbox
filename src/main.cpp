#include <cstdio>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


static void glfw_error_callback(int error, const char* description)
{
    std::fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "RenderSandbox", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // ✅ 初始化 GLAD（必须在 OpenGL Context 创建之后）
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::fprintf(stderr, "Failed to initialize GLAD\n");
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    const char* glsl_version = "#version 330";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);



    //创建一个三角形
    float vertices[] = {
        -0.5f,-0.5f,0.0f,
        0.5f,-0.5f,0.0f,
        0.0f,0.5f,0.0f
        };

    unsigned int VAO,VBO;
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);

    //绑定VAO 状态容器
    glBindVertexArray(VAO);

    //绑定VBO 顶点数据
    glBindBuffer(GL_ARRAY_BUFFER,VBO);//gpu显存开辟一块buffer
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW); //把vertices 从 cpu 拷贝到gpu

    // 3. 告诉 OpenGL 如何解释顶点数据
    glVertexAttribPointer(
        0,              // location = 0
        3,              // vec3
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        (void*)0
    );
    glEnableVertexAttribArray(0);

    //解绑
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);

    Shader shader("assets/shaders/basic.vert", "assets/shaders/basic.frag");

    float colortest[4] = {1.0f,0.3f,0.2f,1.0f};
    float angleDeg = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Hello");
        ImGui::Text("GLFW + ImGui is working.");
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::ColorEdit4("Triangle Color", colortest);
        ImGui::SliderFloat("Angle (deg)", &angleDeg, -180.0f, 180.0f);
        ImGui::End();

        ImGui::Render();

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 画三角形

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(angleDeg), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 proj = glm::mat4(1.0f);
        glm::mat4 mvp = proj * view * model;

        shader.Bind();
        shader.setUniformMat4("u_MVP", mvp);
        shader.setUniform4f("u_Color", colortest[0], colortest[1], colortest[2], colortest[3]);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
