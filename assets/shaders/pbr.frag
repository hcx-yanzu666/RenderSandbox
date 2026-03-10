#version 330 core
out vec4 FragColor;

in vec2 vUV;
in vec3 vFragPos; //worlad space
in vec3 vNormal; //world space

//PBR:用物理模型计算材料如何反射光。
//PBR核心:BRDF 决定多少光会反射到眼睛  核心公式：D(微表面分布 统计微表面有多少朝向H 从小到大决定了高光从集中到扩散) G(几何遮挡：微表面互相挡住光，G越小遮挡越多，越接近1越通畅) F(菲涅尔 角度越斜反射越强 比如水正看透明 斜看反光)
//光照 → BRDF计算 → HDR(允许光照计算产生大于1的亮度，从而保留真实光照强度差异，最后再通过 tone mapping 显示到屏幕) → Tone Mapping(色调映射 压光 避免过曝) → Gamma(srgb伽马矫正) → 屏幕

// ---- 材质参数 ----
uniform sampler2D  u_Texture0;   // albedo map
uniform vec4       u_Color;      // albedo tint(乘以贴图颜色)
uniform float      u_Metallic;   // 0 = 非金属, 1 = 金属
uniform float      u_Roughness;  // 0 = 镜面光滑, 1 = 完全粗糙
uniform float      u_AO;         // ambient occlusion(0~1,暂时传 1.0)

// ---- 相机位置 ----
uniform vec3 u_ViewPos;

// ---- 点光源 ----
struct PointLight {
   vec3 position;
   vec3 color;
};
#define MAX_POINT_LIGHTS 8
uniform int        u_PointLightCount;
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];

// -------------------------------------------------------------------------
// PBR 核心函数
// -------------------------------------------------------------------------

const float PI = 3.14159265359;

// D: 法线分布函数(GGX / Trowbridge - Reitz)
// 作用: 统计微表面中有多少个对齐到H方向(光方向 和 视线方向 的中间方向  如果某个微表面的法线 = H 它就会把光反射到眼睛)的面
// roughness 越大,高光越分散
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a  = roughness * roughness;   // 对粗糙度平方可得更直观的视觉效果
    float a2 = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

// G：几何遮蔽函数（Smith + Schlick-GGX）
// 作用：统计微表面自遮挡比例，roughness 越大遮挡越多
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness)
         * GeometrySchlickGGX(NdotL, roughness);
}

// F：Fresnel 方程（Schlick 近似）
// 作用：视线角度越大（掠射），反射越强
// cosTheta = dot(H, V)，F0 = 垂直入射时的基础反射率
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ----------------------------------------------------------------
// 主函数
// ----------------------------------------------------------------
void main()
{
    // ---- albedo：从 sRGB 转换到线性空间 ----
    // 贴图文件通常存储为 sRGB，PBR 计算必须在线性空间里做
    vec3 albedo = pow(texture(u_Texture0, vUV).rgb, vec3(2.2)) * u_Color.rgb;

    float metallic  = clamp(u_Metallic,  0.0, 1.0);
    float roughness = clamp(u_Roughness, 0.05, 1.0); // 避免完全 0 导致除零
    float ao        = u_AO;

    vec3 N = normalize(vNormal);
    vec3 V = normalize(u_ViewPos - vFragPos);

    // ---- F0：垂直入射时的基础反射率 ----
    // 非金属 F0 ≈ 0.04（几乎所有非金属都接近这个值）
    // 金属 F0 = albedo（金属的高光带颜色）
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // ---- 对每个点光源累加 radiance ----
    vec3 Lo = vec3(0.0);
    int count = clamp(u_PointLightCount, 0, MAX_POINT_LIGHTS);

    for (int i = 0; i < count; i++)
    {
        vec3 L = normalize(u_PointLights[i].position - vFragPos);
        vec3 H = normalize(V + L);

        // 点光源衰减：距离平方反比（物理正确）
        float dist        = length(u_PointLights[i].position - vFragPos);
        float attenuation = 1.0 / (dist * dist);
        vec3  radiance    = u_PointLights[i].color * attenuation;

        // ---- Cook-Torrance BRDF ----
        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3  F = FresnelSchlick(max(dot(H, V), 0.0), F0);

        // 镜面反射项
        vec3  num   = D * G * F;
        float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3  specular = num / denom;

        // kS = F（Fresnel 告诉我们多少光被镜面反射）
        // kD = 1 - kS（剩余的才能漫反射）
        // 金属没有漫反射（metallic=1 时 kD=0）
        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // ---- 环境光（IBL 之前的占位符）----
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color   = ambient + Lo;

    // ---- Tone mapping：HDR -> LDR ----
    // 物理正确的光照计算结果可能超过 1.0（HDR）
    // Reinhard 算子把它压回 [0,1]
    color = color / (color + vec3(1.0));

    // ---- Gamma 矫正：线性 -> sRGB ----
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}