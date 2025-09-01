#pragma once
#include <iostream>
#include "glad.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include <vector>

// slouzi k vytvoreni 4x4 mrizky kontrolnich bodu plochy
void generatePointsOnGrid(float a, float b, glm::vec3 controlPoints[4][4]) 
{
    float halfA = a / 2.0f;  
    float halfB = b / 2.0f;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            // rozdeleni do 3 intervalu proto deleno 3.0f + delky stran "a" a "b"
            controlPoints[i][j] = glm::vec3(-halfA + j * (a / 3.0f),
                0.0f, -halfB + i * (b / 3.0f));
        }
    }
}

// pocita koeficienty u bernsteinova polynomu pro jeho ruzne prvky (i)
float bernstein(int i, float t) 
{
    switch (i) {
        case 0: return (1 - t) * (1 - t) * (1 - t);
        case 1: return 3 * (1 - t) * (1 - t) * t;    
        case 2: return 3 * (1 - t) * t * t;          
        case 3: return t * t * t;                    
        default: return 0.0f;
    }
}

// slouzi pro vypocet jednotlivych bodu plochy
glm::vec3 bezierSurface(glm::vec3 controlPoints[4][4], float u, float v) 
{
    glm::vec3 point(0.0f);
    // cisla ve forloops i,j znaci koeficienty u B v bernsteinove polynomu pro oba smery plochy
    for (int i = 0; i < 4; ++i) {
        float Bu = bernstein(i, u);
        for (int j = 0; j < 4; ++j) {
            float Bv = bernstein(j, v);
            point += Bu * Bv * controlPoints[i][j];
        }
    }
    return point;
}

// vypocet vsech bodu plochy s krokem t
void calculateBezierSurfacePoints(glm::vec3 controlPoints[4][4], float step, std::vector<glm::vec3>& surfacePoints) 
{
    surfacePoints.clear();
    for (float u = 0.0f; u <= 1.0f; u += step) {
        for (float v = 0.0f; v <= 1.0f; v += step) {
            surfacePoints.push_back(bezierSurface(controlPoints, u, v));
        }
    }
}

void render3DBezierSurface(glm::vec3 controlPoints[4][4], unsigned int shaderProgram, 
                           unsigned int VAO_cube, int indexCount, float step)
{
    std::vector<glm::vec3> surfacePoints;
    glUseProgram(shaderProgram);

    // vykresleni krychli kontrolnich bodu plochy
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, controlPoints[i][j]);
            model = glm::scale(model, glm::vec3(0.1f));

            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(model));

            glBindVertexArray(VAO_cube);
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        }
    }

    
    calculateBezierSurfacePoints(controlPoints, step, surfacePoints);

    // vykresleni krychli bodu plochy
    for (const auto& surfacePoint : surfacePoints) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, surfacePoint);
        model = glm::scale(model, glm::vec3(0.05f));

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO_cube);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}






