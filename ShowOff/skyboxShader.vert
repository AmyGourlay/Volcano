/*skyboxShader.frag
  Shader for the skybox
  Code for SkyBox taken and adapted from: https://learnopengl.com/Advanced-OpenGL/Cubemaps
 */

#version 330 core
layout(location = 0) in vec3 aPos;

out vec3 TexCoords;
out float fdistance;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = (projection * view * model) * vec4(aPos, 1.0);
    //vec4 pos = vec4(1.0, 0.0, 0.0, 1.0);
    gl_Position = pos;

    //ftexcoord = texcoord.xy;
}  
