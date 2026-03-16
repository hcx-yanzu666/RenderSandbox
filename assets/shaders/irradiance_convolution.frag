#version 330 core
out vec4 FragColor;
in vec3 vLocalPos;

uniform samplerCube u_EnvMap;

const float PI = 3.14159265359;

void main()
{
    // vLocalPos 就是这个像素代表的"法线方向"
    vec3 normal = normalize(vLocalPos);

   // normal 接近 Y 轴时，改用 X 轴作为参考，避免 cross 结果为零向量
   vec3 up    = abs(normal.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
   vec3 right = normalize(cross(up, normal));
    up         = normalize(cross(normal, right));

    vec3  irradiance = vec3(0.0);
    float nrSamples  = 0.0;
    float sampleDelta = 0.025; // 步长越小越精确，但越慢

    // 用球面坐标遍历法线方向的半球
    // phi  = 水平方向 [0, 2π]
    // theta = 仰角    [0, π/2]
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // 1. 球面坐标 → 切线空间的方向向量
            vec3 tangentDir = vec3(sin(theta) * cos(phi),
                                   sin(theta) * sin(phi),
                                   cos(theta));

            // 2. 切线空间 → 世界空间（用上面建好的坐标轴）
            vec3 sampleDir = tangentDir.x * right
                           + tangentDir.y * up
                           + tangentDir.z * normal;

            // 3. 采样环境光，乘以两个权重
            //    cos(theta) : 兰伯特定律，斜入射贡献更少
            //    sin(theta) : 球面积分修正（解释见下方）
            irradiance += texture(u_EnvMap, sampleDir).rgb
                        * cos(theta) * sin(theta);
            nrSamples++;
        }
    }

    // π 是对半球积分 cos(θ)sinθ dθdφ 的解析结果
    irradiance = PI * irradiance / nrSamples;

    FragColor = vec4(irradiance, 1.0);
}
