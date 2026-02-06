#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

out vec2 vUV;
out vec3 vFragPos;
out vec3 vNormal;

void main()
{
    vUV = aUV;
    vec4 worldPos = u_Model * vec4(aPos,1.0);
    vFragPos = worldPos.xyz;
    mat3 normalMat = mat3(transpose(inverse(u_Model)));
    vNormal = normalize(normalMat * aNormal);

    gl_Position = u_Proj * u_View * worldPos;
}
