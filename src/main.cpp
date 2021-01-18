/**
 *  For image retrieval, use OpenGL's glReadPixels.
 *  Each chromesome should have one or more polygons.
 *  
 *  Pipeline :
 *      1. Read the target image, obtain its width and height and set the window's resolution accordingly
 *      2. Initialize the polygons and chromesome, then do the evolution thing
 *      3. Compute fitness and show the result of the best chromesome
 *      4. Determine the next generation of chromesome and iterate
 */

#include <iostream>
#include <algorithm>
#include <vector>

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "linmath.h"

namespace
{
	GLFWwindow* window;
	GLuint vertex_array, vertex_buffer, index_buffer, program;

	float vertices[4 * 3] =
	{
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f
	};

	const char* vertex_shader_text = R"(
		#version 330 core
	
		layout(location = 0) in vec3 a_Position;

		uniform mat4 P;

		out vec3 v_Position;
		out vec2 v_ComplexValue;

		void main()
		{
			v_Position = a_Position;
			v_ComplexValue = 2.0f * (a_Position.xy) - vec2(1.0f, 0.0f);

		    gl_Position = P * vec4(a_Position, 1.0f);
		}
	)";
	
	const char* fragment_shader_text = R"(
		#version 330 core

		precision highp float;

		layout(location = 0) out vec4 color;

		in vec3 v_Position;
		in vec2 v_ComplexValue;

		// All components are in the range [0…1], including hue.
		vec3 hsv2rgb(vec3 c)
		{
			vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
			vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
			return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
		}

		vec2 squareImaginary(vec2 number){
			return vec2(
				pow(number.x,2)-pow(number.y,2),
				2*number.x*number.y
			);
		}

		float iterateMandebrot()
		{
			const int maxIterations = 100;
			int iteration = 0;

			vec2 z = vec2(0.0f, 0.0f);

			for(iteration = 0; iteration < maxIterations; ++iteration)
			{
				z = squareImaginary(z) + v_ComplexValue;

				if(length(z) > 64)
				{
					return float(iteration - log(length(z))/log(16.0f)) / maxIterations;
				}
			}
			return 0.0f;
		}

		void main()
		{
			float intensity = iterateMandebrot();

			// vec3 rgb = vec3(intensity);
			// color = vec4(rgb, 1.0f);

			vec3 hsv = ceil(intensity) * hsv2rgb(vec3(intensity, 1.0f, 1.0f));
		    color = vec4(hsv, 1.0f);
		}
	)";
}

static void glInit()
{
	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	
	window = glfwCreateWindow(
		512, 512, "Mandelbrot Set", NULL, NULL);
	if (!window)
	{
		std::cerr << "Failed to create window!\n";
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	if (!status)
	{
		std::cerr << "Failed to initialize glad!\n";
	}

	glfwSwapInterval(1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Initialize VAO, VBO, IBO
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);

	glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, vertices, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);

	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

	unsigned int indices[6] = { 0, 1, 2, 3 };
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * 4, indices, GL_STATIC_DRAW);

	// Initialize shader
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, 0);
    glCompileShader(vertex_shader);

	GLint isCompiled = 0;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(vertex_shader, maxLength, &maxLength, &infoLog[0]);

		for (std::vector<char>::const_iterator i = infoLog.begin(); i != infoLog.end(); ++i)
    		std::cout << *i;
		std::cout << "\n";
	}
 
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, 0);
    glCompileShader(fragment_shader);

	isCompiled = 0;
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(fragment_shader, maxLength, &maxLength, &infoLog[0]);

		for (std::vector<char>::const_iterator i = infoLog.begin(); i != infoLog.end(); ++i)
    		std::cout << *i;
		std::cout << "\n";
	}
 
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

		// We don't need the program anymore.
		glDeleteProgram(program);
		
		for (std::vector<char>::const_iterator i = infoLog.begin(); i != infoLog.end(); ++i)
    		std::cout << *i;
		std::cout << "\n";
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
}

int main(int argc, char* argv[])
{
	glInit();

	float ratio = 1.0f;
	vec2 offset;
	offset[0] = offset[1] = 0.0f;
	GLint p_location = glGetUniformLocation(program, "P");

	double previousTime = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
		double currentTime = glfwGetTime();
		float timeStep = static_cast<float>(currentTime - previousTime);
		previousTime = currentTime;

		glClearColor(1.0f, 0.0f, 0.1f, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			ratio -= (ratio <= 0.0001f) ? 0.0f : timeStep * ratio;
		else if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			ratio += (ratio >= 1.0f) ? 0.0f : timeStep * ratio;

		if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			offset[1] -= timeStep * ratio;
		else if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			offset[1] += timeStep * ratio;
		if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			offset[0] += timeStep * ratio;
		else if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			offset[0] -= timeStep * ratio;

		glUseProgram(program);

		mat4x4 p;
		mat4x4_ortho(p, 
			-ratio - offset[0], ratio - offset[0], -ratio - offset[1], ratio - offset[1], 
			1.f, -1.f);
		glUniformMatrix4fv(p_location, 1, GL_FALSE, (const GLfloat*)p);

		glBindVertexArray(vertex_array);
		glDrawArrays(GL_QUADS, 0, 4);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
