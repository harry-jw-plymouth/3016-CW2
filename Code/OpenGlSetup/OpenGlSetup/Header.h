#pragma once
//framebuffer_size_callback() needs GlFW, so include moved here
#include <GLFW/glfw3.h>

//Called on window resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void ProcessUserInput(GLFWwindow* WindowIn);
void Mouse_CallBack(GLFWwindow* window, double xpos, double ypos);
void CheckForCollision();
void SetUpTerrain();

//GLuint program;