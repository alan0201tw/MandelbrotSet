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

				if(length(z) > 2)
				{
					return float(iteration) / maxIterations;
				}
			}
			return maxIterations;
		}

		void main()
		{
			float intensity = iterateMandebrot();

		    color = vec4(intensity, intensity, intensity, 1.0f);
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

	glfwSwapInterval(0);
	// glfwSwapInterval(1);

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
	GLint p_location = glGetUniformLocation(program, "P");

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program);

		mat4x4 p;
		mat4x4_ortho(p, -ratio + 0.2f, ratio + 0.2f, -ratio + 0.2f, ratio + 0.2f, 1.f, -1.f);
		glUniformMatrix4fv(p_location, 1, GL_FALSE, (const GLfloat*)p);
		ratio *= (ratio <= 0.05f) ? 1.0f : 0.9995f;

		glBindVertexArray(vertex_array);
		glDrawArrays(GL_QUADS, 0, 4);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
