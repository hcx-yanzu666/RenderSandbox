#version 330 core
out vec4 FragColor;

in vec2 vUV;

uniform sampler2D u_Texture0;  // 纹理采样器
uniform vec4 u_Color;          // 保留，用来 tint

void main()
{
    vec4 texColor = texture(u_Texture0, vUV);
    FragColor = texColor * u_Color; // 乘颜色作为 tint
}
