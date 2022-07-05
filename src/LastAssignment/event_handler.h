#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "trans_mats.h"
#include <glad/gl.h>
#define GLFW_DLL
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

void resizeGL(GLFWwindow *window, int width, int height);
void mouseEvent(GLFWwindow *window, int button, int action, int mods);
glm::vec3 getVector(double x, double y, int winWidth, int winHeight);
void updateRotate(TransMats* mats, int winWidth, int winHeight);
void updateTranslate(TransMats* mats, int winWidth, int winHeight);
void updateScale(TransMats* mats);
void updateTransform(TransMats* mats, int winWidth, int winHeight);
void motionEvent(GLFWwindow *window, double xpos, double ypos);
void wheelEvent(GLFWwindow *window, double xoffset, double yoffset);

#endif // EVENT_HANDLER_H