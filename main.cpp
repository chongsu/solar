/*********************************************************
FILE : main.cpp (csci3260 2017-2018 Final Project)
*********************************************************/
/*********************************************************
Student Information
Student ID:  1155077005
Student Name:SU, Chong
*********************************************************/

#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"
#include "Dependencies\glm\glm.hpp"
#include "Dependencies\glm\gtc\matrix_transform.hpp"

#include<stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include <chrono>
using namespace std;
using glm::vec3;
using glm::mat4;
using glm::perspective;
using glm::translate;
using glm::rotate;
using glm::scale;
clock_t seconds = clock();
GLint programID;
GLint Skybox_programID;
GLint shadowMapProgramID;
GLuint FramebufferName=0;
GLuint depthTexture;
struct skybox {
	GLuint skyboxVAO;
	GLuint skyboxVBO;
	GLuint cubemapTextureID;
} skyboxA;
struct skyboxCamera {
	int x;
	int y;
	int z;
} skbCamera;
struct worldcamera {
	float x = 0.0001f;
	float y = 0;
	float z = 30.0f;
	float radius = 30.0f;
	float initViewHorizontal = -90.0f;
	float initViewVertical = -90.0f;
	float viewRotateDegree[3] = { 0.0f, 0.0f, 0.0f };
} worldCameraA;
struct mouseControl {
	bool mouseControlOn = true;
	bool yChangeUP = false;
	bool zChangeLEFT = false;
	double y_projectionChangeSpeed = 1.0;
	double z_projectionChangeSpeed = 1.0;
} mouseControlObj;

int rotation_status = 0;


float x_mouse = 0.0f;  //control the position of camera
float y_mouse = 0.0f;
float x_origin = 0.0f;
float y_origin = 0.0f;
float x_plane = 0.0f;
float z_plane = 0.0f;

float diffuse_delta = 1.0f;
float specular_delta = 0.3f;


GLuint VertexArrayID[6]; // Define the Vao&Vbo and interaction parameter
GLuint vertexbuffer[6];
GLuint uvbuffer[6];
GLuint normalbuffer[6];
GLuint TextureBuffer[6];
GLuint texture[6];
GLuint cube_vbo_idx;
bool res[6];
int drawSize[6];
GLuint normal_texture;






bool checkStatus(
	GLuint objectID,
	PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
	PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
	GLenum statusType)
{
	GLint status;
	objectPropertyGetterFunc(objectID, statusType, &status);
	if (status != GL_TRUE)
	{
		GLint infoLogLength;
		objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* buffer = new GLchar[infoLogLength];

		GLsizei bufferSize;
		getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
		cout << buffer << endl;

		delete[] buffer;
		return false;
	}
	return true;
}

bool checkShaderStatus(GLuint shaderID)
{
	return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}
bool checkProgramStatus(GLuint programID)
{
	return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}
string readShaderCode(const char* fileName)
{
	ifstream meInput(fileName);
	if (!meInput.good())
	{
		cout << "File failed to load..." << fileName;
		exit(1);
	}
	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>()
	);
}
void installShaders()
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];

	string temp = readShaderCode("VertexShaderCode.glsl");
    adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	temp = readShaderCode("FragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
		return;

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	if (!checkProgramStatus(programID))
		return;

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	glUseProgram(programID);
}
void installShadowMapShaders() {

	GLuint vertexShaderID= glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID= glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	string temp = readShaderCode("ShadowVertexShaderCode.glsl");
	//new vertex shaderfile, should also create new shaderfile for fragment.
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);

	temp = readShaderCode("ShadowFragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
		return;
	//…
	shadowMapProgramID = glCreateProgram(); //new program ID
	glAttachShader(shadowMapProgramID, vertexShaderID);
	glAttachShader(shadowMapProgramID, fragmentShaderID);
	glLinkProgram(shadowMapProgramID);
	if (!checkProgramStatus(shadowMapProgramID))
		return;
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	glUseProgram(shadowMapProgramID);
}
void installskyboxShaders()
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];

	string temp = readShaderCode("skyboxVertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	temp = readShaderCode("skyboxFragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
		return;

	Skybox_programID = glCreateProgram();
	glAttachShader(Skybox_programID, vertexShaderID);
	glAttachShader(Skybox_programID, fragmentShaderID);
	glLinkProgram(Skybox_programID);

	if (!checkProgramStatus(Skybox_programID))
		return;

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);
	printf("skybox shader installed\n");

	glUseProgram(Skybox_programID);
}
void keyboard(unsigned char key, int x, int y)
{
	if (key == 's') {
		rotation_status++;
		rotation_status = rotation_status % 2;
		if (rotation_status == 0) printf("Jeep rotation starts.\n");
		else printf("Jeep rotation ends.\n");
    }
	if (key == ' ') {
		mouseControlObj.mouseControlOn = !mouseControlObj.mouseControlOn;
		if (!mouseControlObj.mouseControlOn) printf("Camera motion enabled.\n");
		else printf("Camera motion disabled.\n");
	}
	if (key == 'q' ) {
		if (diffuse_delta < 5.0f) {
			diffuse_delta ++;
			printf("Diffuse light increased.\n");
		}
		else {
			printf("Diffuse light reached maximal.\n");
		}
	
	}
	if (key == 'w') {
		if(diffuse_delta > 0.0f){
			diffuse_delta -= 0.5f;
			printf("Diffuse light decreased.\n");
		}
		else {
			printf("Diffuse light reached minimal.\n");
		}
	
	}
	if (key == 'z' ) {
		if(specular_delta < 100.0f ){
		    specular_delta +=0.01f;
		    printf("Specular light increased.\n");
		}
		else {
			printf("Specular light reached maximal.\n");
		}
	}
	if (key == 'x' ) {
		if(specular_delta > 0.0f){
			specular_delta -=0.01f;
			printf("Specular light decreased.\n");
		}
		else {
			printf("Specular light reached minimal.\n");
		}
		
	}
}
void Mouse_Wheel_Func(int button, int state, int x, int y)
{
	if ((button == 3) || (button == 4))
	{
		if (state == GLUT_UP) return;
		if (button == 3)
		{
			worldCameraA.radius -= 1.0f;
		}
		else
		{
			worldCameraA.radius += 1.0f;

		}
	}
}

void move(int key, int x, int y) 
{ 
	if (key == GLUT_KEY_UP) {
	     z_plane+=10;
	
	}
	if (key == GLUT_KEY_DOWN) {
		 z_plane-=10;
	}
		
	
	if (key == GLUT_KEY_LEFT) {
		 x_plane+=0.2;
		
	}
	if (key == GLUT_KEY_RIGHT) {
	    x_plane-=0.2;
	
	}

}

//TODO: Use Mouse to do interactive events and animation
void PassiveMouse(int x, int y)
{   
	if (mouseControlObj.mouseControlOn) {
		if (y > 400) {
			mouseControlObj.yChangeUP = true;
			mouseControlObj.y_projectionChangeSpeed = (y - 400.0) / 400.0;
		}
		else if (y < 400) {
			mouseControlObj.yChangeUP = false;
			mouseControlObj.y_projectionChangeSpeed = (400.0 - y) / 400.0;
		}
		if (x > 500) {
			mouseControlObj.zChangeLEFT = true;
			mouseControlObj.z_projectionChangeSpeed = (x - 500.0) / 500.0;
		}
		else {
			mouseControlObj.zChangeLEFT = false;
			mouseControlObj.z_projectionChangeSpeed = (500.0 - x) / 500.0;
		}
	}

}

bool loadOBJ(
	const char * path,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

				   // else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i<vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}

	return true;
}
GLuint loadBMP_custom(const char * imagepath) {

	printf("Reading image %s\n", imagepath);

	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	unsigned char * data;

	FILE * file = fopen(imagepath, "rb");
	if (!file) { printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); return 0; }

	if (fread(header, 1, 54, file) != 54) {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (*(int*)&(header[0x1E]) != 0) { printf("Not a correct BMP file\n");    return 0; }
	if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");    return 0; }

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);
	if (imageSize == 0)    imageSize = width*height * 3;
	if (dataPos == 0)      dataPos = 54;

	data = new unsigned char[imageSize];
	fread(data, 1, imageSize, file);
	fclose(file);


	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	return textureID;
}
GLuint loadBMP_data(const char * imagepath, unsigned char *& img_data, int & img_w, int & img_h)
{
	printf("Reading image %s\n", imagepath);
	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;

	// Open the file
	FILE * file = fopen(imagepath, "rb");
	if (!file)
	{
		printf("%s could not be opened! \n", imagepath);
		getchar();
		return 0;
	}
	// Read the header, i.e. the 54 first bytes
	// If less than 54 bytes are read, problem
	if (fread(header, 1, 54, file) != 54)
	{
		printf("Not a correct BMP file\n");
		return 0;
	}
	// A BMP files always begins with "BM"
	if (header[0] != 'B' || header[1] != 'M')
	{
		printf("Not a correct BMP file\n");
		return 0;
	}
	// Make sure this is a 24bpp file
	if (*(int*)&(header[0x1E]) != 0) { printf("Not a correct BMP file\n");    return 0; }
	if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");    return 0; }

	// Read the information about the image
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	img_w = *(int*)&(header[0x12]);
	img_h = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0)    imageSize = img_w*img_h * 3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos == 0)      dataPos = 54; // The BMP header is done that way
										 // Create a buffer
	img_data = new unsigned char[imageSize];
	// Read the actual data from the file into the buffer
	fread(img_data, 1, imageSize, file);
	// Everything is in memory now, the file can be closed
	fclose(file);

}
GLuint loadCubemap(vector<const GLchar*> faces)
{
	printf("Start loading skybox textures\n");
	int width, height;
	unsigned char* image;
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		loadBMP_data(faces[i], image, width, height);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
			0, GL_RGB, GL_UNSIGNED_BYTE, image);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return textureID;
}
void sendDataToOpenGL()
{
	glUseProgram(Skybox_programID);

	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};
	
	glGenVertexArrays(1, &skyboxA.skyboxVAO);
	glGenBuffers(1, &skyboxA.skyboxVBO);
	glBindVertexArray(skyboxA.skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxA.skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);

	vector<const GLchar*>earth_faces;
	
		earth_faces.push_back("texture/sea_skybox/right.bmp");
		earth_faces.push_back("texture/sea_skybox/left.bmp");
		earth_faces.push_back("texture/sea_skybox/bottom.bmp");
		earth_faces.push_back("texture/sea_skybox/top.bmp");
		earth_faces.push_back("texture/sea_skybox/back.bmp");
		earth_faces.push_back("texture/sea_skybox/front.bmp");
	
	skyboxA.cubemapTextureID = loadCubemap(earth_faces);

	glUseProgram(programID);

	//object0: earth (A)
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	//object1: the space vehicle (D)
	std::vector<glm::vec3> vertices1;
	std::vector<glm::vec2> uvs1;
	std::vector<glm::vec3> normals1;
	//object2: glass planet (C)
	std::vector<glm::vec3> vertices2;
	std::vector<glm::vec2> uvs2;
	std::vector<glm::vec3> normals2;
	//object3: moon (B)
	std::vector<glm::vec3> vertices3;
	std::vector<glm::vec2> uvs3;
	std::vector<glm::vec3> normals3;
	//object4: lightbox (E)
	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f,-1.0f,-1.0f, // triangle 1 : begin
		-1.0f,-1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f, // triangle 1 : end
		1.0f, 1.0f,-1.0f, // triangle 2 : begin
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f, // triangle 2 : end
		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f
	};
	
	glGenVertexArrays(1, &VertexArrayID[4]);
	glBindVertexArray(VertexArrayID[4]);
	glGenBuffers(1, &vertexbuffer[4]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[4]);
	glVertexAttribPointer(0, 3,	GL_FLOAT,	GL_FALSE,	0,	(void*)0);


	//object0: earth (A)
	glGenVertexArrays(1, &VertexArrayID[0]);
	glBindVertexArray(VertexArrayID[0]);
	res[0] = loadOBJ("obj/planet.obj", vertices, uvs, normals);
	texture[0] = loadBMP_custom("texture/earth.bmp");

	normal_texture = loadBMP_custom("normal_map/earth_normal.bmp");

	glGenBuffers(1, &vertexbuffer[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), 
		&vertices[0], GL_STATIC_DRAW);
	glGenBuffers(1, &uvbuffer[0]);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2),
		&uvs[0], GL_STATIC_DRAW);
	glGenBuffers(1, &normalbuffer[0]);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3),
		&normals[0], GL_STATIC_DRAW);
	drawSize[0] = vertices.size();

	
	glEnableVertexAttribArray(0);  // 1rst attribute buffer : vertices
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		0, (void*)0);
	glEnableVertexAttribArray(1);	// 2nd attribute buffer : UVs
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[0]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
		0, (void*)0);
	glEnableVertexAttribArray(2); //3rd normal buffer: normals
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer[0]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
		0, (void*)0);
	
	//object1: the space vehcle (D)
	glGenVertexArrays(1, &VertexArrayID[1]);
	glBindVertexArray(VertexArrayID[1]);
	res[1] = loadOBJ("obj/Arc170.obj", vertices1, uvs1, normals1);
	texture[1] = loadBMP_custom("texture/helicopter.bmp");
	glGenBuffers(1, &vertexbuffer[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, vertices1.size() * sizeof(glm::vec3),
		&vertices1[0], GL_STATIC_DRAW);
	glGenBuffers(1, &uvbuffer[1]);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, uvs1.size() * sizeof(glm::vec2),
		&uvs1[0], GL_STATIC_DRAW);
	glGenBuffers(1, &normalbuffer[1]);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, normals1.size() * sizeof(glm::vec3),
		&normals1[0], GL_STATIC_DRAW);
	drawSize[1] = vertices1.size();
	glEnableVertexAttribArray(0);  // 1rst attribute buffer : vertices
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		0, (void*)0);
	glEnableVertexAttribArray(1); // 2nd attribute buffer : UVs
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
		0, (void*)0);
	glEnableVertexAttribArray(2);  //3rd normal buffer: normals
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer[1]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
		0, (void*)0);
	
	//object2: glass planet (C)
	glGenVertexArrays(1, &VertexArrayID[2]);
	glBindVertexArray(VertexArrayID[2]);
	res[2] = loadOBJ("obj/planet.obj", vertices2, uvs2, normals2);
	texture[2] = loadBMP_custom("texture/glass_a.bmp");
	texture[4] = loadBMP_custom("texture/polygon_tex.bmp");
	glGenBuffers(1, &vertexbuffer[2]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[2]);
	glBufferData(GL_ARRAY_BUFFER, vertices2.size() * sizeof(glm::vec3),
		&vertices2[0], GL_STATIC_DRAW);
	glGenBuffers(1, &uvbuffer[2]);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[2]);
	glBufferData(GL_ARRAY_BUFFER, uvs2.size() * sizeof(glm::vec2),
		&uvs2[0], GL_STATIC_DRAW);
	glGenBuffers(1, &normalbuffer[2]);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer[2]);
	glBufferData(GL_ARRAY_BUFFER, normals2.size() * sizeof(glm::vec3),
		&normals2[0], GL_STATIC_DRAW);
	drawSize[2] = vertices2.size();
	glEnableVertexAttribArray(0);   // 1st attribute buffer : vertices
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[2]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		0, (void*)0);
	glEnableVertexAttribArray(1);   // 2nd attribute buffer : UVs
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[2]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
		0, (void*)0);
	glEnableVertexAttribArray(2);   //3rd normal buffer: normals
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
		0, (void*)0);

	//object3: moon (B)
	glGenVertexArrays(1, &VertexArrayID[3]);
	glBindVertexArray(VertexArrayID[3]);
	res[3] = loadOBJ("obj/planet.obj", vertices3, uvs3, normals3);
	texture[3] = loadBMP_custom("texture/apple.bmp");
	glGenBuffers(1, &vertexbuffer[3]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[3]);
	glBufferData(GL_ARRAY_BUFFER, vertices3.size() * sizeof(glm::vec3),
		&vertices3[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvbuffer[3]);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[3]);
	glBufferData(GL_ARRAY_BUFFER, uvs3.size() * sizeof(glm::vec2),
		&uvs3[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalbuffer[3]);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer[3]);
	glBufferData(GL_ARRAY_BUFFER, normals3.size() * sizeof(glm::vec3),
		&normals3[0], GL_STATIC_DRAW);

	drawSize[3] = vertices3.size();

	glEnableVertexAttribArray(0);  // 1rst attribute buffer : vertices
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[3]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		0, (void*)0);
	glEnableVertexAttribArray(1);	// 2nd attribute buffer : UVs
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[3]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
		0, (void*)0);
	glEnableVertexAttribArray(2); //3rd normal buffer: normals
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer[3]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
		0, (void*)0);



	glUseProgram(shadowMapProgramID);
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);
	glDrawBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "FrameBufferdoes not complete!" << endl;
	}
}
void renderSkyBox(void) {
	glDepthMask(GL_FALSE);
	glUseProgram(Skybox_programID);

	GLuint Skb_ModelUniformLocation = glGetUniformLocation(Skybox_programID, "M");
	mat4 Skb_ModelMatrix = mat4(1.0f);
	glUniformMatrix4fv(Skb_ModelUniformLocation, 1, GL_FALSE, &Skb_ModelMatrix[0][0]);

	// becuase the translation and zoom should not affect skybox
	if (worldCameraA.z > 0) {
		skbCamera.z = 1;
	}
	else if (worldCameraA.z < 0) {
		skbCamera.z = -1;
	}
	float kx = worldCameraA.x ;
	float ky = worldCameraA.y ;
	mat4 skbview = glm::lookAt(
			vec3(kx / worldCameraA.radius, ky/ worldCameraA.radius , skbCamera.z ),     // Camera is at (4,3,-3), in World Space
			vec3(0, 0, 0),                        // and looks at the origin
			vec3(0, -1, 0));                      // Head is up (set to 0,-1,0 to look upside-down)
	mat4 skbprojection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 50.0f);

	glUniformMatrix4fv(glGetUniformLocation(Skybox_programID, "view"), 1, GL_FALSE, &skbview[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(Skybox_programID, "projection"), 1, GL_FALSE, &skbprojection[0][0]);


	glBindVertexArray(skyboxA.skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(Skybox_programID, "skybox"), 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxA.cubemapTextureID);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}
void ProjectionLoop() {
	worldCameraA.x = worldCameraA.radius* cos(glm::radians(worldCameraA.initViewHorizontal + worldCameraA.viewRotateDegree[1]))*sin(glm::radians(worldCameraA.initViewVertical + worldCameraA.viewRotateDegree[0]));
	worldCameraA.y = worldCameraA.radius* cos(glm::radians(worldCameraA.initViewVertical + worldCameraA.viewRotateDegree[0]));
	worldCameraA.z = worldCameraA.radius* sin(glm::radians(worldCameraA.initViewHorizontal + worldCameraA.viewRotateDegree[1]))*sin(glm::radians(worldCameraA.initViewVertical + worldCameraA.viewRotateDegree[0]));
	if (mouseControlObj.mouseControlOn && (seconds % 30) == 1) {
		printf("worldCameraAy = %f worldCameraAx = %f radius = %f \n", worldCameraA.y, worldCameraA.x, worldCameraA.radius);
		if (mouseControlObj.yChangeUP) {
			worldCameraA.viewRotateDegree[0] = worldCameraA.viewRotateDegree[0] - 1.0f;
		}
		else { 
			worldCameraA.viewRotateDegree[0] = worldCameraA.viewRotateDegree[0] + 1.0f;
		}
		if (mouseControlObj.zChangeLEFT) {
			worldCameraA.viewRotateDegree[1] = worldCameraA.viewRotateDegree[1] - 1.0f;
		}
		else {
			worldCameraA.viewRotateDegree[1] = worldCameraA.viewRotateDegree[1] + 1.0f;
		}
	}

}
void paintGL(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	srand(seconds);
	seconds = clock();
	//draw skybox
	renderSkyBox();
	glDepthMask(GL_TRUE);
	ProjectionLoop();
	// earthA matrix 
	mat4 modelTransformMatrix_earthA = mat4(1.0f);
	modelTransformMatrix_earthA = glm::scale(modelTransformMatrix_earthA, vec3(2.0f, 2.0f, 2.0f));
	modelTransformMatrix_earthA = glm::translate(modelTransformMatrix_earthA, vec3(6.0f, 0.0f, -2.0f));
	modelTransformMatrix_earthA = rotate(modelTransformMatrix_earthA, -(float)clock() / (float)CLOCKS_PER_SEC *1.0f, glm::vec3(0.0f, 0.000001f, 0.0f));

	// airplane matrixs

	mat4 modelTransformMatrix_airplane = glm::scale(mat4(1.0f), vec3(0.005f, 0.005f, 0.005f));
	modelTransformMatrix_airplane = glm::translate(modelTransformMatrix_airplane, vec3(6.0f, 0.0f, -2.0f));
	modelTransformMatrix_airplane = rotate(modelTransformMatrix_airplane, (float)clock() / (float)CLOCKS_PER_SEC *1.0f, glm::vec3(0.0f, 1, 0.0f));

	// planetC matrixs 
	mat4 modelTransformMatrix_planetC = mat4(1.0f);
	modelTransformMatrix_planetC = glm::translate(modelTransformMatrix_planetC, vec3(-5.0f, 5.0f, -10.0f));
	modelTransformMatrix_planetC = rotate(modelTransformMatrix_planetC, (float)clock() / (float)CLOCKS_PER_SEC *1.0f, glm::vec3(0.0f, 0.000001f, 0.0f));
	
	//planetB matrix
	mat4 modelTransformMatrix_planetB = scale(mat4(1.0f), vec3(0.6f, 0.6f, 0.6f)); 
	modelTransformMatrix_planetB = translate(modelTransformMatrix_planetB, vec3(22.0f, 0.0f, -2.0f));
	modelTransformMatrix_planetB = translate(modelTransformMatrix_planetB, vec3(20*cos((float)clock() / (float)CLOCKS_PER_SEC),0, 20 * sin((float)clock() / (float)CLOCKS_PER_SEC)));
	modelTransformMatrix_planetB = rotate(modelTransformMatrix_planetB, -(float)clock() / (float)CLOCKS_PER_SEC *1.0f, glm::vec3(0.0f, 0.000001f, 0.0f));
	
	//lightbox matrix 
	mat4 modelTransformMatrix_lightbox = translate(mat4(1.0f),
		vec3(10 * cos((float)clock() / (float)CLOCKS_PER_SEC), 10 * sin((float)clock() / (float)CLOCKS_PER_SEC), 0));
	
	//--------------------------------------------------------------------
	glUseProgram(shadowMapProgramID);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glViewport(0, 0, 1024, 1024);

	glm::vec3 lightInvDir = glm::vec3(0.5f, 2, 2);
	glm::mat4 depthProjectionMatrix =
		glm::ortho<float>(-30, 30, -30, 30, 0, 40);
	glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(x_mouse, 0, y_mouse), glm::vec3(0, 1, 0));
	
	glm::mat4 depthunBiasMVP_jeep = depthProjectionMatrix* depthViewMatrix* modelTransformMatrix_earthA;
	glm::mat4 depthunBiasMVP_plane = depthProjectionMatrix* depthViewMatrix* modelTransformMatrix_planetC;
	glm::mat4 depthunBiasMVP_airplane = depthProjectionMatrix* depthViewMatrix* modelTransformMatrix_airplane;
	glm::mat4 depthunBiasMVP_heli = depthProjectionMatrix* depthViewMatrix* modelTransformMatrix_planetB;
	glm::mat4 biasMatrix(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);
	glm::mat4 depthMVP_jeep = biasMatrix *depthunBiasMVP_jeep;
	glm::mat4 depthMVP_airplane = biasMatrix *depthunBiasMVP_airplane;
	glm::mat4 depthMVP_plane = biasMatrix *depthunBiasMVP_plane;
	glm::mat4 depthMVP_heli = biasMatrix *depthunBiasMVP_heli;
	GLuint depthMatrixLocation = glGetUniformLocation(shadowMapProgramID, "depthMVP");


	glBindVertexArray(VertexArrayID[0]); //bind model information
	glUniformMatrix4fv(depthMatrixLocation, 1, GL_FALSE, &depthunBiasMVP_jeep[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[0]);

	//render the shadow map for airplane
	glBindVertexArray(VertexArrayID[1]); //bind model information
	glUniformMatrix4fv(depthMatrixLocation, 1, GL_FALSE, &depthunBiasMVP_airplane[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[1]);

	glBindVertexArray(VertexArrayID[2]); //bind model information
	glUniformMatrix4fv(depthMatrixLocation, 1, GL_FALSE, &depthunBiasMVP_plane[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[2]);

	glBindVertexArray(VertexArrayID[3]); //bind model information
	glUniformMatrix4fv(depthMatrixLocation, 1, GL_FALSE, &depthunBiasMVP_heli[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[3]);

	//--------------------------------------------------------------
	glUseProgram(programID);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glViewport(0, 0,512, 512);

	GLint ambientLightUniformLocation = glGetUniformLocation(programID, "ambientLight");
	vec3 ambientLight(0.5f, 0.5f, 0.5f);
	GLint eyePositionUniformLocation = glGetUniformLocation(programID, "eyePositionWorld");
	vec3 eyePosition(x_mouse, 30, -20 + y_mouse);
	GLint lightPositionUniformLocation = glGetUniformLocation(programID, "lightPositionWorld");
	vec3 lightPosition = vec3(10 * cos((float)clock() / (float)CLOCKS_PER_SEC), 10 * sin((float)clock() / (float)CLOCKS_PER_SEC), 0);

	GLint modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
	GLint uniProj = glGetUniformLocation(programID, "projection");
	GLint uniView = glGetUniformLocation(programID, "view");

	GLint diffuseUniformLocation = glGetUniformLocation(programID, "diffusefactor");
	float diffusefactor = diffuse_delta;
	GLint specularUniformLocation = glGetUniformLocation(programID, "specularfactor");
	float specularfactor = specular_delta;
	
	glm::mat4 view;
	glm::mat4 projection;
	view = glm::lookAt(glm::vec3(worldCameraA.x, worldCameraA.y, worldCameraA.z), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	// projection matrix
	projection = glm::perspective(45.0f, 1.0f, 0.1f, 200.0f);


	glUniformMatrix4fv(uniView, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, &projection[0][0]);
	glUniform1f(diffuseUniformLocation, diffusefactor);
	glUniform1f(specularUniformLocation, specularfactor);
	glUniform3fv(ambientLightUniformLocation, 1, &ambientLight[0]);
	glUniform3fv(lightPositionUniformLocation, 1, &lightPosition[0]);
	glUniform3fv(eyePositionUniformLocation, 1, &eyePosition[0]);

	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	GLuint TextureID_2 = glGetUniformLocation(programID, "myTextureSampler_2");

	GLuint NormalTextureID = glGetUniformLocation(programID, "myTextureSampler_3");

	GLuint shadowMapID = glGetUniformLocation(programID, "shadowMap");
	depthMatrixLocation = glGetUniformLocation(programID, "depthMVP");

	GLuint Normal_FlagID = glGetUniformLocation(programID, "normalMapping_flag");
	int normalMapping_flag = 0;

	glBindVertexArray(VertexArrayID[0]);
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1,GL_FALSE, &modelTransformMatrix_earthA[0][0]);
	glUniformMatrix4fv(depthMatrixLocation, 1, GL_FALSE, &depthMVP_jeep[0][0]);
	normalMapping_flag = 1;
	glUniform1i(Normal_FlagID, normalMapping_flag);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glUniform1i(TextureID, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glUniform1i(shadowMapID, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glUniform1i(TextureID_2, 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, normal_texture);
	glUniform1i(NormalTextureID, 3);

	glDrawArrays(GL_TRIANGLES, 0, drawSize[0]);



	glBindVertexArray(VertexArrayID[1]);
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix_airplane[0][0]);
	glUniformMatrix4fv(depthMatrixLocation, 1, GL_FALSE, &depthMVP_airplane[0][0]);
	normalMapping_flag = 0;
	glUniform1i(Normal_FlagID, normalMapping_flag);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glUniform1i(TextureID, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glUniform1i(shadowMapID, 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glUniform1i(TextureID_2, 2);


	glDrawArrays(GL_TRIANGLES, 0, drawSize[1]);

	

	glBindVertexArray(VertexArrayID[2]);
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1,GL_FALSE, &modelTransformMatrix_planetC[0][0]);
	glUniformMatrix4fv(depthMatrixLocation, 1, GL_FALSE, &depthMVP_plane[0][0]);
	normalMapping_flag = 0;
	glUniform1i(Normal_FlagID, normalMapping_flag);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glUniform1i(TextureID, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glUniform1i(shadowMapID, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture[4]);
	glUniform1i(TextureID_2, 2);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[2]);
	

	glBindVertexArray(VertexArrayID[3]);
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix_planetB[0][0]);
	glUniformMatrix4fv(depthMatrixLocation, 1, GL_FALSE, &depthMVP_heli[0][0]);
	normalMapping_flag = 0;
	glUniform1i(Normal_FlagID, normalMapping_flag);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glUniform1i(TextureID, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glUniform1i(shadowMapID, 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glUniform1i(TextureID_2, 2);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[3]);

	glBindVertexArray(VertexArrayID[4]);
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix_lightbox[0][0]);
	normalMapping_flag = 0;
	glUniform1i(Normal_FlagID, normalMapping_flag);
	glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
	glFlush();
	glutPostRedisplay();
}
void initializedGL(void)
{
	glewInit();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	installShaders();
	installskyboxShaders();
	installShadowMapShaders();
	sendDataToOpenGL();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(700, 700);
	glutCreateWindow("i-Navigation");

	initializedGL();
	glutDisplayFunc(paintGL);
	glutMouseFunc(Mouse_Wheel_Func);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(move);
	glutPassiveMotionFunc(PassiveMouse);

	glutMainLoop();

	return 0;
}