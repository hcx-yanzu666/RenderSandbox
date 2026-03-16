#version 330 core
out vec4 FragColor;
in vec3 vLocalPos;

uniform samplerCube u_Skybox;

void main()
{
    // vLocalPos 就是方向向量，直接采样 Cubemap
    vec3 color = texture(u_Skybox, vLocalPos).rgb;

    // HDR 转 LDR（简单 tone mapping）
    color = color / (color + vec3(1.0));
    // Gamma 矫正
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
