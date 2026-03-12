#version 330 core
out vec4 FragColor;
in vec2 vUV;

uniform sampler2D u_SceneTex;
uniform int   u_Mode;              // 0=none 1=invert 2=grayscale
uniform float u_VignetteStrength;  // 0..1

//暗角(中心亮边缘暗)
vec3 vignette(vec2 uv, vec3 c, float s)
{
    //计算中心距离（找画面中心）
    vec2 d = uv - vec2(0.5);
    float r2 = dot(d, d);
    //计算渐变因子
    float v = smoothstep(0.8, 0.2, r2);
    //混合强度
    return mix(c, c * v, s);
}

void main()
{
    //根据当前像素的 UV 从场景纹理里采样颜色
    vec3 c = texture(u_SceneTex, vUV).rgb;

        // ---- Tone mapping：HDR -> LDR ----
        // 物理正确的光照计算结果可能超过 1.0（HDR）
        // Reinhard 算子把它压回 [0,1]
        c = c / (c + vec3(1.0));

        // ---- Gamma 矫正：线性 -> sRGB ----
        c = pow(c, vec3(1.0 / 2.2));

    //Mode1 反色
    if (u_Mode == 1) c = vec3(1.0) - c;
    //Mode2 灰度
    if (u_Mode == 2) {
        //灰度公式 人眼感知加权
        float g = dot(c, vec3(0.2126, 0.7152, 0.0722));
        c = vec3(g);
    }

    c = vignette(vUV, c, u_VignetteStrength);
    FragColor = vec4(c, 1.0);
}
