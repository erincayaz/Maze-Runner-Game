#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/common.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Window.hpp"
#include "VAO.hpp"
#include "ShaderProgram.hpp"
#include "Parametric3DShape.hpp"
#include <iostream>
#include <vector>
using namespace std;

// Globals
unsigned width = 800, height = 800;
bool increaseX1 = false;
int u_transform;
float X1 = 0;
float Y1 = 0;
float X2 = 0;
float Y2 = 0;

void draw(unsigned frameCount, VAO &object1, VAO &object2, VAO &object3, VAO &object4)
{
    glm::mat4 transform;

    // Draw object 1 at (-0.5, 0.5, 0)
    transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3(-0.5 + X1, 0.5 + Y1, 0));
    transform = glm::scale(transform, glm::vec3(0.2));
    transform = glm::rotate(transform, glm::radians(frameCount * 0.5f), glm::vec3(1, 1, 0));
    glUniformMatrix4fv(u_transform, 1, GL_FALSE, glm::value_ptr(transform));

    object1.bind();
    glDrawElements(GL_TRIANGLES, object1.getIndicesCount(), GL_UNSIGNED_INT, NULL);

    /*
    // Draw object 2 at (0.5, -0.5, 0)
    transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3(X2, Y2, 0));
    transform = glm::scale(transform, glm::vec3(0.2));
    transform = glm::rotate(transform, glm::radians(frameCount * 0.5f), glm::vec3(1, 1, 0));
    glUniformMatrix4fv(u_transform, 1, GL_FALSE, glm::value_ptr(transform));

    object2.bind();
    glDrawElements(GL_TRIANGLES, object2.getIndicesCount(), GL_UNSIGNED_INT, NULL);

    // Draw object 3 at (-0.5, -0.5, 0)
    transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3(-0.5, -0.5, 0));
    transform = glm::scale(transform, glm::vec3(0.2));
    transform = glm::rotate(transform, glm::radians(frameCount * 0.5f), glm::vec3(1, 1, 0));
    glUniformMatrix4fv(u_transform, 1, GL_FALSE, glm::value_ptr(transform));

    object3.bind();
    glDrawElements(GL_TRIANGLES, object3.getIndicesCount(), GL_UNSIGNED_INT, NULL);

    // Draw object 4 at (0.5, 0.5, 0)
    transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3(0.5, 0.5, 0));
    transform = glm::scale(transform, glm::vec3(0.2));
    transform = glm::rotate(transform, glm::radians(frameCount * 0.5f), glm::vec3(1, 1, 0));
    glUniformMatrix4fv(u_transform, 1, GL_FALSE, glm::value_ptr(transform));

    object4.bind();
    glDrawElements(GL_TRIANGLES, object4.getIndicesCount(), GL_UNSIGNED_INT, NULL);
    */
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        Y1 += 0.1;
    }
    else if(key == GLFW_KEY_S && action == GLFW_PRESS) {
        Y1 -= 0.1;
    }
    else if(key == GLFW_KEY_A && action == GLFW_PRESS) {
        X1 -= 0.1;
    }
    else if(key == GLFW_KEY_D && action == GLFW_PRESS) {
        increaseX1 = true;
    }
    else if(key == GLFW_KEY_D && action == GLFW_RELEASE) {
        increaseX1 = false;
    }
}

static void cursorPositionCallback(GLFWwindow* window, double x, double y)
{
    X2 = 2.0*((float)x / width) - 1;
    Y2 = 2.0*(1-((float)y / height)) - 1;
}
        
int main()
{
    Window::init(width, height, "my window");
    glfwSetKeyCallback(Window::window, keyCallback);
    glfwSetCursorPosCallback(Window::window, cursorPositionCallback);

    // create objects
    vector<glm::vec3> positions;
    vector<unsigned int> indices;

    Parametric3DShape::generate(positions, indices, ParametricLine::halfCircle, 30, 30);
    VAO sphereHighRes(positions, indices);

    Parametric3DShape::generate(positions, indices, ParametricLine::halfCircle, 15, 15);
    VAO sphereLowRes(positions, indices);

    Parametric3DShape::generate(positions, indices, ParametricLine::circle, 30, 30);
    VAO donut(positions, indices);

    Parametric3DShape::generate(positions, indices, ParametricLine::spikes, 30, 30);
    VAO spikyDonut(positions, indices);

    // create shader
    ShaderProgram sp("../shader/vertex.vert", "../shader/frag.frag");
    u_transform = glGetUniformLocation(sp.id, "u_transform");

    sp.use();
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // only draw vertices

    // game loop
    long long unsigned int frameCount = 0;
    while (!Window::isClosed())
    {
        if(increaseX1)
            X1 += 0.01;

        glClear(GL_COLOR_BUFFER_BIT);

        // DRAW
        draw(frameCount, sphereHighRes, sphereLowRes, donut, spikyDonut);

        Window::swapBuffersAndPollEvents();
        frameCount++;
    }

    glfwTerminate();

    return 0;
}