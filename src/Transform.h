#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Transform：描述一个物体“怎么摆放”
// 位置 position、旋转 rotationEuler（欧拉角：度）、缩放 scale
struct Transform
{
    glm::vec3 position {0.0f, 0.0f, 0.0f};
    glm::vec3 rotationEulerDeg {0.0f, 0.0f, 0.0f}; // pitch(x), yaw(y), roll(z) 习惯而已
    glm::vec3 scale {1.0f, 1.0f, 1.0f};

    // 由 Transform 生成 model 矩阵
    glm::mat4 ToMatrix() const
    {
        glm::mat4 m(1.0f);
        m = glm::translate(m, position);

        // 旋转顺序：X -> Y -> Z（你可以固定一个顺序，之后再讲欧拉角的坑）
        m = glm::rotate(m, glm::radians(rotationEulerDeg.x), glm::vec3(1,0,0));
        m = glm::rotate(m, glm::radians(rotationEulerDeg.y), glm::vec3(0,1,0));
        m = glm::rotate(m, glm::radians(rotationEulerDeg.z), glm::vec3(0,0,1));

        m = glm::scale(m, scale);
        return m;
    }
};