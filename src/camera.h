#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "gtx/rotate_vector.hpp"
#include "gtx/vector_angle.hpp"

class Camera 
{
    // nastaveni uvodni pozice kamery
    public:
    glm::vec3 Position;
    glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

    int width, height;
    float speed = 0.1f;
    float slowerSpeed = 0.03f;
    float sensitivity = 100.0f;

    Camera(int w, int h, glm::vec3 pos)
    {
        width = w;
        height = h;
        Position = pos;
    }

    void Matrix(float FOVdeg, float nearPlane, float farPlane, unsigned int shaderProgram, const char* uniform, bool is2DMode)
    {
        // 2 matice zobrazeni a to pohledova a projekcni (vysledek je zavisly na kombinaci techto dvou matic)
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
        /*
         pro 2d zobrazeni je pouzita ortograficka projekce 
         a pro 3d zobrazeni se pouziva perspektivni projekce
        */
        if (is2DMode) {
            projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
        } else {
            view = glm::lookAt(Position, glm::vec3(0.0f, 0.0f, 0.0f), Up);
            projection = glm::perspective(glm::radians(FOVdeg), (float)(width / height), nearPlane, farPlane);
        }
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, uniform), 1, GL_FALSE, glm::value_ptr(projection * view));
    }

    // umoznuje ovladani kamery pomoci klavesnice
    void Inputs(GLFWwindow* window)
    {
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            Position += speed * Orientation;
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            Position += speed * -glm::normalize(glm::cross(Orientation, Up));
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            Position += speed * -Orientation;
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            Position += speed * glm::normalize(glm::cross(Orientation, Up));
        if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            Position += speed * Up;
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            Position += speed * -Up;
    }
};
