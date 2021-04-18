/*skyboxShader.frag
  Shader for the skybox
  Code for SkyBox taken and adapted from: https://learnopengl.com/Advanced-OpenGL/Cubemaps
 */

#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube cubemapTexture;

void main()
{    
    FragColor = texture(cubemapTexture, TexCoords);
}
