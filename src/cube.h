std::vector<float> generateCubeVertices(float size)
{
    float halfSize = size / 2.0f;
    return {
        // zadni strana (red)
        -halfSize, -halfSize, -halfSize,  1.0f, 0.0f, 0.0f,
         halfSize, -halfSize, -halfSize,  1.0f, 0.0f, 0.0f,
         halfSize,  halfSize, -halfSize,  1.0f, 0.0f, 0.0f,
        -halfSize,  halfSize, -halfSize,  1.0f, 0.0f, 0.0f,
        // predni strana (green)
        -halfSize, -halfSize,  halfSize,  0.0f, 1.0f, 0.0f,
         halfSize, -halfSize,  halfSize,  0.0f, 1.0f, 0.0f,
         halfSize,  halfSize,  halfSize,  0.0f, 1.0f, 0.0f,
        -halfSize,  halfSize,  halfSize,  0.0f, 1.0f, 0.0f,
        // leva strana (blue)
        -halfSize, -halfSize, -halfSize,  0.0f, 0.0f, 1.0f,
        -halfSize,  halfSize, -halfSize,  0.0f, 0.0f, 1.0f,
        -halfSize,  halfSize,  halfSize,  0.0f, 0.0f, 1.0f,
        -halfSize, -halfSize,  halfSize,  0.0f, 0.0f, 1.0f,
        // prava strana (yellow)
         halfSize, -halfSize, -halfSize,  1.0f, 1.0f, 0.0f,
         halfSize,  halfSize, -halfSize,  1.0f, 1.0f, 0.0f,
         halfSize,  halfSize,  halfSize,  1.0f, 1.0f, 0.0f,
         halfSize, -halfSize,  halfSize,  1.0f, 1.0f, 0.0f,
        // dolni strana (purple)
        -halfSize, -halfSize, -halfSize,  1.0f, 0.0f, 1.0f,
         halfSize, -halfSize, -halfSize,  1.0f, 0.0f, 1.0f,
         halfSize, -halfSize,  halfSize,  1.0f, 0.0f, 1.0f,
        -halfSize, -halfSize,  halfSize,  1.0f, 0.0f, 1.0f,
        // horni strana (cyan)
        -halfSize,  halfSize, -halfSize,  0.0f, 1.0f, 1.0f,
         halfSize,  halfSize, -halfSize,  0.0f, 1.0f, 1.0f,
         halfSize,  halfSize,  halfSize,  0.0f, 1.0f, 1.0f,
        -halfSize,  halfSize,  halfSize,  0.0f, 1.0f, 1.0f 
    };
}


std::vector<unsigned int> generateCubeIndices()
{
    return {
        /*
         pro popis strany je zavedeno indexovani
         propojeni mezi jednotlivymi hranami kostky je umozneno pomoci na sebe navazujicich indexu
        */
        // zadni strana
        0, 1, 2, 2, 3, 0,
        // predni strana
        4, 5, 6, 6, 7, 4,
        // leva strana
        0, 3, 7, 7, 4, 0,
        // prava strana
        1, 5, 6, 6, 2, 1,
        // dolni strana
        0, 1, 5, 5, 4, 0,
        // horni strana
        3, 2, 6, 6, 7, 3
    };
}


std::vector<unsigned int> handleCubeIntoBuffers(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) 
{
    unsigned int VBO_cube, EBO_cube;
    glGenBuffers(1, &VBO_cube);
    glGenBuffers(1, &EBO_cube);

    unsigned int VAO_cube;
    glGenVertexArrays(1, &VAO_cube);

    glBindVertexArray(VAO_cube);

    // predani vrcholku do VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO_cube);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // pozice = layout 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // barva = layout 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // provadi vykresleni pomoci indexu
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_cube);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    std::vector<unsigned int> buffers;
    buffers.push_back(VAO_cube);
    buffers.push_back(VBO_cube);
    buffers.push_back(EBO_cube);
    return buffers;
}
