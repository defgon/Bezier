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

// vypocet bezierovy krivky pro body P0 az P3 s krokem t
glm::vec3 bezierCurve(const glm::vec3& P0, const glm::vec3& P1, const glm::vec3& P2, const glm::vec3& P3, float t) 
{
    float b03 = (1.0f - t) * (1.0f - t) * (1.0f - t);
    float b13 = 3 * (1.0f - t) * (1.0f - t) * t;
    float b23 = 3 * (1.0f - t) * t * t;
    float b33 = t * t * t;

    glm::vec3 p;
    p.x = b03 * P0.x + b13 * P1.x + b23 * P2.x + b33 * P3.x;
    p.y = b03 * P0.y + b13 * P1.y + b23 * P2.y + b33 * P3.y;
    // z je ve 2D = 0.0f
    p.z = 0.0f;
    return p;
}

// vypocitava body krivky pomoci bernsteinova polynomu pri kroku t
void calculateBezierCurvePoints(const glm::vec3 controlPoints[4], float step, std::vector<glm::vec3>& curvePoints) 
{
    curvePoints.clear();
    for (float t = 0.0f; t <= 1.0f; t += step) {
        curvePoints.push_back(bezierCurve(controlPoints[0], controlPoints[1], controlPoints[2], controlPoints[3], t));
    }
}

std::vector<unsigned int> handlePointsIntoBuffers(glm::vec3 controlPoints[4])
{ 
    
    // VBO krivka a usecky
    unsigned int VBO_control, VBO_curve, VBO_lines;
    glGenBuffers(1, &VBO_control);
    glGenBuffers(1, &VBO_curve);
    glGenBuffers(1, &VBO_lines);

    // VAO krivka a usecky
    unsigned int VAO_control, VAO_curve, VAO_lines;
    glGenVertexArrays(1, &VAO_control);  
    glGenVertexArrays(1, &VAO_curve);
    glGenVertexArrays(1, &VAO_lines);

    // VAO and VBO pro kontrolni body
    glBindVertexArray(VAO_control);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_control);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), controlPoints, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // VAO and VBO pro body na krivce
    glBindVertexArray(VAO_curve);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_curve);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // predani bufferu opengl pro cary mezi body na krivce
    glBindVertexArray(VAO_lines);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_lines);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    std::vector<unsigned int> a;
    a.push_back(VAO_curve);
    a.push_back(VBO_curve);
    a.push_back(VAO_lines);
    a.push_back(VBO_lines);
    a.push_back(VAO_control);
    a.push_back(VBO_control);
    return a;
}


void render2DBezierCurve(const glm::vec3 controlPoints[4], float step, unsigned int shaderProgram,
                         unsigned int VAO_curve, unsigned int VBO_curve, 
                         unsigned int VAO_lines, unsigned int VBO_lines,
                         unsigned int VAO_control, unsigned int VBO_control) 
{
    std::vector<glm::vec3> curvePoints;
    calculateBezierCurvePoints(controlPoints, step, curvePoints);

    std::vector<float> curvePt;
    for (const glm::vec3& point : curvePoints) {
        // prirazeni barvy bodum na krivce
        curvePt.push_back(point.x);
        curvePt.push_back(point.y);
        curvePt.push_back(point.z);
        curvePt.push_back(1.0f);
        curvePt.push_back(1.0f);
        curvePt.push_back(1.0f);
    }

    std::vector<float> controlVertices;
    for (int i = 0; i < 4; ++i) {
        // prirazeni barvy kontrolnim bodum krivky
        controlVertices.push_back(controlPoints[i].x);
        controlVertices.push_back(controlPoints[i].y);
        controlVertices.push_back(controlPoints[i].z);
        controlVertices.push_back(1.0f);
        controlVertices.push_back(0.0f);
        controlVertices.push_back(0.0f);
    }

    glUseProgram(shaderProgram);

    // ve 2d nedela nic, je to pouze identita
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(model));

    // vykresleni krivky v opengl
    glBindVertexArray(VAO_curve);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_curve);
    glBufferData(GL_ARRAY_BUFFER, curvePt.size() * sizeof(float), curvePt.data(), GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_LINE_STRIP, 0, curvePoints.size());

    // vykresleni kontrolnich bodu krivky v opengl
    glBindVertexArray(VAO_control);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_control);
    glBufferData(GL_ARRAY_BUFFER, controlVertices.size() * sizeof(float), controlVertices.data(), GL_DYNAMIC_DRAW);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // nastaveni velikosti bodu pro kontrolni body krivky
    glPointSize(5.0f);
    glDrawArrays(GL_POINTS, 0, 4);

    // pridani cary mezi poslednim kontrolnim bodem a poslednim bodem z bodu krivky
    if (!curvePoints.empty()) {
        std::vector<float> lineVertices = {
            controlPoints[3].x, controlPoints[3].y, controlPoints[3].z, 1.0f, 1.0f, 1.0f,
            curvePoints.back().x, curvePoints.back().y, curvePoints.back().z, 1.0f, 1.0f, 1.0f
        };

        glBindVertexArray(VAO_lines);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_lines);
        glBufferData(GL_ARRAY_BUFFER, lineVertices.size() * sizeof(float), lineVertices.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_LINES, 0, 2);

        glBindVertexArray(0);
    }
    glBindVertexArray(0);
}
