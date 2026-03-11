#include <cstdio>
    #include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Texture2D.h"
#include "Material.h"
#include "Object.h"

#include "render/Renderer.h"
#include "render/Light.h"
#include "render/Framebuffer.h"
#include "render/PostProcessPass.h"
#include "Model.h"


// ---------------------- 回调 ----------------------
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

// ---------------------- 主函数 ----------------------
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

    // ---------------------- GL 状态 ----------------------
    bool enableDepth = true;
    bool enableCull  = false;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // pbr.frag 已手动做 pow(color, 1/2.2)，不能再开 FRAMEBUFFER_SRGB
    // 否则会做两次 gamma，图像过曝发白
    // glEnable(GL_FRAMEBUFFER_SRGB);

    // ---------------------- ImGui 初始化 ----------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    const char* glsl_version = "#version 330";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // ---------------------- 资源：纹理/着色器 ----------------------
    Texture2D albedo("assets/textures/container.jpg", true);
    Shader shader("assets/shaders/basic.vert", "assets/shaders/pbr.frag");
    Shader postShader("assets/shaders/post.vert", "assets/shaders/post.frag");

    Model model;
    if (!model.Load("assets/models/demo_cube.obj"))
    {
        std::fprintf(stderr, "Failed to load model!\n");
        return -1;
    }
    // ---------------------- 相机（你现在的控制逻辑不动） ----------------------
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

    // ---------------------- ImGui 参数（材质参数） ----------------------
    float tintColor[4] = { 1.0f, 0.3f, 0.2f, 1.0f };
    float shininess = 32.0f;
    float ambientStrength = 0.08f;
    int postMode = 0;
    float vignetteStrength = 0.35f;
    bool drawModel = true;
    float modelYaw = 0.0f;
    float modelScaleMul = 1.0f;
    // PBR 调节参数
    float metallic  = 0.0f;
    float roughness = 0.5f;
    float ao        = 1.0f;
    float lightIntensity = 30.0f;  // PBR 距离平方衰减，需要更高的光源强度

    // ---------------------- Material（共享） ----------------------
    Material litMat;
    litMat.shader = &shader;
    litMat.albedo = &albedo;
    litMat.color = glm::vec4(tintColor[0], tintColor[1], tintColor[2], tintColor[3]);
    litMat.shininess = shininess;
    litMat.ambientStrength = ambientStrength;

    // ---------------------- Object 列表（每个物体一个 model） ----------------------
    std::vector<Object> objects;
    objects.reserve(9);

    for (int x = -1; x <= 1; ++x)
    {
        for (int z = -1; z <= 1; ++z)
        {
            Object obj;
            obj.transform.position = glm::vec3((float)x * 1.5f, 0.0f, (float)z * 1.5f);
            obj.material = &litMat;
            objects.push_back(obj);
        }
    }

    // ---------------------- Renderer + Lights（A：Renderer 持有 lights） ----------------------
    Renderer renderer;

    std::vector<PointLight> lights;
    lights.push_back({ glm::vec3( 1.5f, 4.0f,  2.0f), glm::vec3(1.0f, 1.0f, 1.0f) });
    lights.push_back({ glm::vec3(-1.5f, 3.0f, -1.0f), glm::vec3(1.0f, 0.5f, 0.2f) });

    renderer.SetPointLights(lights);

    int fbw = 0, fbh = 0;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    
    Framebuffer sceneFbo;
    sceneFbo.Create(fbw, fbh);
    PostProcessPass postPass;
    postPass.Init(&postShader);
    // ---------------------- 主循环 ----------------------
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // delta time
        float now = (float)glfwGetTime();
        float dt = now - lastTime;
        lastTime = now;

        // 键盘移动
        float velocity = moveSpeed * dt;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cameraFront * velocity;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cameraFront * velocity;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += cameraRight * velocity;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= cameraRight * velocity;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) cameraPos += worldUp * velocity;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) cameraPos -= worldUp * velocity;

        // 鼠标右键旋转视角
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
            double yoffset = lastY - ypos;

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

        // 鼠标中键平移
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

        // 滚轮缩放 FOV
        if (g_ScrollY != 0.0f)
        {
            fovDeg -= g_ScrollY * 2.0f;
            if (fovDeg < 20.0f) fovDeg = 20.0f;
            if (fovDeg > 90.0f) fovDeg = 90.0f;
            g_ScrollY = 0.0f;
        }

        // 由 yaw/pitch 重建相机基向量
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);

        cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
        cameraUp    = glm::normalize(glm::cross(cameraRight, cameraFront));

        // ---------------------- ImGui ----------------------
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("RenderSandbox");
        ImGui::ColorEdit4("Tint Color", tintColor);
        ImGui::SliderFloat("FOV (deg)", &fovDeg, 20.0f, 90.0f);
        ImGui::SliderFloat("Move Speed", &moveSpeed, 0.1f, 10.0f);
        ImGui::SliderFloat("Mouse Sens", &mouseSensitivity, 0.01f, 0.5f);
        ImGui::Checkbox("Depth Test", &enableDepth);
        ImGui::Checkbox("Backface Cull", &enableCull);

        ImGui::Text("Camera Pos: (%.2f, %.2f, %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);
        ImGui::Text("Yaw %.1f  Pitch %.1f", yaw, pitch);

        ImGui::Separator();
        ImGui::Text("Material");
        ImGui::SliderFloat("Shininess", &shininess, 2.0f, 256.0f);
        ImGui::SliderFloat("Ambient", &ambientStrength, 0.0f, 0.3f);
        ImGui::Text("PBR");
        ImGui::SliderFloat("Metallic",   &metallic,       0.0f, 1.0f);
        ImGui::SliderFloat("Roughness",  &roughness,      0.05f, 1.0f);
        ImGui::SliderFloat("AO",         &ao,             0.0f, 1.0f);
        ImGui::SliderFloat("Light Intensity", &lightIntensity, 1.0f, 300.0f);
        ImGui::Separator();
        ImGui::Text("PostProcess");
        ImGui::Combo("Mode", &postMode, "None\0Invert\0Grayscale\0");
        ImGui::SliderFloat("Vignette", &vignetteStrength, 0.0f, 1.0f);
        ImGui::Separator();
        ImGui::Text("Model");
        ImGui::Checkbox("Draw Model", &drawModel);
        ImGui::SliderFloat("Model Yaw", &modelYaw, -180.0f, 180.0f);
        ImGui::SliderFloat("Model Scale Mul", &modelScaleMul, 0.1f, 5.0f);
        ImGui::Text("Model Radius: %.3f", model.GetRadius());
        ImGui::Separator();
        ImGui::End();

        ImGui::Render();

        // ---------------------- GL 状态开关 ----------------------
        if (enableDepth) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (enableCull)  glEnable(GL_CULL_FACE);  else glDisable(GL_CULL_FACE);

        // ---------------------- 清屏 + Offscreen(FBO) ----------------------
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        sceneFbo.Resize(w, h);

        sceneFbo.Bind();
        glViewport(0, 0, w, h);

        glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // ---------------------- 计算 view/proj ----------------------
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        float aspect = (h == 0) ? 1.0f : (float)w / (float)h;
        glm::mat4 proj = glm::perspective(glm::radians(fovDeg), aspect, 0.1f, 100.0f);

        // ---------------------- 每帧把 ImGui 参数写回材质 ----------------------
        litMat.color = glm::vec4(tintColor[0], tintColor[1], tintColor[2], tintColor[3]);
        litMat.shininess = shininess;
        litMat.ambientStrength = ambientStrength;
        litMat.metallic  = metallic;
        litMat.roughness = roughness;
        litMat.ao        = ao;

        // 用 lightIntensity 缩放光源颜色传入 shader
        // PBR 距离平方衰减需要更强的光源才能看到效果
        litMat.Bind(cameraPos);   // 激活 shader + 传材质 uniform
        shader.setUniform1i("u_PointLightCount", (int)lights.size());
        for (int li = 0; li < (int)lights.size(); li++)
        {
            shader.setUniform3f(("u_PointLights[" + std::to_string(li) + "].position").c_str(),
                lights[li].position.x, lights[li].position.y, lights[li].position.z);
            glm::vec3 scaledColor = lights[li].color * lightIntensity;
            shader.setUniform3f(("u_PointLights[" + std::to_string(li) + "].color").c_str(),
                scaledColor.x, scaledColor.y, scaledColor.z);
        }

        if (drawModel)
        {
            glm::mat4 modelMat(1.0f);
            modelMat = glm::rotate(modelMat, glm::radians(modelYaw), glm::vec3(0.0f, 1.0f, 0.0f));

            float radius = model.GetRadius();
            float fitScale = (radius > 0.0001f) ? (1.2f / radius) : 1.0f;
            if (fitScale > 100.0f) fitScale = 100.0f;
            modelMat = glm::scale(modelMat, glm::vec3(fitScale * modelScaleMul));
            modelMat = glm::translate(modelMat, -model.GetCenter());

            shader.SetMatrices(modelMat, view, proj);
            model.Draw();
        }

        // ---------------------- Present: FBO -> Default ----------------------
        postPass.Execute(sceneFbo.ColorTex(), w, h, postMode, vignetteStrength);



        // ---------------------- ImGui 渲染 ----------------------
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // ---------------------- 清理 ----------------------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
