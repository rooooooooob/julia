#include <glad/glad.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>

static struct
{
	float x, y;
	float r, g, b;
} vertices[3] =
{
	{ -10.f, 10.f, 0.f, 0.f, 0.f },
	{ -10.f, -20.f, 0.f, 0.f, 0.f },
	{ 20.f, 10.f, 0.f, 0.f, 0.f }
};
static const char* vertex_shader_text =
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec2 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";
// TODO: don't hard-code dimensions
static const char* fragment_shader_text =
"varying vec3 color;\n"
"bool is_nan( float val )\n"
"{\n"
"    return (val < 0.0 || 0.0 < val || val == 0.0) ? false : true;\n"
"}\n"
"void main()\n"
"{\n"
"    vec2 tl = vec2(-2.f, -1.5f);\n"
"    vec2 br = vec2(-0.75f, 1.5f);\n"
"    vec2 wh = br - tl;\n"
"    vec2 c = (gl_FragCoord.xy / vec2(640, 480)) * wh;\n"
"    vec2 z = c;\n"
"    gl_FragColor = vec4(0, 0, 0, 1);\n"
"    for (int i = 0; i < 2222; ++i)\n"
"    {\n"
"        z = vec2(z.x*z.x - z.y*z.y, 2.f*z.x*z.y) + c;\n"
"        if (dot(z, z) > 4.f)\n"
"        {\n"
"            gl_FragColor = vec4(1, 0, 0, 1);\n"
"            break;\n"
"        }\n"
"    }\n"
"}\n";
static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}
int main(void)
{
	GLFWwindow* window;
	GLuint vertex_buffer, vertex_shader, fragment_shader, program;
	GLint mvp_location, vpos_location, vcol_location;
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		exit(EXIT_FAILURE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	window = glfwCreateWindow(640, 480, "Julia", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);
	// NOTE: OpenGL error checks have been omitted for brevity
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
	glCompileShader(vertex_shader);
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
	glCompileShader(fragment_shader);
	GLint isCompiled = 0;
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &maxLength);

		GLchar *errorLog = malloc(sizeof(GLchar) * maxLength);
		glGetShaderInfoLog(fragment_shader, maxLength, &maxLength, errorLog);

		fprintf(stderr, "Shader compilation errors:\n%s", errorLog);
		free(errorLog);
	}
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	mvp_location = glGetUniformLocation(program, "MVP");
	vpos_location = glGetAttribLocation(program, "vPos");
	vcol_location = glGetAttribLocation(program, "vCol");
	glEnableVertexAttribArray(vpos_location);
	glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
		sizeof(float) * 5, (void*)0);
	glEnableVertexAttribArray(vcol_location);
	glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
		sizeof(float) * 5, (void*)(sizeof(float) * 2));
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		printf("There was an error: %X !\n", err);
	}
	while (!glfwWindowShouldClose(window))
	{
		float ratio;
		int width, height;
		mat4x4 m, p, mvp;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		mat4x4_identity(m);
		mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		mat4x4_mul(mvp, p, m);
		glUseProgram(program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}