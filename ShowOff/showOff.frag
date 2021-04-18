/*lightsCameraAction.frag
  Main Fragement Shader*/

#version 420 core

// Fog parameters, could make them uniforms and pass them into the shader
const float fog_maxdist = 5.0;
const float fog_mindist = 0.1;
//const vec4  fog_colour = vec4(0.4, 0.4, 0.4, 1.0);
 const vec4  fog_colour = vec4(0.0, 0.0, 0.0, 1.0);
const float fogDensity = 0.2; 

uniform mat4 model, view, projection;
uniform uint colourmode, emitmode;
uniform mat3 normalmatrix;
uniform vec4 lightpos;
uniform uint attenuationmode;
uniform uint fogmode;
//uniform samplercube cubemap;
uniform sampler2D tex1;
//uniform sampler2D tex2;

// global constants (for this vertex shader)
vec3 specular_albedo = vec3(1.0, 0.8, 0.6);
vec3 global_ambient = vec3(0.05, 0.05, 0.05);
int  shininess = 8;

in vec3 fposition, flightpos, fnormal, emissive, texcoords;
in vec4 fscolour;
in vec2 ftexcoord;
in float attenuation;
out vec4 outputcolor;
out vec4 fcolour;
void main()
{	
	vec3 emissive = vec3(0);
	vec4 position_h = vec4(fposition, 1.0);
	vec4 diffuse_albedo;
	vec4 texcolour = texture(tex1, ftexcoord);

	diffuse_albedo = fscolour;

	vec3 ambient = diffuse_albedo.xyz *0.2;

	// define our vectors to calculate diffuse and specular lighting
	mat4 mv_matrix = view * model;		// calculate the model-view transformation
	vec4 p = mv_matrix * position_h;	// modify the vertex position (x, y, z, w) by the model-view transformation
	vec3 n = normalize(normalmatrix * fnormal);		// modify the normals by the normal-matrix (i.e. to model-view (or eye) coordinates )
	vec3 l = flightpos - p.xyz;		// calculate the vector from the light position to the vertex in eye space
	float distancetolight = length(l);	// for attenuation
	l = normalize(l);					// normalise our light vector

	// calculate the specular component using phong specular reflection
	vec3 v = normalize(-p.xyz);	
	vec3 r = reflect(-l, n);

	// calculate the attenuation factor;
	float attenuation;
	if (attenuationmode != 1)
	{
		attenuation = 1.0;
	}
	else
	{
		// define attenuation constants. these could be uniforms for greater flexibility
		float attenuation_k1 = 0.1;
		float attenuation_k2 = 0.1;
		float attenuation_k3 = 0.1;
		attenuation = 1.0 / (attenuation_k1 + attenuation_k2*distancetolight + attenuation_k3 * pow(distancetolight, 2));
	}

	// if emitmode is 1 then we enable emmissive lighting
	if (emitmode == 1) emissive = vec3(1.0, 1.0, 1.0); 

	// calculate the diffuse component
	vec3 diffuse = max(dot(n, l), 0.0) * diffuse_albedo.xyz;
	vec3 specular = pow(max(dot(r, v), 0.0), shininess) * specular_albedo;

	//vec4 shadedColor = vec4(attenuation*(ambient + diffuse + specular) + emissive + global_ambient, 1.0); //vec4((diffuse + global_ambient), 1.0);
	//outputcolor = vec4(attenuation*(ambient + diffuse + specular) + emissive + global_ambient, 1.0);

	// Calculate linear fog
	float fog_factor;
	//outputcolor = shadedColor;
	if (fogmode == 0)
	{
		//outputcolor = vec4(attenuation*(ambient + diffuse + specular) + emissive + global_ambient, 1.0);
		outputcolor =  vec4 (ambient + diffuse, 1.0);
	}
	else 
	{
		// dist used by all fog calculations
		float dist = length(fposition.xyz);
		
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

		vec4 shadedColor = vec4(attenuation*(ambient + diffuse + specular) + emissive + global_ambient, 1.0);
		outputcolor = mix(fog_colour, shadedColor, fog_factor);
	}
}

