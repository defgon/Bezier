cmake_minimum_required(VERSION 3.0.0)
project(my_opengl_project VERSION 0.1.0)
cmake_policy(SET CMP0072 NEW)

# Find packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Set the directories
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/imgui)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(SHADERS_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(GLM_DIR ${CMAKE_SOURCE_DIR}/glm)

# Gather source files using file(GLOB)
file(GLOB IMGUI_SOURCES
    ${IMGUI_DIR}/*.cpp
    ${IMGUI_DIR}/*.h
)

file(GLOB GLM_SOURCES
    ${GLM_DIR}/*.cpp
    ${GLM_DIR}/*.h
)

# Include directories
include_directories(
    ${SRC_DIR}
    ${IMGUI_DIR}
    ${SHADERS_DIR}
    ${GLM_DIR}
)

# Create executable
add_executable(my_opengl_project 
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/glad.h
    ${SRC_DIR}/glad.c
    ${SRC_DIR}/camera.h
    ${SRC_DIR}/bezierCurve.h
    ${SRC_DIR}/bezierSurface.h
    ${SRC_DIR}/cube.h
    ${IMGUI_SOURCES}
    ${GLM_SOURCES}
)

# Link libraries
target_link_libraries(my_opengl_project 
    glfw
    OpenGL::GL
)

# Optionally specify C++ standard
set_target_properties(my_opengl_project PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)
