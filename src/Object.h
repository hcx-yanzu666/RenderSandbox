#pragma once
#include <glm/glm.hpp>
#include "Transform.h"

struct Material;

// Object：一个具体实体（“这一个箱子”）
// - 它有 Transform（怎么摆放）
// - 它引用一个 Material（用什么渲染）
struct Object
{
    Transform transform;
    Material* material = nullptr;
};