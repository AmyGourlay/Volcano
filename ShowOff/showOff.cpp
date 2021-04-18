/*showOff.cpp
  Main Program File*/

/* Link to static libraries */
#ifdef _DEBUG
#pragma comment(lib, "glfw3D.lib")
#pragma comment(lib, "glloadD.lib")
#else
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glload.lib")
#endif
#pragma comment(lib, "opengl32.lib")

/* Include the header to the GLFW wrapper class which
   also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include <iostream>
#include <stack>
#include <vector>
#include <random>

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

// Include headers for our objects
#include "sphere.h"
#include "cube.h"
#include "tiny_loader.h"
#include "terrain_object.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "points2.h"

/* Include the hacked version of SOIL */
#include "soil.h"

using namespace std;
using namespace glm;

unsigned int loadCubemap(vector<string> faces);

/* Define buffer object indices */
GLuint elementbuffer;

GLuint program[5];	/* Identifier for the shader prgoram */
GLuint current_program = 0;
GLuint vao;			/* Vertex array (Containor) object. This is the index of the VAO that will be the container for
					   our buffer objects */

GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					  I've included this to show you how to pass in an unsigned integer into
					  your vertex shader. */
GLuint emitmode;
GLuint attenuationmode;
GLuint viewmode;
//GLuint rand;

/* Position and view globals */
GLfloat angle_x, angle_inc_x, x, model_scale, z, y, vx, vy, vz, anglexyz, fraction, x_inc, z_inc, boat_angle;
GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;
GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons
GLuint numlats, numlongs;	//Define the resolution of the sphere object
//GLfloat speed;				// movement increment

GLfloat light_x, light_y, light_z;

/* Uniforms*/
GLuint modelID[5], viewID[5], projectionID[5], lightposID[5], normalmatrixID[5];
GLuint colourmodeID[5], emitmodeID[5], attenuationmodeID[5], viewmodeID[5], fogmodeID[5], point_sizeID[5], randID[5];

GLint uniTime;

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/
GLuint numspherevertices;
GLuint numhemispherevertices;

GLuint fogmode;

TinyObjLoader balloon;
TinyObjLoader boat;

mat4 model;

terrain_object* heightfield;
int octaves;
GLfloat perlin_scale, perlin_frequency;
GLfloat land_size;
GLfloat sealevel = 0;

/* Point sprite object and adjustable parameters */
points2* point_anim;
GLfloat speed;
GLfloat maxdist;
GLfloat point_size;		// Used to adjust point size in the vertex shader


/* Global instances of our objects */
Sphere aSphere;
Cube aCube;

/* Define textureID*/
GLuint texID, tex2ID, tex3ID;

GLuint animationTextures[10];
int timePassed;

// Define fogmode strings to output to screen
string fog_mode_desc[] = { "off", "linear", "exp", "exp2" };

/**
*Uses stb_image.h to load an image and create a texture from it
*/
bool load_texture(const char* filename, GLuint& texID, bool bGenMipmaps, bool flip = true)
{
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texID);
	// local image parameters
	int width, height, nrChannels;

	stbi_set_flip_vertically_on_load(flip);

	/* load an image file using stb_image */
	unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);

	// check for an error during the load process
	if (data)
	{
		// Note: this is not a full check of all pixel format types, just the most common two!
		int pixel_format = 0;
		if (nrChannels == 3)
			pixel_format = GL_RGB;
		else
			pixel_format = GL_RGBA;

		// Bind the texture ID before the call to create the texture.
			// texID[i] will now be the identifier for this specific texture
		
		glBindTexture(GL_TEXTURE_2D, texID);

		// Create the texture, passing in the pointer to the loaded image pixel data
		glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, data);

		// Generate Mip Maps
		if (bGenMipmaps)
			glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		printf("stb_image  loading error: filename=%s", filename);
		return false;
	}
	stbi_image_free(data);
	return true;
}

/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper *glw)
{
	/* Set the object transformation controls to their initial values */
	speed = 0.01f;
	x = 2;
	y = 0;
	z = 1;
	x_inc = 0.01;
	z_inc = 0.01;
	vx = 0; vx = 0, vz = 0;
	light_x = 0; light_y = 0.5; light_z = 1;
	angle_x = angle_y = 0; angle_z = 0;
	angle_inc_x = 0;
	angle_inc_y = -0.2;
	angle_inc_z = 0;
	model_scale = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 0; emitmode = 0; viewmode = 0; fogmode = 0;
	attenuationmode = 1; // Attenuation is on by default
	numlats = 40;		// Number of latitudes in our sphere
	numlongs = 40;		// Number of longitudes in our sphere

	anglexyz = 0;
	fraction = 0.1f;

	boat_angle = 225.f;

	/* Define the Blending function */
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Load and create our balloon object*/
	balloon.load_obj("hotAirBalloon2.obj");
	balloon.overrideColour(vec4(0.91f, 0.76f, 0.65f, 1.f));

	/* Load and create our balloon object*/
	boat.load_obj("boat.obj");
	boat.overrideColour(vec4(1.f, 1.f, 1.f, 1.f));

	/* Create the heightfield object */
	octaves = 4;
	perlin_scale = 2.f;
	perlin_frequency = 1.f;
	land_size = 5.f;
	heightfield = new terrain_object(octaves, perlin_frequency, perlin_scale);
	heightfield->createTerrain(200, 200, land_size, land_size);
	heightfield->setColourBasedOnHeight();
	heightfield->createObject();

	speed = 0.005f;
	maxdist = 0.6f;
	point_anim = new points2(10000, maxdist, speed);
	point_anim->create();
	point_size = 50;

	/* Load and build the vertex and fragment shaders */
	try
	{
		program[0] = glw->LoadShader("showOff.vert", "showOff.frag");
		program[1] = glw->LoadShader("skyboxShader.vert", "skyboxShader.frag");
		program[2] = glw->LoadShader("terrain.vert", "terrain.frag");
		program[4] = glw->LoadShader("point_sprites.vert", "point_sprites.frag");
	}
	catch (exception &e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	for (int i = 0; i < 5; i++)
	{
		/* Define uniforms to send to vertex shader */
		modelID[i] = glGetUniformLocation(program[i], "model");
		colourmodeID[i] = glGetUniformLocation(program[i], "colourmode");
		emitmodeID[i] = glGetUniformLocation(program[i], "emitmode");
		attenuationmodeID[i] = glGetUniformLocation(program[i], "attenuationmode");
		viewID[i] = glGetUniformLocation(program[i], "view");
		projectionID[i] = glGetUniformLocation(program[i], "projection");
		lightposID[i] = glGetUniformLocation(program[i], "lightpos");
		normalmatrixID[i] = glGetUniformLocation(program[i], "normalmatrix");
		viewmodeID[i] = glGetUniformLocation(program[i], "texturemode");
		fogmodeID[i] = glGetUniformLocation(program[i], "fogmode");
		point_sizeID[i] = glGetUniformLocation(program[i], "size");
	}

	
	uniTime = glGetUniformLocation(program[2], "time");

	current_program = 0;

	/* create our sphere and cube objects */
	aSphere.makeSphere(numlats, numlongs);
	aCube.makeCube();

	glUseProgram(program[2]);

	glActiveTexture(GL_TEXTURE1);

	//Load stone texture for terrain
	if (!load_texture("Textures//stone.jpg", texID, true, false))
	{
		cout << "Fatal error loading texture: " << endl;
		exit(0);
	}

	int loc = glGetUniformLocation(program[2], "tex3");
	if (loc >= 0) glUniform1i(loc, 1);

	/* Define the texture behaviour parameters */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	glUseProgram(program[2]);

	glActiveTexture(GL_TEXTURE2);

	//Load lava texture for terrain
	if (!load_texture("Textures//Lava.png", texID, true, false))
	{
		cout << "Fatal error loading texture: " << endl;
		exit(0);
	}

	int loc3 = glGetUniformLocation(program[2], "tex2");
	if (loc3 >= 1) glUniform1i(loc3, 2);

	/* Define the texture behaviour parameters */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	glActiveTexture(GL_TEXTURE3);

	//Load sand texture for terrain
	if (!load_texture("Textures//sand2.jpg", texID, true, false))
	{
		cout << "Fatal error loading texture: " << endl;
		exit(0);
	}

	int loc4 = glGetUniformLocation(program[2], "tex4");
	if (loc4 >= 2) glUniform1i(loc4, 3);

	/* Define the texture behaviour parameters */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	glUseProgram(program[2]);

	glActiveTexture(GL_TEXTURE4);

	//Load water texture for terrain
	if (!load_texture("Textures//tex_Water.jpg", texID, true, false))
	{
		cout << "Fatal error loading texture: " << endl;
		exit(0);
	}

	int loc5 = glGetUniformLocation(program[2], "tex5");
	if (loc5 >= 3) glUniform1i(loc5, 4);

	/* Define the texture behaviour parameters */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	glUseProgram(program[4]);

	glActiveTexture(GL_TEXTURE5);

	//Load smoke texture for particle animation
	if (!load_texture("Textures//blackSmoke12.png", texID, true, false))
	{
		cout << "Fatal error loading texture: " << endl;
		exit(0);
	}

	int loc6 = glGetUniformLocation(program[4], "tex6");
	if (loc6 >= 4) glUniform1i(loc6, 5);

	/* Define the texture behaviour parameters */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);


	glUseProgram(program[1]);

	glActiveTexture(GL_TEXTURE0);

	//Code for SkyBox taken and adapted from: https://learnopengl.com/Advanced-OpenGL/Cubemaps
	vector<string> faces
	{
		"Textures//skybox_px.jpg",
		"Textures//skybox_nx.jpg",
		"Textures//skybox_py.jpg",
		"Textures//skybox_ny.jpg",
		"Textures//skybox_pz.jpg",
		"Textures//skybox_nz.jpg",
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	// This is the location of the texture object (TEXTURE0), i.e. tex1 will be the name
	// of the sampler in the fragment shader
	int loc2 = glGetUniformLocation(program[1], "cubemapTexture");
	glUniform1i(loc2, 0);

	glEnable(GL_PROGRAM_POINT_SIZE);
}

/* Called to update the display. Note that this function is called in the event loop in the wrapper
   class because we registered display as a callback function */
void display()
{
	/* Define the background colour */
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	/* Clear the colour and frame buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Enable depth test  */
	glEnable(GL_DEPTH_TEST);

	/* Enable Blending for the analytic point sprite */
	glEnable(GL_BLEND);

	/* Make the compiled shader program current */
	glUseProgram(program[current_program]);

	// Define our model transformation in a stack and 
	// push the identity matrix onto the stack
	stack<mat4> model;
	model.push(mat4(1.0f));

	// Define the normal matrix
	mat3 normalmatrix;

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);

	mat4 view;

	if (viewmode == 0) //Hot Air Balloon view mode
	{
		// Camera matrix
		view = lookAt(
			vec3(-0.03, 2.15, 3.09), // Camera is at (0,0,4), in World Space
			vec3(0, 1.5, 0), // and looks at the origin
			vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);

		// Apply rotations to the view position. This wil get appleid to the whole scene
		view = rotate(view, -radians(vx), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
		view = rotate(view, radians(180.f), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
		view = rotate(view, radians(angle_y), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
		view = rotate(view, -radians(vz), vec3(0, 0, 1));
	}
	else if (viewmode == 1) //wide view mode
	{
		// Camera matrix
		view = lookAt(
			vec3(0, 1, 0), // Camera is at (0,0,4), in World Space
			vec3(-1, 1, 0), // and looks at the origin
			vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);

		// Apply rotations to the view position. This wil get appleid to the whole scene
		view = rotate(view, -radians(vx), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
		view = rotate(view, -radians(vy), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
		view = rotate(view, -radians(vz), vec3(0, 0, 1));
	}

	//Code for SkyBox taken and adapted from: https://learnopengl.com/Advanced-OpenGL/Cubemaps
	glUseProgram(program[1]);
	glDepthMask(GL_FALSE);
	model.push(model.top());
	{

		glUniformMatrix4fv(modelID[1], 1, GL_FALSE, &(model.top()[0][0]));
		glUniformMatrix4fv(projectionID[1], 1, GL_FALSE, &projection[0][0]);
		glUniformMatrix4fv(viewID[1], 1, GL_FALSE, &view[0][0]);
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[1], 1, GL_FALSE, &normalmatrix[0][0]);

		aCube.drawCube(0);
	}
	model.pop();
	glDepthMask(GL_TRUE);

	glUseProgram(program[2]);

	model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale));//scale equally in all axis
	
	if (viewmode == 1)
	{
		model.top() = translate(model.top(), vec3(-9, 0, 0));
		model.top() = rotate(model.top(), -radians(10.f), vec3(0, 0, 1));
	}

	// Define the light position and transform by the view matrix
	vec4 lightpos = view * vec4(light_x, light_y, light_z, 1.0);

	// Send our projection and view uniforms to the currently bound shader
	// I do that here because they are the same for all objects
	for (int i = 0; i < 4; i++)
	{
		glUniform1ui(colourmodeID[i], colourmode);
		glUniform1ui(attenuationmodeID[i], attenuationmode);
		glUniformMatrix4fv(viewID[i], 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(projectionID[i], 1, GL_FALSE, &projection[0][0]);
		glUniform4fv(lightposID[i], 1, value_ptr(lightpos));
		glUniform1ui(viewmodeID[i], viewmode);
		glUniform1ui(fogmodeID[i], fogmode);
		glUniform1ui(point_sizeID[i], point_size);
	}

	glUseProgram(program[0]);

	/* Draw a small sphere in the lightsource position to visually represent the light source */
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(0, 2, -2));
		model.top() = scale(model.top(), vec3(0.05f, 0.05f, 0.05f)); // make a small sphere
																	 // Recalculate the normal matrix and send the model and normal matrices to the vertex shader																							// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																						// Recalculate the normal matrix and send to the vertex shader
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our lightposition sphere  with emit mode on*/
		emitmode = 1;
		glUniform1ui(emitmodeID[current_program], emitmode);
		//aSphere.drawSphere(drawmode);
		emitmode = 0;
		glUniform1ui(emitmodeID[current_program], emitmode);
	}
	model.pop();

	glUseProgram(program[2]);

	//Draw our terrain
	model.push(model.top());
	{
		/* Define the model transformations for our Terrain object - we don't transform it to
	   make it easier to place objects on the terrain*/
		glUniformMatrix4fv(modelID[2], 1, GL_FALSE, &(model.top()[0][0]));

		/* Draw our terrain */

		heightfield->drawObject(drawmode);
	}
	model.pop();

	glUseProgram(program[0]);

	//Draw our boat object
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(x, 0, z));
		model.top() = rotate(model.top(), radians(boat_angle), vec3(0, 1, 0));
		model.top() = scale(model.top(), vec3(0.05f, 0.05f, 0.05f));//scale equally in all axis

		// Send our uniforms variables to the currently bound shader,
		glUniformMatrix4fv(modelID[0], 1, GL_FALSE, &model.top()[0][0]);
		glUniform1ui(colourmodeID[0], colourmode);
		glUniformMatrix4fv(viewID[0], 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(projectionID[0], 1, GL_FALSE, &projection[0][0]);

		/* Draw our Blender Boat object */
		boat.drawObject(drawmode);
	}
	model.pop();

	glUseProgram(program[4]);

	//Draw our particle animation
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(0.5, 1, 1.6));
		model.top() = scale(model.top(), vec3(10.f, 10.f, 10.f));//scale equally in all axis

		glUniformMatrix4fv(modelID[4], 1, GL_FALSE, &model.top()[0][0]);
		glUniform1ui(colourmodeID[4], colourmode);
		glUniform1f(point_sizeID[4], point_size);
		glUniformMatrix4fv(viewID[4], 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(projectionID[4], 1, GL_FALSE, &projection[0][0]);

		point_anim->draw();
		point_anim->animate();
	}
	model.pop();

	//Rotate the Hot Air Balloon
	model.top() = rotate(model.top(), -radians(180.f), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model.top() = rotate(model.top(), -radians(angle_y), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis

	glUseProgram(program[0]);
	
	//Draw our Hot Air Balloon
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(0, 2, 3));
		model.top() = rotate(model.top(), -radians(90.f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.0005f, 0.0005f, 0.0005f));//scale equally in all axis

		// Send our uniforms variables to the currently bound shader,
		glUniformMatrix4fv(modelID[0], 1, GL_FALSE, &model.top()[0][0]);
		glUniform1ui(colourmodeID[0], colourmode);
		glUniformMatrix4fv(viewID[0], 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(projectionID[0], 1, GL_FALSE, &projection[0][0]);

		/* Draw our Blender Balloon object */
		balloon.drawObject(drawmode);
	}
	model.pop();

	glUseProgram(program[0]);
	
	glDisableVertexAttribArray(0);
	glUseProgram(program[current_program]);

	/* Modify our animation variables */
	angle_x += angle_inc_x;
	angle_y += angle_inc_y;
	angle_z += angle_inc_z;

	//Modify boat direction depending on position
	if (boat_angle == 225)
	{
		x -= x_inc;
		z -= z_inc;

		if (x < 0.8)
		{
			boat_angle = 45.f;
		}
	}

	if (boat_angle == 45)
	{
		x += x_inc;
		z += z_inc;

		if (x > 2.0)
		{
			boat_angle = 225.f;
		}
	}

}

//Code for SkyBox taken and adapted from: https://learnopengl.com/Advanced-OpenGL/Cubemaps
unsigned int loadCubemap(vector<string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrComponents;

	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}




/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = ((float)w / 640.f*4.f) / ((float)h / 480.f*3.f);
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{

	bool recreate_terrain = false;		// Set to true if we want to recreate the terrain
	bool placeObject = false;			// Set to true to set our object on the terrain

	/* Enable this call if you want to disable key responses to a held down key*/
	//if (action != GLFW_PRESS) return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == 'Q') angle_inc_y -= 0.05f;
	if (key == 'W') angle_inc_y += 0.05f;

	//Key to alter fog levels
	if (key == 'F' && action != GLFW_PRESS)
	{
		fogmode == 3 ? fogmode = 0 : fogmode++;
	}

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == '.' && action != GLFW_PRESS)
	{
		drawmode++;
		if (drawmode > 2) drawmode = 0;
	}

	if (key == 'V' && action != GLFW_PRESS) viewmode == 1 ? viewmode = 0 : viewmode++;

	point_anim->updateParams(maxdist, speed);
}

/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper *glw = new GLWrapper(1024, 768, "Position light example");;

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);

	/* Output the OpenGL vendor and version */
	glw->DisplayVersion();

	cout << "KEY CONTROLS" << endl;
	cout << "ESC - Close Window" << endl;
	cout << "" << endl;
	cout << "HOT AIR BALLOON CONTROLS" << endl;
	cout << "Q - Rotate Anti-Clockwise" << endl;
	cout << "W - Rotate Clockwise" << endl;
	cout << "" << endl;
	cout << "VIEW CONTROLS" << endl;
	cout << "V - Change View Position" << endl;
	cout << "" << endl;
	cout << "FOG MODE CONTROLS" << endl;
	cout << "F - Changing the Fog Settings" << endl;
	cout << "" << endl;

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}

