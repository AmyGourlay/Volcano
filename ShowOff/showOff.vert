/*lightsCameraAction.vert
  Main Vertex Shader*/

// Specify minimum OpenGL version
#version 420 core

// Define the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 texcoord;

// This is the output vertex colour sent to the rasterizer
out vec4 fscolour, diffuse_albedo;
out vec3 fposition, flightpos, fnormal, texCoords;
out vec2 ftexcoord;

// These are the uniforms that are defined in the application
uniform mat4 model, view, projection;
uniform mat3 normalmatrix;
uniform uint colourmode, emitmode, texturemode;
uniform vec4 lightpos;
uniform uint attenuationmode;

void main()
{
	vec4 position_h = vec4(position, 1.0);	// Convert the (x,y,z) position to homogeneous coords (x,y,z,w)
	vec3 light_pos3 = lightpos.xyz;			

	fscolour = vec4(colour, 1.0);//vec4(0.0, 1.0, 0.0, 1.0);
	fnormal = normal;
	flightpos = light_pos3;
	fposition = position;

	gl_Position = (projection * view * model) * position_h;

	// Output the texture coordinates with no modifications
	ftexcoord = texcoord.xy;
}


