#include <iostream>
#include "glad.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "camera.h"
#include "bezierCurve.h"
#include "bezierSurface.h"
#include "cube.h"

/*
  POPIS PROGRAMU
  **************
- jako prvni zobrazuje program 2d bezierovu krivku
- po rozevreni menu na levo lze manipulovat s jednotlivymi body krivky pomoci posuvniku 
- v menu dale je zde take umozneno menit krok krivky a tim i samotne zobrazeni
- pro prepnuti do 3d bezierovy plochy je treba rozkliknout "Basic window settings" a vybrat "Go 3D plane"
- ovladani jednotlivych bodu plochy v 3d je umozneno pomoci sipek na klavesnici
- prepinani mezi jednotlivymi body v 3d plose je umozneno pomoci Ctrl+Cisel (sloupec) a Cisel (radek)
- v menu dale je zde take umozneno menit krok plochy (tj. její spojitost)
- otaceni kamery v 3d je umozneno pomoci tlacitek WASD (pro rotaci), mezernik pro oddaleni kamery 
  a shift pro priblizeni
*/

// k uzavreni okna
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// umoznuje nacitani shaderu ze souboru
const char* readShader(const std::string& filename) 
{
    std::ifstream shaderFile(filename);
    if (!shaderFile) {
        throw std::runtime_error("Unable to open file: " + filename);
    }

    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();
    std::string shaderCode = shaderStream.str();

    char* shaderCodeStr = new char[shaderCode.length() + 1];
    std::copy(shaderCode.begin(), shaderCode.end(), shaderCodeStr);
    shaderCodeStr[shaderCode.length()] = '\0'; // ukonceni stringu

    return shaderCodeStr;
}

// 3d a 2d
std::vector<unsigned int> handleShaderProgram(const char* fragment_shaderCode, const char* vertex_shaderCode)
{
    // vertex shader
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertex_shaderCode, NULL);
    glCompileShader(vertexShader);

    // fragment shader
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragment_shaderCode, NULL);
    glCompileShader(fragmentShader);

    // shader program
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glUseProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return {shaderProgram}; 
}

int main() 
{
    if(!glfwInit()){
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // velikosti okna
    const unsigned int width = 1000;
    const unsigned int height = 600;

    GLFWwindow* window = glfwCreateWindow(width, height, "Bezier Curves", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeLimits(window, 850, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        glfwTerminate();
        return -1;
    }
    // barva pozadi okna
    glClearColor(0, 0, 0, 1.0f);


    //IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    // 2d/3d zobrazení
    bool is2DMode = true;
    // krok bezierovy krivky 2d
    float step2d = 0.01f;

    // kontrolni body pro 2d
    glm::vec3 controlPoints2d[4] = {
        {0.5f, 0.5f, 0.0f},
        {0.5f, -0.5f, 0.0f},
        {0.0f, 0.5f, 0.0f},
        {0.0f, -0.5f, 0.0f}
    };

    // kontrolní body pro 3d
    glm::vec3 controlPoints3d[4][4];
    generatePointsOnGrid(0.5f, 0.5f, controlPoints3d);
    std::vector<glm::vec3> bezierSurfacePoints;
    // krok bezierovy krivky 3d
    float step3d = 0.05f;

    // inicializace shaderu
    std::string shaderPath = "../shaders/fragment_shader.glsl";
    const char* fragment_shaderCode = readShader(shaderPath);
    shaderPath = "../shaders/vertex_shader.glsl";
    const char* vertex_shaderCode = readShader(shaderPath);

    // 2d shader
    std::vector<unsigned int> setUpShader = handleShaderProgram(fragment_shaderCode,vertex_shaderCode);
    std::vector<unsigned int> setup2d = handlePointsIntoBuffers(controlPoints2d);
    setup2d.insert(setup2d.begin(), setUpShader.begin(), setUpShader.end());

    // 3d nacteni kostek
    std::vector<float> cubeVertices = generateCubeVertices(0.1f);
    std::vector<unsigned int> cubeIndices = generateCubeIndices();
    std::vector<unsigned int> cubeBuffers = handleCubeIntoBuffers(cubeVertices, cubeIndices);

    // umoznuje nepruhlednost v 3d zobrazeni
    glEnable(GL_DEPTH_TEST);

    Camera camera(width,height, glm::vec3(40.0f,40.0f,30.0f));

    // pro manipulaci bodu v 3d
    int selectedRow = 0;
    int selectedCol = 0;
    float moveSpeed = 0.01f;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        processInput(window);

        // IMGUI nacteni
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            static int counter = 0;
            ImGui::Begin("Settings");
            
            // umoznuje zmenit mezi 2d a 3d krivkou/plochou
            if (ImGui::CollapsingHeader("Basic window settings")) {
                if (ImGui::Button("Go 2D plane")) {
                    is2DMode = true;
                }
                if (ImGui::Button("Go 3D plane")) {
                    is2DMode = false;
                }
            }

            // umoznuje ovladani jednotlivych bodu krivky v 2d
            if (is2DMode && ImGui::CollapsingHeader("2d bezier curves")) {
                ImGui::SliderFloat("Step", &step2d, 0.01f, 1.0f, "%.3f");
                if(ImGui::CollapsingHeader("Points")){
                    for(int i = 0; i < 4; i++){
                        if(ImGui::CollapsingHeader(("Point " + std::to_string(i)).c_str())){
                            ImGui::SliderFloat(("X" + std::to_string(i)).c_str(),
                             &controlPoints2d[i].x, -1.0f, 1.0f);
                            ImGui::SliderFloat(("Y" + std::to_string(i)).c_str(),
                             &controlPoints2d[i].y, -1.0f, 1.0f);
                        }
                    }
                }
            }
            // umoznuje ovladani jednotlivych bodu plochy v 3d
            if (!is2DMode && ImGui::CollapsingHeader("3d bezier surfaces")) {
                ImGui::SliderFloat("Step", &step3d, 0.01f, 1.0f, "%.3f");
                ImGui::Text("Selected row: = %d", selectedRow);
                ImGui::Text("Selected column: = %d", selectedCol);
            }
            ImGui::Text("Application average \n %.3f ms/frame \n (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            
            ImGui::End();
        }

        ImGui::Render();
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        camera.Inputs(window);

        if (is2DMode) {
            // nastavuje kameru pro 2d zobrazeni krivky
            camera.Matrix(45.0f,0.1f,100.f, setUpShader[0],"camMatrix", is2DMode);
            render2DBezierCurve(controlPoints2d, step2d, setup2d[0], setup2d[1], setup2d[2], setup2d[3], setup2d[4], setup2d[5], setup2d[6]);
        } else {
            // pro manipulaci s kontrolnímy body ve 3d
            bool ctrlPressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
            // prepinaci system mezi kontrolnimi body v 3d pomoci Ctrl+Cisel (sloupec) a Cisel (radek)
            if (ctrlPressed) {
                if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
                    selectedCol = 0; 
                } else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
                    selectedCol = 1;  
                } else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
                    selectedCol = 2;  
                } else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
                    selectedCol = 3;  
                }
            } else {
                if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
                    selectedRow = 0; 
                } else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
                    selectedRow = 1;  
                } else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
                    selectedRow = 2;  
                } else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
                    selectedRow = 3;  
                }
                // ovladani jednotlivych bodu plochy v 3d pomoci sipek na klavesnici
                if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
                    controlPoints3d[selectedRow][selectedCol].x -= moveSpeed;
                }
                if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
                    controlPoints3d[selectedRow][selectedCol].x += moveSpeed;
                }
                if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
                    controlPoints3d[selectedRow][selectedCol].y += moveSpeed;
                }
                if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
                    controlPoints3d[selectedRow][selectedCol].y -= moveSpeed;
                }
            }
            // naciteni kamery pro 3d zobrazeni plochy
            camera.Matrix(45.0f,0.1f,200.f, setUpShader[0],"camMatrix", is2DMode);
            render3DBezierSurface(controlPoints3d,setUpShader[0],cubeBuffers[0],cubeIndices.size(),step3d);
        }

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Konec
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
