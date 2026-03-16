#version 330 core
layout(location = 0)in vec3 aPos;

//单位立方体的顶点坐标范围是[-1,1] 从远点看向任意顶点 这个坐标本身就是方向。
out vec3 vLocalPos; //顶点的本地坐标，就是方向向量

uniform mat4 u_Projection;
uniform mat4 u_View;

void main()
{
    vLocalPos = aPos;
    gl_Position = u_Projection * u_View * vec4(aPos,1.0);
}