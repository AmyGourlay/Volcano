// Minimal vertex shader

#version 400

// These are the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 texcoord;

// Uniform variables are passed in from the application
uniform mat4 model, view, projection;
uniform uint colourmode;
out vec2 ftexcoord;
out vec3 fposition;

// Output the vertex colour - to be rasterized into pixel fragments
out vec4 fcolour, diffuse_colour, fposition_h;
out vec3 fnormal;
out float fdistance;

void main()
{
	diffuse_colour = vec4(colour,1.0);
	vec4 position_h = vec4(position, 1.0);

	fposition = position;
	fnormal = normal;
	fposition_h = position_h;

	// Define the vertex position
	gl_Position = projection * view * model * position_h;
	
	ftexcoord = texcoord.xy;
	fposition = position;
}

