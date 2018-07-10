// Include standard headers
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <cstring>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Include others OpenGL helper 
#include <common/shader.hpp>
#include <common/controls.hpp>


//+ complete the implementation of this function
bool load_OBJ(
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
		printf("Impossible to open the file !\n");
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
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
			//+ read texture uv data
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			//+ read normal data
			glm::vec3 normals;
			fscanf(file, "%f %f %f\n", &normals.x, &normals.y,&normals.z);
			temp_normals.push_back(normals);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}

			//+ store the index to vertexIndices, uvIndices, normalIndices array
			for (int i = 0; i < 3;  i++) {
				vertexIndices.push_back(vertexIndex[i]);
				uvIndices.push_back(uvIndex[i]);
				normalIndices.push_back(normalIndex[i]);
			}
		
		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	
	//+ Fill out_vertices, out_uvs, out_normals array
	std::cout << "vertexIndices 0 : " << temp_vertices[0].x << " "<< temp_vertices[vertexIndices[0]].y << " "<<temp_vertices[vertexIndices[0]].z << std::endl;
	std::cout << "uv 0 : " << temp_uvs[uvIndices[0]].x << " " << temp_uvs[uvIndices[0]].y  << std::endl;
	std::cout << "vertexIndices Size : " << vertexIndices.size() << std::endl;
	std::cout << "uvIndices Size : " << uvIndices.size() << std::endl;
	std::cout << "normalIndices Size : " << normalIndices.size() << std::endl;
	//out_vertices
	for (int i = 0; i < vertexIndices.size(); i++) {
		out_vertices.push_back(temp_vertices[vertexIndices[i]-1]);
	}


	//out_uvs
	for (int i = 0; i < uvIndices.size(); i++) {
		out_uvs.push_back(temp_uvs[uvIndices[i] - 1]);
	}

	//out_normals
	for (int i = 0; i < normalIndices.size(); i++) {
		out_normals.push_back(temp_normals[normalIndices[i] - 1]);
	}

	return true;
}





int main(void)
{
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Load OBJ", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.2 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// Dark background
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_CULL_FACE);

	// Create and compile our GLSL program from the shaders
	GLuint programID;
	GLuint program_wireframe = LoadShaders("simpleColor.vert", "simpleColor.frag");
	GLuint program_flatshade = LoadShaders("perpixelPhong.vert", "perpixelPhong.frag");
	programID = program_wireframe;

	//array of vec3 to store Geometry 
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	//Create VBO
	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);

	//Create+set VAO
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	// 2nd attribute buffer : normals
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// random seed
	int edgeIdx = 0;
	srand(time(NULL));

	//+ Change string to your .obj filename.  The .obj file should be in \WorkingDir
	bool res = load_OBJ("moedel.obj", vertices, uvs, normals);

	do {

		// if user press R, random new edge to do bevel
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
			edgeIdx = rand() % 12;
		}
		// switch shading mode Wireframe or Flatshade
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			programID = program_wireframe;
		}
		if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			programID = program_flatshade;
		}



		//Transfer data to VBO
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_DYNAMIC_DRAW);


		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set the shader program
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 ModelViewMatrix = ViewMatrix * ModelMatrix;
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// send uniform variable to shader
		glUniformMatrix4fv(glGetUniformLocation(programID, "MVP"), 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(programID, "ModelViewMatrix"), 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(programID, "ViewMatrix"), 1, GL_FALSE, &ViewMatrix[0][0]);

		glm::vec3 ObjColor = glm::vec3(1.0, 1.0, 0.0);
		glm::vec3 LightColor = glm::vec3(1.0, 1.0, 1.0);
		glm::vec3 lightPos = glm::vec3(20, 20, 20);
		float shiny = 100;
		glUniform1f(glGetUniformLocation(programID, "Shininess"), shiny);
		glUniform3f(glGetUniformLocation(programID, "Kd"), ObjColor.x, ObjColor.y, ObjColor.z);
		glUniform3f(glGetUniformLocation(programID, "Ld"), LightColor.x, LightColor.y, LightColor.z);
		glUniform3f(glGetUniformLocation(programID, "LightPosition"), lightPos.x, lightPos.y, lightPos.z);

		// Draw
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);


	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
