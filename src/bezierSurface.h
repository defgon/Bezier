void generatePointsOnGrid(float a, float b, glm::vec3 controlPoints[4][4]) 
{
    float halfA = a / 2.0f;  
    float halfB = b / 2.0f;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            // rozdělení do 3 intervalů proto děleno 3.0f + strany "a" a "b"
            controlPoints[i][j] = glm::vec3(-halfA + j * (a / 3.0f),
                0.0f, -halfB + i * (b / 3.0f));
        }
    }
}

// koeficienty u bernsteinova polynomu
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

// výpočet bodu na ploše
glm::vec3 bezierSurface(glm::vec3 controlPoints[4][4], float u, float v) 
{
    glm::vec3 point(0.0f);
    // čísla ve forloops značí koeficienty u B v bernsteinově polynomu
    for (int i = 0; i < 4; ++i) {
        float Bu = bernstein(i, u);
        for (int j = 0; j < 4; ++j) {
            float Bv = bernstein(j, v);
            point += Bu * Bv * controlPoints[i][j];
        }
    }
    return point;
}

// výpočet všech bodů plochy
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

    // render krychlí kontrolních bodů
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

    // render krychlí bodů povrchu
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






