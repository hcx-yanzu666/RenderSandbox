#include <cstdio>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Texture2D.h"

// ---------------------- Callbacks ----------------------
static void glfw_error_callback(int error, const char* description)
{
    std::fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static float g_ScrollY = 0.0f;
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    (void)window; (void)xoffset;
    g_ScrollY += (float)yoffset;
}

// ---------------------- Main ----------------------
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
    glfwSetScrollCallback(window, scroll_callback);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::fprintf(stderr, "Failed to initialize GLAD\n");
        return -1;
    }
    //按srgb格式加载
    Texture2D albedo("assets/textures/container.jpg", true);

    // --------- GL states (default ON; can be toggled in ImGui) ---------
    bool enableDepth = true;
    bool enableCull  = true;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_FRAMEBUFFER_SRGB);


    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // --------- ImGui ---------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    const char* glsl_version = "#version 330";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // --------- Cube geometry (positions only) ---------
    // pos.xyz + uv
    float vertices[] = {
    // --------- Front face (z = +0.5), normal = (0,0,1)
    -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,

     0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,

    // --------- Back face (z = -0.5), normal = (0,0,-1)
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,-1.0f,   1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,   0.0f, 0.0f,-1.0f,   0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,   0.0f, 0.0f,-1.0f,   0.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,-1.0f,   1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,   0.0f, 0.0f,-1.0f,   1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,   0.0f, 0.0f,-1.0f,   0.0f, 1.0f,

    // --------- Left face (x = -0.5), normal = (-1,0,0)
    -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f,

    // --------- Right face (x = +0.5), normal = (1,0,0)
     0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,

     0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,

    // --------- Bottom face (y = -0.5), normal = (0,-1,0)
    -0.5f, -0.5f, -0.5f,   0.0f,-1.0f, 0.0f,   0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,   0.0f,-1.0f, 0.0f,   1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,   0.0f,-1.0f, 0.0f,   1.0f, 0.0f,

     0.5f, -0.5f,  0.5f,   0.0f,-1.0f, 0.0f,   1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,   0.0f,-1.0f, 0.0f,   0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,   0.0f,-1.0f, 0.0f,   0.0f, 1.0f,

    // --------- Top face (y = +0.5), normal = (0,1,0)
    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,

     0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f
};


    unsigned int VAO = 0, VBO = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //apos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //anormal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //auv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Shader shader("assets/shaders/basic.vert", "assets/shaders/basic.frag");

    // --------- Render params ---------
    float color[4] = { 1.0f, 0.3f, 0.2f, 1.0f };
    // Camera (editor-like)
    glm::vec3 cameraPos(0.0f, 0.0f, 3.0f);
    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    float yaw = -90.0f;
    float pitch = 0.0f;
    float mouseSensitivity = 0.12f;
    glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraRight(1.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
    bool firstMouse = true;
    double lastX = 0.0, lastY = 0.0;
    bool panning = false;
    double panLastX = 0.0, panLastY = 0.0;
    float panSpeed = 0.005f;
    float fovDeg = 60.0f;
    float moveSpeed = 2.5f;
    float lastTime = (float)glfwGetTime();

    //光照参数
    float lightPos[3] = {1.2f,1.0f,2.0f};
    float lightColor[3] = {1.0f,1.0f,1.0f};
    //光照
    float shininess = 32.0f;
    //环境光强度
    float ambientStrength = 0.08f;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // delta time
        float now = (float)glfwGetTime();
        float dt = now - lastTime;
        lastTime = now;

        // --------- Optional: keyboard move (feel free to disable) ---------
        float velocity = moveSpeed * dt;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cameraFront * velocity;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cameraFront * velocity;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += cameraRight * velocity;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= cameraRight * velocity;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) cameraPos += worldUp * velocity;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) cameraPos -= worldUp * velocity;

        // --------- Mouse: RMB rotate (only when pressed) ---------
        int rmb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
        if (rmb == GLFW_PRESS)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            if (firstMouse) {
                lastX = xpos;
                lastY = ypos;
                firstMouse = false;
            }

            double xoffset = xpos - lastX;
            double yoffset = lastY - ypos; // invert Y for "look up"

            lastX = xpos;
            lastY = ypos;

            xoffset *= mouseSensitivity;
            yoffset *= mouseSensitivity;

            yaw   += (float)xoffset;
            pitch += (float)yoffset;

            if (pitch > 89.0f) pitch = 89.0f;
            if (pitch < -89.0f) pitch = -89.0f;
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            firstMouse = true;
        }

        // --------- Mouse: MMB pan ---------
        int mmb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);

        if (mmb == GLFW_PRESS)
        {
            if (!panning) {
                panning = true;
                panLastX = mx;
                panLastY = my;
            }

            double dx = mx - panLastX;
            double dy = my - panLastY;
            panLastX = mx;
            panLastY = my;

            cameraPos += cameraRight * (float)(dx * panSpeed);
            cameraPos -= cameraUp    * (float)(dy * panSpeed);
        }
        else
        {
            panning = false;
        }

        // --------- Scroll: FOV zoom ---------
        if (g_ScrollY != 0.0f)
        {
            fovDeg -= g_ScrollY * 2.0f;
            if (fovDeg < 20.0f) fovDeg = 20.0f;
            if (fovDeg > 90.0f) fovDeg = 90.0f;
            g_ScrollY = 0.0f;
        }

        // --------- Rebuild camera basis from yaw/pitch (stable, orthonormal) ---------
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);

        cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
        cameraUp    = glm::normalize(glm::cross(cameraRight, cameraFront));

        // --------- ImGui ---------
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("RenderSandbox");
        ImGui::ColorEdit4("Color", color);
        ImGui::SliderFloat("FOV (deg)", &fovDeg, 20.0f, 90.0f);
        ImGui::SliderFloat("Move Speed", &moveSpeed, 0.1f, 10.0f);
        ImGui::SliderFloat("Mouse Sens", &mouseSensitivity, 0.01f, 0.5f);
        ImGui::Checkbox("Depth Test", &enableDepth);
        ImGui::Checkbox("Backface Cull", &enableCull);
        ImGui::Text("Camera Pos: (%.2f, %.2f, %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);
        ImGui::Text("Yaw %.1f  Pitch %.1f", yaw, pitch);
        ImGui::Separator();
        ImGui::Text("Lighting");
        ImGui::SliderFloat3("Light Pos", lightPos, -10.0f, 10.0f);
        ImGui::ColorEdit3("Light Color", lightColor);
        ImGui::SliderFloat("Shininess", &shininess, 2.0f, 256.0f);
        ImGui::SliderFloat("Ambient", &ambientStrength, 0.0f, 0.3f);
        ImGui::End();

        ImGui::Render();

        // --------- Apply GL state toggles ---------
        if (enableDepth) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (enableCull)  glEnable(GL_CULL_FACE);  else glDisable(GL_CULL_FACE);

        // --------- Render ---------
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);

        glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Matrices
        glm::mat4 model(1.0f);
        // rotate around a non-axis-aligned vector to show depth/cull clearly
        model = glm::rotate(model, now * 0.8f, glm::normalize(glm::vec3(0.3f, 1.0f, 0.2f)));

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        float aspect = (h == 0) ? 1.0f : (float)w / (float)h;
        glm::mat4 proj = glm::perspective(glm::radians(fovDeg), aspect, 0.1f, 100.0f);

        shader.Bind();
        albedo.Bind(0);
        shader.setUniform1i("u_Texture0", 0);
        shader.setUniform3f("u_ViewPos", cameraPos.x, cameraPos.y, cameraPos.z);
        shader.setUniform3f("u_LightPos",   lightPos[0], lightPos[1], lightPos[2]);
        shader.setUniform3f("u_LightColor", lightColor[0], lightColor[1], lightColor[2]);
        shader.setUniform1f("u_Shininess",  shininess);
        shader.setUniform1f("u_AmbientStrength", ambientStrength);
        shader.SetMatrices(model, view, proj);
        shader.setUniform4f("u_Color", color[0], color[1], color[2], color[3]);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // cleanup
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
