//// Minimal fragment shader
//
//#version 400
//
//in vec4 fcolour;
//out vec4 outputColor;
//void main()
//{
//	outputColor = fcolour;
//}

// Minimal fragment shader with added fog.
// The fog has been implemented as no fog, linear, exp and exp2
// controlled by a uniform defined in the application

#version 420

in vec4 texcoords, lavaTexCoords, diffuse_colour, fposition_h;
float fdistance;
in vec2 ftexcoord;
in vec2 flavatexcoord;
in vec3 fposition, fnormal;
out vec4 outputColor;
vec4 fcolour;

// Fog parameters, could make them uniforms and pass them into the shader
float fog_maxdist = 4.0;
float fog_mindist = 0.1;
vec4  fog_colour = vec4(0.4, 0.4, 0.4, 1.0);
const float fogDensity = 0.2;

layout (binding=1) uniform sampler2D tex3;
layout (binding=2) uniform sampler2D tex2;
layout (binding=3) uniform sampler2D tex4;
layout (binding=4) uniform sampler2D tex5;

uniform float time;
uniform mat4 model, view, projection;
uniform uint colourmode, emitmode;
uniform mat3 normalmatrix;
uniform vec4 lightpos;
uniform uint attenuationmode;
uniform uint fogmode;

vec4 ambient = vec4(0.2, 0.2, 0.2, 1.0);
vec3 light_dir = vec3(0.0, 0.0, 10.0);

void main()
{
	vec4 fdiffuse_colour;

	vec4 texcolour;
	if (ftexcoord.y > -0.15 && ftexcoord.y < 10)
	{
		texcolour = texture(tex2, ftexcoord);
		fdiffuse_colour = vec4(1.0, 1.0, 1.0, 1.0) * texcolour;
	}
	else if (ftexcoord.y <= -0.15 && ftexcoord.y > -2.4)
	{
		texcolour = texture(tex3, ftexcoord);
		fdiffuse_colour = vec4(0.847, 0.847, 0.75, 1.0) * texcolour;
	}
	else if (ftexcoord.y <= -2.4 && ftexcoord.y > -3.1)
	{
		texcolour = texture(tex4, ftexcoord);
		fdiffuse_colour = vec4(0.847, 0.847, 0.75, 1.0) * texcolour;
	}
	else if (ftexcoord.y <= -3.1 && ftexcoord.y > -10)
	{
		texcolour = texture(tex5, ftexcoord);
		fdiffuse_colour = vec4(0.33, 0.33, 0.33, 0.3) * texcolour;
	}
	else
	{
		texcolour = texture(tex5, ftexcoord);
		fdiffuse_colour = vec4(0.9, 0.8, 0.9, 1.0) * texcolour;
	}

	ambient = diffuse_colour * 0.2;

	vec4 specular_colour = vec4(0.0,0.0,0.0,1.0);
	float shininess = 8.0;

	mat4 mv_matrix = view * model;
	mat3 normalmatrix = mat3(mv_matrix);
	vec3 N = mat3(mv_matrix) * fnormal;
	N = normalize(N);
	light_dir = normalize(light_dir);

	vec3 diffuse = max(dot(N, light_dir), 0.0) * diffuse_colour.xyz;

	vec4 P = fposition_h * mv_matrix;
	vec3 half_vec = normalize(light_dir + P.xyz);
	vec4 specular = pow(max(dot(N, half_vec), 0.0), shininess) * specular_colour;

	fdistance = length(P.xyz);

	// Define the vertex colour
	fcolour = vec4(diffuse,1.0) + ambient + specular;// Set colours based on height in vertex terrain

	outputColor = fcolour * texcolour;

	float fog_factor;
	if (fogmode == 0)
	{
		// No fog
		outputColor = fcolour * texcolour;
	}
	else 
	{
		// dist used by all fog calculations
		float dist = abs(fdistance);
		
		if (fogmode == 1){
			// linear fog
			fog_factor = (fog_maxdist-dist) / (fog_maxdist - fog_mindist);
		}
		else if (fogmode == 2) {
			// exponential fog
			fog_factor = 1.0 / exp(dist  * fogDensity);
		}
		else{
			// exponential squared fog
			fog_factor = 1.0 / pow(exp(dist  * fogDensity), 2.0);
		}

		// Limit fog factor to range 0 to 1
		fog_factor = clamp(fog_factor, 0.0, 1.0);
		outputColor = mix(fog_colour, fcolour, fog_factor) * texcolour;
	}
}