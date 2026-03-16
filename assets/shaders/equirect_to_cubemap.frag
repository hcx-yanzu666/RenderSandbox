#version 330 core
out vec4 FragColor;
in vec3 vLocalPos;

uniform sampler2D u_EquirectMap;  // 你加载的 HDR 全景图

// 1/(2π) 和 1/π，用于把弧度归一化到 [0,1]
const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
    // atan(z, x) → 水平角度（-π ~ π）
    // asin(y)    → 垂直角度（-π/2 ~ π/2）
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;   // 归一化到 [-0.5, 0.5]
    uv += 0.5;       // 平移到 [0, 1]
    return uv;
}

void main()
{
    vec2 uv = SampleSphericalMap(normalize(vLocalPos));
    vec3 color = texture(u_EquirectMap, uv).rgb;
    FragColor = vec4(color, 1.0);
}
