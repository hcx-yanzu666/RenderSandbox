#version 330 core
out vec4 FragColor;

in vec2 vUV;
in vec3 vFragPos;   // world space
in vec3 vNormal;    // world space

uniform sampler2D u_Texture0;
uniform vec4  u_Color;

uniform vec3 u_LightDir;//光线传播方向 平行光
uniform vec3  u_LightColor;
uniform vec3  u_ViewPos;
uniform float u_Shininess;
uniform float u_AmbientStrength;

void main()
{
    vec3 albedo = texture(u_Texture0, vUV).rgb * u_Color.rgb;

    vec3 N = normalize(vNormal);
    vec3 L = normalize(-u_LightDir);
    vec3 V = normalize(u_ViewPos   - vFragPos);


    // 1) ambient（先给个常量）
    // 环境光 常数*物体颜色
    vec3 ambient = u_AmbientStrength * albedo;

    // 2) diffuse（Lambert）(diff : 漫反射)
    // 漫反射原理：法线向量与光线向量的点乘 向量角度越大->点乘越小->离光源角度更大->更暗   光线垂直于物体表面时最亮
    float diff = max(dot(N,L),0.0);

    // 3) specular（Blinn-Phong) (镜面光)
    //H:理想反射方向
    //法线 +  观察的方向  得到一个中间角度 H 这个就是最佳反射角度
    vec3 H = normalize(L + V);
    //然后通过这个最佳角度 和  当前的法线算点乘 N 越朝向 H，越像把光反射进眼睛
    float spec = pow(max(dot(N, H), 0.0), u_Shininess);

    vec3 diffuse  = diff * albedo * u_LightColor;
    vec3 specular = spec * u_LightColor;

    vec3 color = ambient + diffuse + specular;
    FragColor = vec4(color, 1.0);
}
