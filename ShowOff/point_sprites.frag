// Fragment shader designed for textured Point Sprites
// Discards black colour so that it efectively becpomes like a transparent
// background and does not overlay other points or objects
// Iain Martin November 2018
// Addapted from Example 6.1 in the Redbook V4.3

#version 420

in vec4 fcolour;
in float Transp;
out vec4 outputColor;

layout (binding=5) uniform sampler2D tex6;

void main()
{
	vec4 texcolour = texture(tex6, gl_PointCoord);

	/* Discard the black colours to avoid them overwritting other stars*/
	if (texcolour.r < 0.1 && texcolour.g < 0.1 && texcolour.b < 0.1) discard;

	outputColor = fcolour * texcolour;
	outputColor.a *= Transp;
}