// Minimal vertex shader for Point Sprite object
// Adapted from Example 6.1 in the Redbook V4.3

#version 400

// These are the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;


// Uniform variables are passed in from the application
uniform mat4 model, view, projection;
uniform uint colourmode;

// Output the vertex colour - to be rasterized into pixel fragments
out vec4 fcolour;
out float Transp;
uniform float size;

void main()
{
	vec4 colour_h = vec4(colour, 0.5);
	vec4 pos = vec4(position, 1.0);
	vec4 pos2 = model * pos;
	Transp = 0.0;
	
	// Pass through the vertex colour
	fcolour = vec4(0.658824, 0.658824, 0.658824, 1.0);//colour_h;

	// Define the vertex position
	gl_Position = projection * view * model * pos;

	gl_PointSize = (2.0 - pos2.z / pos2.w) * size;
	//gl_PointSize = size;

	Transp = 0.0001- (1 - pos2.z / pos2.w);
}

