#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <stack>
#include <set>
#include <math.h>

#include "camera.h"
#include "entity.h"

#include <iostream>

struct gameObject {
    glm::vec3 pos;
    glm::vec3 size;
    float rotation;
    glm::vec3 rotAxis;
    string texture;

    gameObject() {
        pos = glm::vec3(0.0f);
        size = glm::vec3(1.0f);
        rotation = 0.0f;
        rotAxis = glm::vec3(0.0f);
        texture = "ground";
    }

    gameObject(glm::vec3 p, float r = 0.0f, glm::vec3 ra = glm::vec3(0.0f, 0.0f, 0.0f), string t = "ground", glm::vec3 s = glm::vec3(1.0f, 1.0f, 1.0f)) {
        pos = p;
        size = s;
        rotation = r;
        rotAxis = ra;
        texture = t;
    }
};

std::vector <gameObject> objects;
std::vector <glm::vec3> lightPos;
std::vector <glm::vec3> ghostPos;
glm::vec3 spiderPos;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
void computeMap();
void compMap();
void gravity();
bool checkFinish();
bool checkSpiderCollision();
bool checkGhostCollision();
void calculatePosOfSpider();
void calculatePosOfGhost();
void computePath(vector <pair <int, int>> path, int x, int y, vector <pair <int, int>> & resultPath);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(2.0f, 10.0f, 2.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Maze
//----CONSTANTS-------------------------------------------------------
#define GRID_WIDTH 30
#define GRID_HEIGHT 15
#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3
//----GLOBAL VARIABLES------------------------------------------------
char grid[GRID_WIDTH * GRID_HEIGHT];
char grid_map[GRID_WIDTH * GRID_HEIGHT * 9];
vector <pair <int, int>> resultPath;
//----FUNCTION PROTOTYPES---------------------------------------------
void ResetGrid();
int XYToIndex(int x, int y);
int XYToIndex3(int x, int y);
int IsInBounds(int x, int y);
void Visit(int x, int y);
void PrintGrid();
/////////////////////////////////////////////////////////////////////

/// A Star Definitions ///////////////////////////////////////////////
// Creating a shortcut for int, int pair type
typedef pair<int, int> Pair;

// Creating a shortcut for pair<int, pair<int, int>> type
typedef pair<double, pair<int, int> > pPair;

// A structure to hold the necessary parameters
struct cell {
    // Row and Column index of its parent
    // Note that 0 <= i <= ROW-1 & 0 <= j <= COL-1
    int parent_i, parent_j;
    // f = g + h
    double f, g, h;
};

bool isValid(int row, int col);
bool isUnBlocked(int row, int col);
bool isDestination(int row, int col, Pair dest);
double calculateHValue(int row, int col, Pair dest);
void tracePath(cell cellDetails[][GRID_WIDTH * 3], Pair dest);
void aStarSearch(Pair src, Pair dest);

stack<Pair> spiderPath;
stack<Pair> lastPath;
int pcX, pcY;
//////////////////////////////////////////////////////////////////////

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader lightingShader("colors.vert", "colors.frag");
    Shader lightCubeShader("light_cube.vert", "light_cube.frag");
    Shader ourShader("vertex.vert", "frag.frag");

    // Load Shader
    Model ourModel("backpack.obj");
    Model spiderModel("scene.gltf");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    float vertices2[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f
    };

    // positions all containers
    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f,  0.0f,  0.0f)
    };

    for (int i = 0; i < 1; i++) {
        int rX = rand() % 90 + 10, rY = rand() % 60 + 10;
        glm::vec3 temp = glm::vec3(rX, -0.3, rY);
        ghostPos.push_back(temp);
    }

    // Maze Generation
    srand(time(0));
    ResetGrid();
    Visit(1, 1);
    compMap();
    PrintGrid();
    //computeMap();
    /*****************/

    // Cube Pos
    glm::vec3 cubePos = glm::vec3(0.0f, -10.0f, -2.0f);
    /////////

    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f,  0.2f,  2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3(0.0f,  5.0f, -3.0f)
    };

    // first, configure the cube's VAO (and VBO)
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Plane's VAO
    unsigned int VBO2, planeVAO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &VBO2);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

    glBindVertexArray(planeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // load textures (we now use a utility function to keep the code more organized)
    // -----------------------------------------------------------------------------
    unsigned int diffuseMap = loadTexture("Bricks076A_1K_Color.png");
    unsigned int specularMap = loadTexture("Bricks076A_1K_Displacement.png");

    unsigned int diffuseMapGround = loadTexture("Ground037_1K_Color.png");
    unsigned int specularMapGround = loadTexture("Ground037_1K_Displacement.png");

    // shader configuration
    // --------------------
    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {

        // Debug

        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);
        gravity();

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);
        lightingShader.setFloat("material.shininess", 32.0f);

        // directional light
        lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        // point light 1
        lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        lightingShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[0].constant", 1.0f);
        lightingShader.setFloat("pointLights[0].linear", 0.09);
        lightingShader.setFloat("pointLights[0].quadratic", 0.032);
        // point light 2
        lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        lightingShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[1].constant", 1.0f);
        lightingShader.setFloat("pointLights[1].linear", 0.09);
        lightingShader.setFloat("pointLights[1].quadratic", 0.032);
        // point light 3
        lightingShader.setVec3("pointLights[2].position", pointLightPositions[2]);
        lightingShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[2].constant", 1.0f);
        lightingShader.setFloat("pointLights[2].linear", 0.09);
        lightingShader.setFloat("pointLights[2].quadratic", 0.032);
        // point light 4
        lightingShader.setVec3("pointLights[3].position", pointLightPositions[3]);
        lightingShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[3].constant", 1.0f);
        lightingShader.setFloat("pointLights[3].linear", 0.09);
        lightingShader.setFloat("pointLights[3].quadratic", 0.032);
        // spotLight
        lightingShader.setVec3("spotLight.position", camera.Position);
        lightingShader.setVec3("spotLight.direction", camera.Front);
        lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("spotLight.specular", 0.0f, 1.0f, 1.0f);
        lightingShader.setFloat("spotLight.constant", 1.0f);
        lightingShader.setFloat("spotLight.linear", 0.09);
        lightingShader.setFloat("spotLight.quadratic", 0.032);
        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 10.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down
        lightingShader.setMat4("model", model);
        ourModel.Draw(lightingShader);

        // render spider
        calculatePosOfSpider();

        model = glm::mat4(1.0f);
        model = glm::translate(model, spiderPos);
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0f, 0, 0));
        lightingShader.setMat4("model", model);
        spiderModel.Draw(lightingShader);
        
        calculatePosOfGhost();
        for (int i = 0; i < ghostPos.size(); i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, ghostPos[i]);
            model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
            model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0f, 0, 0));
            lightingShader.setMat4("model", model);
            spiderModel.Draw(lightingShader);
        }

        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        // render containers
        glBindVertexArray(cubeVAO);
        for (unsigned int i = 0; i < 1; i++)
        {
            float velocity = 1.0f * deltaTime;

            // calculate the model matrix for each object and pass it to shader before drawing
            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, cubePos);
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


        glBindVertexArray(planeVAO);
        for (unsigned int i = 0; i < objects.size(); i++) {
            if (objects[i].texture == "ground") {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, diffuseMapGround);
                // bind specular map
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, specularMapGround);
            }
            else if (objects[i].texture == "wall") {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, diffuseMap);
                // bind specular map
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, specularMap);
            }

            model = glm::mat4(1.0f);
            model = glm::translate(model, objects[i].pos);
            model = glm::rotate(model, glm::radians(objects[i].rotation), objects[i].rotAxis);
            lightingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);

        // we now draw as many light bulbs as we have point lights.
        glBindVertexArray(lightCubeVAO);
        for (unsigned int i = 0; i < lightPos.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, lightPos[i]);
            model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
            lightCubeShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        if (checkFinish()) {
            camera.Position = glm::vec3(2.0f, 20.0f, 2.0f);
            ResetGrid();
            Visit(1, 1);
            PrintGrid();
            objects.clear();
            lightPos.clear();
            compMap();

            for (int i = 0; i < ghostPos.size(); i++) {
                int rX = rand() % 90 + 10, rY = rand() % 60 + 10;
                glm::vec3 temp = glm::vec3(rX, -0.3, rY);
                ghostPos[i] = temp;
            }
        }
        if (checkSpiderCollision() || checkGhostCollision()) {
            camera.Position = glm::vec3(3.0f, 3.0f, 3.0f);
            spiderPos = glm::vec3(lightPos[0].x, 0, lightPos[0].z);

            for (int i = 0; i < ghostPos.size(); i++) {
                int rX = rand() % 90 + 10, rY = rand() % 60 + 10;
                glm::vec3 temp = glm::vec3(rX, -0.3, rY);
                ghostPos[i] = temp;
            }
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &VBO2);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void calculatePosOfSpider() {
    int x = (int)ceil(spiderPos.x), y = (int)ceil(spiderPos.z);
    int cx = (int)ceil(camera.Position.x), cy = (int)ceil(camera.Position.z);

    Pair src = make_pair(y, x);
    Pair dest = make_pair(cy, cx);
    aStarSearch(src, dest);

    if (spiderPath.empty() && !lastPath.empty()) {
        spiderPath = lastPath;
    }

    if (!spiderPath.empty()) {
        Pair coord = spiderPath.top();
        if (coord.second == x && coord.first == y) {
            spiderPath.pop();
            coord = spiderPath.top();
        }

        float velocity = 3.0f * deltaTime;
        if (coord.second > x) {
            if (coord.first> y)
                spiderPos += glm::vec3(1.0f, 0, 1.0f) * velocity;
            else if (coord.first < y)
                spiderPos += glm::vec3(1.0f, 0, -1.0f) * velocity;
            else
                spiderPos += glm::vec3(1.0f, 0, 0) * velocity;
        }
        else if (coord.second < x) {
            if (coord.first > y)
                spiderPos += glm::vec3(-1.0f, 0, 1.0f) * velocity;
            else if (coord.first < y)
                spiderPos -= glm::vec3(1.0f, 0, 1.0f) * velocity;
            else
                spiderPos -= glm::vec3(1.0f, 0, 0) * velocity;
        }
        else {
            if (coord.first > y) {
                spiderPos += glm::vec3(0, 0, 1.0f) * velocity;
            }
            else {
                spiderPos -= glm::vec3(0, 0, 1.0f) * velocity;
            }
        }
    }
    else {
        float velocity = 3.0f * deltaTime;
        glm::vec3 cameraPos = camera.Position;
        float cx = cameraPos.x, cy = cameraPos.z;
        glm::vec3 temp = spiderPos;
        float x = temp.x, y = temp.z;

        if (cx > x) {
            if (cx > y)
                spiderPos += glm::vec3(1.0f, 0, 1.0f) * velocity;
            else if (cy < y)
                spiderPos += glm::vec3(1.0f, 0, -1.0f) * velocity;
            else
                spiderPos += glm::vec3(1.0f, 0, 0) * velocity;
        }
        else if (cx < x) {
            if (cy > y)
                spiderPos += glm::vec3(-1.0f, 0, 1.0f) * velocity;
            else if (cy < y)
                spiderPos -= glm::vec3(1.0f, 0, 1.0f) * velocity;
            else
                spiderPos -= glm::vec3(1.0f, 0, 0) * velocity;
        }
        else {
            if (cy > y) {
                spiderPos += glm::vec3(0, 0, 1.0f) * velocity;
            }
            else {
                spiderPos -= glm::vec3(0, 0, 1.0f) * velocity;
            }
        }
    }
}

void calculatePosOfGhost() {
    glm::vec3 cameraPos = camera.Position;
    float cx = cameraPos.x, cy = cameraPos.z;

    for (int i = 0; i < ghostPos.size(); i++) {
        glm::vec3 temp = ghostPos[i];
        float x = temp.x, y = temp.z;
        float velocity = 2.5f * deltaTime;

        if (cx > x) {
            if (cy > y)
                ghostPos[i] += glm::vec3(1.0f, 0, 1.0f) * velocity;
            else if (cy < y)
                ghostPos[i] += glm::vec3(1.0f, 0, -1.0f) * velocity;
            else
                ghostPos[i] += glm::vec3(1.0f, 0, 0) * velocity;
        }
        else if (cx < x) {
            if (cy > y)
                ghostPos[i] += glm::vec3(-1.0f, 0, 1.0f) * velocity;
            else if (cy < y)
                ghostPos[i] -= glm::vec3(1.0f, 0, 1.0f) * velocity;
            else
                ghostPos[i] -= glm::vec3(1.0f, 0, 0) * velocity;
        }
        else {
            if (cy > y) {
                ghostPos[i] += glm::vec3(0, 0, 1.0f) * velocity;
            }
            else {
                ghostPos[i] -= glm::vec3(0, 0, 1.0f) * velocity;
            }
        }
    }
}



// Collision detection by looking at the direction camera wants to move and check if it collides with any object.
//---------------------------------------------------------------------------------------------------------------
bool checkCollision(std::vector <gameObject> objects, string direction, float distance) {
    for (int i = 0; i < objects.size(); i++) {
        if (direction == "front") {
            if ((camera.Position + camera.Front * distance).x > objects[i].pos.x - objects[i].size.x / 2 &&
                (camera.Position + camera.Front * distance).x < objects[i].pos.x + objects[i].size.x / 2 &&
                (camera.Position + camera.Front * distance).y > objects[i].pos.y - objects[i].size.y / 2 &&
                (camera.Position + camera.Front * distance).y < objects[i].pos.y + objects[i].size.y / 2 &&
                (camera.Position + camera.Front * distance).z > objects[i].pos.z - objects[i].size.z / 2 &&
                (camera.Position + camera.Front * distance).z < objects[i].pos.z + objects[i].size.z / 2) {
                return true;
            }
        }
        else if (direction == "back") {
            if ((camera.Position - camera.Front * distance).x > objects[i].pos.x - objects[i].size.x / 2 &&
                (camera.Position - camera.Front * distance).x < objects[i].pos.x + objects[i].size.x / 2 &&
                (camera.Position - camera.Front * distance).y > objects[i].pos.y - objects[i].size.y / 2 &&
                (camera.Position - camera.Front * distance).y < objects[i].pos.y + objects[i].size.y / 2 &&
                (camera.Position - camera.Front * distance).z > objects[i].pos.z - objects[i].size.z / 2 &&
                (camera.Position - camera.Front * distance).z < objects[i].pos.z + objects[i].size.z / 2) {
                return true;
            }
        }
        else if (direction == "left") {
            if ((camera.Position - camera.Right * distance).x > objects[i].pos.x - objects[i].size.x / 2 &&
                (camera.Position - camera.Right * distance).x < objects[i].pos.x + objects[i].size.x / 2 &&
                (camera.Position - camera.Right * distance).y > objects[i].pos.y - objects[i].size.y / 2 &&
                (camera.Position - camera.Right * distance).y < objects[i].pos.y + objects[i].size.y / 2 &&
                (camera.Position - camera.Right * distance).z > objects[i].pos.z - objects[i].size.z / 2 &&
                (camera.Position - camera.Right * distance).z < objects[i].pos.z + objects[i].size.z / 2) {
                return true;
            }
        }
        else if (direction == "right") {
            if ((camera.Position + camera.Right * distance).x > objects[i].pos.x - objects[i].size.x / 2 &&
                (camera.Position + camera.Right * distance).x < objects[i].pos.x + objects[i].size.x / 2 &&
                (camera.Position + camera.Right * distance).y > objects[i].pos.y - objects[i].size.y / 2 &&
                (camera.Position + camera.Right * distance).y < objects[i].pos.y + objects[i].size.y / 2 &&
                (camera.Position + camera.Right * distance).z > objects[i].pos.z - objects[i].size.z / 2 &&
                (camera.Position + camera.Right * distance).z < objects[i].pos.z + objects[i].size.z / 2) {
                return true;
            }
        }
        else if (direction == "up") {
            if ((camera.Position + camera.Up * distance).x > objects[i].pos.x - objects[i].size.x / 2 &&
                (camera.Position + camera.Up * distance).x < objects[i].pos.x + objects[i].size.x / 2 &&
                (camera.Position + camera.Up * distance).y > objects[i].pos.y - objects[i].size.y / 2 &&
                (camera.Position + camera.Up * distance).y < objects[i].pos.y + objects[i].size.y / 2 &&
                (camera.Position + camera.Up * distance).z > objects[i].pos.z - objects[i].size.z / 2 &&
                (camera.Position + camera.Up * distance).z < objects[i].pos.z + objects[i].size.z / 2) {
                return true;
            }
        }
        else if (direction == "down") {
            if ((camera.Position - camera.Up * distance).x > objects[i].pos.x - objects[i].size.x / 2 &&
                (camera.Position - camera.Up * distance).x < objects[i].pos.x + objects[i].size.x / 2 &&
                (camera.Position - camera.Up * distance).y > objects[i].pos.y - objects[i].size.y / 2 &&
                (camera.Position - camera.Up * distance).y < objects[i].pos.y + objects[i].size.y / 2 &&
                (camera.Position - camera.Up * distance).z > objects[i].pos.z - objects[i].size.z / 2 &&
                (camera.Position - camera.Up * distance).z < objects[i].pos.z + objects[i].size.z / 2) {
                return true;
            }
        }
    }
    return false;
}

bool checkFinish() {
    glm::vec3 pos = lightPos[0];
    
    bool collisionX = camera.Position.x + 1 >= pos.x && pos.x + 0.2f >= camera.Position.x;
    bool collisionY = camera.Position.y + 1 >= pos.y && pos.y + 0.2f >= camera.Position.y;
    bool collisionZ = camera.Position.z + 1 >= pos.z && pos.z + 0.2f >= camera.Position.z;

    return collisionX && collisionY && collisionZ;
}

bool checkSpiderCollision() {
    glm::vec3 pos = spiderPos;

    bool collisionX = camera.Position.x + 1 >= pos.x && pos.x + 1.25f >= camera.Position.x;
    bool collisionY = camera.Position.y + 1 >= pos.y && pos.y + 0.75f >= camera.Position.y;
    bool collisionZ = camera.Position.z + 1 >= pos.z && pos.z + 1.25f >= camera.Position.z;

    return collisionX && collisionY && collisionZ;
}

bool checkGhostCollision() {
    for (int i = 0; i < ghostPos.size(); i++) {
        glm::vec3 pos = ghostPos[i];
        
        bool collisionX = camera.Position.x + 1 >= pos.x && pos.x + 0.75f >= camera.Position.x;
        bool collisionY = camera.Position.y + 1 >= pos.y && pos.y + 1.0f >= camera.Position.y;
        bool collisionZ = camera.Position.z + 1 >= pos.z && pos.z + 0.75f >= camera.Position.z;

        if (collisionX && collisionY && collisionZ)
            return true;
    }
    return false;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && !checkCollision(objects, "front", 0.25))
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !checkCollision(objects, "back", 0.25))
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && !checkCollision(objects, "left", 0.25))
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && !checkCollision(objects, "right", 0.25))
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !checkCollision(objects, "up", 1))
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && !checkCollision(objects, "down", 1))
        camera.ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        spiderPos = lightPos[0];
}

void compMap() {
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (grid[XYToIndex(x, y)] == ' ') {
                gameObject temp(glm::vec3(x * 3, -1, y * 3), 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
                objects.push_back(temp);

                temp = *new gameObject(glm::vec3(x * 3 - 1, -1, y * 3), 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
                objects.push_back(temp);

                temp = *new gameObject(glm::vec3(x * 3 + 1, -1, y * 3), 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
                objects.push_back(temp);

                temp = *new gameObject(glm::vec3(x * 3, -1, y * 3 - 1), 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
                objects.push_back(temp);

                temp = *new gameObject(glm::vec3(x * 3 - 1, -1, y * 3 - 1), 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
                objects.push_back(temp);

                temp = *new gameObject(glm::vec3(x * 3 + 1, -1, y * 3 - 1), 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
                objects.push_back(temp);

                temp = *new gameObject(glm::vec3(x * 3, -1, y * 3 + 1), 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
                objects.push_back(temp);

                temp = *new gameObject(glm::vec3(x * 3 - 1, -1, y * 3 + 1), 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
                objects.push_back(temp);

                temp = *new gameObject(glm::vec3(x * 3 + 1, -1, y * 3 + 1), 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
                objects.push_back(temp);

                // walls

                if (grid[XYToIndex(x, y + 1)] != ' ') {
                    gameObject temp(glm::vec3(x * 3, 0, y * 3 + 2), 0, glm::vec3(0.0f, 0.0f, 1.0f), "wall");
                    objects.push_back(temp);

                    temp = *new gameObject(glm::vec3(x * 3 + 1, 0, y * 3 + 2), 0, glm::vec3(0.0f, 0.0f, 1.0f), "wall");
                    objects.push_back(temp);

                    temp = *new gameObject(glm::vec3(x * 3 - 1, 0, y * 3 + 2), 0, glm::vec3(0.0f, 0.0f, 1.0f), "wall");
                    objects.push_back(temp);

                    temp = *new gameObject(glm::vec3(x * 3, 1, y * 3 + 2), 0, glm::vec3(0.0f, 0.0f, 1.0f), "wall");
                    objects.push_back(temp);

                    temp = *new gameObject(glm::vec3(x * 3 + 1, 1, y * 3 + 2), 0, glm::vec3(0.0f, 0.0f, 1.0f), "wall");
                    objects.push_back(temp);

                    temp = *new gameObject(glm::vec3(x * 3 - 1, 1, y * 3 + 2), 0, glm::vec3(0.0f, 0.0f, 1.0f), "wall");
                    objects.push_back(temp);
                }
                if (grid[XYToIndex(x, y - 1)] != ' ') {
                    gameObject temp(glm::vec3(x * 3, 0, y * 3 - 2), 180.0f, glm::vec3(0.0f, 1.0f, 0.0f), "wall");
                    objects.push_back(temp);

                    temp = *new gameObject(glm::vec3(x * 3 + 1, 0, y * 3 - 2), 180.0f, glm::vec3(0.0f, 1.0f, 0.0f), "wall");
                    objects.push_back(temp);

                    temp = *new gameObject(glm::vec3(x * 3 - 1, 0, y * 3 - 2), 180.0f, glm::vec3(0.0f, 1.0f, 0.0f), "wall");
                    objects.push_back(temp);

                    temp = *new gameObject(glm::vec3(x * 3, 1, y * 3 - 2), 180.0f, glm::vec3(0.0f, 1.0f, 0.0f), "wall");
                    objects.push_back(temp);

                    temp = *new gameObject(glm::vec3(x * 3 + 1, 1, y * 3 - 2), 180.0f, glm::vec3(0.0f, 1.0f, 0.0f), "wall");
                    objects.push_back(temp);

                    temp = *new gameObject(glm::vec3(x * 3 - 1, 1, y * 3 - 2), 180.0f, glm::vec3(0.0f, 1.0f, 0.0f), "wall");
                    objects.push_back(temp);
                }
                if (grid[XYToIndex(x + 1, y)] != ' ') {
                    gameObject temp(glm::vec3(x * 3 + 2, 0, y * 3), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f), "wall");
                    objects.push_back(temp);

                    temp = *new gameObject(glm::vec3(x * 3 + 2, 0, y * 3 + 1), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f), "wall");
                    objects.push_back(temp);

                    temp = *new gameObject(glm::vec3(x * 3 + 2, 0, y * 3 - 1), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f), "wall");
                    objects.push_back(temp);

                    temp = *new gameObject(glm::vec3(x * 3 + 2, 1, y * 3), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f), "wall");
                    objects.push_back(temp);

                    temp = *new gameObject(glm::vec3(x * 3 + 2, 1, y * 3 + 1), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f), "wall");
                    objects.push_back(temp);

                    temp = *new gameObject(glm::vec3(x * 3 + 2, 1, y * 3 - 1), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f), "wall");
                    objects.push_back(temp);
                }
                if (grid[XYToIndex(x - 1, y)] != ' ') {
                    gameObject temp(glm::vec3(x * 3 - 2, 0, y * 3), 270.0f, glm::vec3(0.0f, 1.0f, 0.0f), "wall");
                    objects.push_back(temp);

                    temp = *new gameObject(glm::vec3(x * 3 - 2, 0, y * 3 + 1), 270.0f, glm::vec3(0.0f, 1.0f, 0.0f), "wall");
                    objects.push_back(temp);

                    temp = *new gameObject(glm::vec3(x * 3 - 2, 0, y * 3 - 1), 270.0f, glm::vec3(0.0f, 1.0f, 0.0f), "wall");
                    objects.push_back(temp);

                    temp = *new gameObject(glm::vec3(x * 3 - 2, 1, y * 3), 270.0f, glm::vec3(0.0f, 1.0f, 0.0f), "wall");
                    objects.push_back(temp);

                    temp = *new gameObject(glm::vec3(x * 3 - 2, 1, y * 3 + 1), 270.0f, glm::vec3(0.0f, 1.0f, 0.0f), "wall");
                    objects.push_back(temp);

                    temp = *new gameObject(glm::vec3(x * 3 - 2, 1, y * 3 - 1), 270.0f, glm::vec3(0.0f, 1.0f, 0.0f), "wall");
                    objects.push_back(temp);
                }
                
            }
        }
    }

    for (int y = 0; y < GRID_HEIGHT; y++) {
        if (grid[XYToIndex(GRID_WIDTH - 2, y)] == ' ') {
            lightPos.push_back(glm::vec3((GRID_WIDTH - 2) * 3, 0.5f, y * 3));
            spiderPos = glm::vec3((GRID_WIDTH - 2) * 3, 0, y * 3);
            break;
        }
    }
}

void gravity() {
    if(!checkCollision(objects, "front", 1) && !checkCollision(objects, "back", 1) && !checkCollision(objects, "down", 1))
        camera.Position -= glm::vec3(0.0f, 1.0f, 0.0f) * deltaTime;
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void ResetGrid()
{
    // Fills the grid with walls ('#' characters).
    for (int i = 0; i < GRID_WIDTH * GRID_HEIGHT; ++i)
    {
        grid[i] = '#';
    }
}
int XYToIndex(int x, int y)
{
    // Converts the two-dimensional index pair (x,y) into a
    // single-dimensional index. The result is y * ROW_WIDTH + x.
    return y * GRID_WIDTH + x;
}

int XYToIndex3(int x, int y) {
    return y * (GRID_WIDTH * 3) + x;
}

void PrintGrid3() {
    for (int y = 0; y < GRID_HEIGHT * 3; ++y)
    {
        for (int x = 0; x < GRID_WIDTH * 3; ++x)
        {
            cout << grid_map[XYToIndex3(x, y)];
        }
        cout << endl;
    }
}

int IsInBounds(int x, int y)
{
    // Returns "true" if x and y are both in-bounds.
    if (x < 0 || x >= GRID_WIDTH) return false;
    if (y < 0 || y >= GRID_HEIGHT) return false;
    return true;
}
// This is the recursive function we will code in the next project
void Visit(int x, int y)
{
    // Starting at the given index, recursively visits every direction in a
    // randomized order.
    // Set my current location to be an empty passage.
    grid[XYToIndex(x, y)] = ' ';
    // Create an local array containing the 4 directions and shuffle their order.
    int dirs[4];
    dirs[0] = NORTH;
    dirs[1] = EAST;
    dirs[2] = SOUTH;
    dirs[3] = WEST;
    for (int i = 0; i < 4; ++i)
    {
        int r = rand() & 3;
        int temp = dirs[r];
        dirs[r] = dirs[i];
        dirs[i] = temp;
    }
    // Loop through every direction and attempt to Visit that direction.
    for (int i = 0; i < 4; ++i)
    {
        // dx,dy are offsets from current location. Set them based
        // on the next direction I wish to try.
        int dx = 0, dy = 0;
        switch (dirs[i])
        {
        case NORTH: dy = -1; break;
        case SOUTH: dy = 1; break;
        case EAST: dx = 1; break;
        case WEST: dx = -1; break;
        }
        // Find the (x,y) coordinates of the grid cell 2 spots
        // away in the given direction.
        int x2 = x + (dx << 1);
        int y2 = y + (dy << 1);
        if (IsInBounds(x2, y2))
        {
            if (grid[XYToIndex(x2, y2)] == '#')
            {
                // (x2,y2) has not been visited yet... knock down the
                // wall between my current position and that position
                grid[XYToIndex(x2 - dx, y2 - dy)] = ' ';
                // Recursively Visit (x2,y2)
                Visit(x2, y2);
            }
        }
    }
}

void PrintGrid()
{
    for (int i = 0; i < GRID_HEIGHT * 3; i++) {
        for (int j = 0; j < GRID_WIDTH * 3; j++) {
            grid_map[XYToIndex3(j, i)] = '#';
        }
    }

    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            if (grid[XYToIndex(j, i)] == ' ') {
                for (int k = 0; k < 3; k++) {
                    grid_map[XYToIndex3(j * 3, i * 3 - 1 + k)] = ' ';
                    grid_map[XYToIndex3(j * 3 - 1, i * 3 - 1 + k)] = ' ';
                    grid_map[XYToIndex3(j * 3 + 1, i * 3 - 1 + k)] = ' ';
                }
            }
        }
    }

    PrintGrid3();

    // Displays the finished maze to the screen.
    for (int y = 0; y < GRID_HEIGHT; ++y)
    {
        for (int x = 0; x < GRID_WIDTH; ++x)
        {
            cout << grid[XYToIndex(x, y)];
        }
        cout << endl;
    }
}

void computePath(vector <pair <int, int>> path, int x, int y, vector <pair <int, int>> & resultPath) {

    int cx = path[path.size() - 1].first, cy = path[path.size() - 1].second;
    if (cx == x && cy == y) {
        resultPath = path;
        return;
    }

    int bcx = -1, bcy = -1;
    if (path.size() != 1) {
        bcx = path[path.size() - 2].first; 
        bcy = path[path.size() - 2].second;
    }

    if (cx + 1 < GRID_WIDTH && grid[XYToIndex(cx + 1, cy)] != '#' && (bcx == -1 || (bcx != -1 && !(bcx == cx + 1 && bcy == cy)))) {
        path.push_back(make_pair(cx + 1, cy));
        computePath(path, x, y, resultPath);
        path.pop_back();
    }
    if (grid[XYToIndex(cx - 1, cy)] != '#' && (bcx == -1 || (bcx != -1 && !(bcx == cx - 1 && bcy == cy)))) {
        path.push_back(make_pair(cx - 1, cy));
        computePath(path, x, y, resultPath);
        path.pop_back();
    }
    if (grid[XYToIndex(cx, cy + 1)] != '#' && (bcx == -1 || (bcx != -1 && !(bcx == cx && bcy == cy + 1)))) {
        path.push_back(make_pair(cx, cy + 1));
        computePath(path, x, y, resultPath);
        path.pop_back();
    }
    if (grid[XYToIndex(cx, cy - 1)] != '#' && (bcx == -1 || (bcx != -1 && !(bcx == cx && bcy == cy - 1)))) {
        path.push_back(make_pair(cx, cy - 1));
        computePath(path, x, y, resultPath);
        path.pop_back();
    }
    
    return;
}


// astar

// A Utility Function to check whether given cell (row, col)
// is a valid cell or not.
bool isValid(int row, int col)
{
    // Returns true if row number and column number
    // is in range
    return (row >= 0) && (row < GRID_HEIGHT * 3) && (col >= 0)
        && (col < GRID_WIDTH * 3);
}

// A Utility Function to check whether the given cell is
// blocked or not
bool isUnBlocked(int row, int col)
{
    // Returns true if the cell is not blocked else false
    if (grid_map[XYToIndex3(col, row)] == ' ')
        return (true);
    else
        return (false);
}

// A Utility Function to check whether destination cell has
// been reached or not
bool isDestination(int row, int col, Pair dest)
{
    if (row == dest.first && col == dest.second)
        return (true);
    else
        return (false);
}

// A Utility Function to calculate the 'h' heuristics.
double calculateHValue(int row, int col, Pair dest)
{
    // Return using the distance formula
    return ((double)sqrt(
        (row - dest.first) * (row - dest.first)
        + (col - dest.second) * (col - dest.second)));
}

// A Utility Function to trace the path from the source
// to destination
void tracePath(cell cellDetails[][GRID_WIDTH * 3], Pair dest)
{
    //printf("\nThe Path is ");
    int row = dest.first;
    int col = dest.second;

    stack<Pair> Path;

    while (!(cellDetails[row][col].parent_i == row
        && cellDetails[row][col].parent_j == col)) {
        Path.push(make_pair(row, col));
        int temp_row = cellDetails[row][col].parent_i;
        int temp_col = cellDetails[row][col].parent_j;
        row = temp_row;
        col = temp_col;
    }

    Path.push(make_pair(row, col));
    spiderPath = Path;
    lastPath = spiderPath;
    pcX = dest.second;
    pcY = dest.first;
    while (!Path.empty()) {
        pair<int, int> p = Path.top();
        Path.pop();
        //printf("-> (%d,%d) ", p.first, p.second);
    }



    return;
}

// A Function to find the shortest path between
// a given source cell to a destination cell according
// to A* Search Algorithm
void aStarSearch(Pair src, Pair dest)
{
    // If the source is out of range
    if (isValid(src.first, src.second) == false) {
        printf("Source is invalid\n");
        stack<Pair> Path;
        spiderPath = Path;
        return;
    }

    // If the destination is out of range
    if (isValid(dest.first, dest.second) == false) {
        printf("Destination is invalid\n");
        stack<Pair> Path;
        spiderPath = Path;
        return;
    }

    // Either the source or the destination is blocked
    if (isUnBlocked(src.first, src.second) == false
        || isUnBlocked(dest.first, dest.second)
        == false) {

        if (isUnBlocked(dest.first, dest.second) == false) {
            dest.first = pcY;
            dest.second = pcX;
        }
        else {
            printf("Source or the destination is blocked\n");
            stack<Pair> Path;
            spiderPath = Path;
            return;
        }
    }

    // If the destination cell is the same as source cell
    if (isDestination(src.first, src.second, dest)
        == true) {
        printf("We are already at the destination\n");
        stack<Pair> Path;
        spiderPath = Path;
        return;
    }

    // Create a closed list and initialise it to false which
    // means that no cell has been included yet This closed
    // list is implemented as a boolean 2D array
    bool closedList[GRID_HEIGHT * 3][GRID_WIDTH * 3];
    memset(closedList, false, sizeof(closedList));

    // Declare a 2D array of structure to hold the details
    // of that cell
    cell cellDetails[GRID_HEIGHT * 3][GRID_WIDTH * 3];

    int i, j;

    for (i = 0; i < GRID_HEIGHT * 3; i++) {
        for (j = 0; j < GRID_WIDTH * 3; j++) {
            cellDetails[i][j].f = FLT_MAX;
            cellDetails[i][j].g = FLT_MAX;
            cellDetails[i][j].h = FLT_MAX;
            cellDetails[i][j].parent_i = -1;
            cellDetails[i][j].parent_j = -1;
        }
    }

    // Initialising the parameters of the starting node
    i = src.first, j = src.second;
    cellDetails[i][j].f = 0.0;
    cellDetails[i][j].g = 0.0;
    cellDetails[i][j].h = 0.0;
    cellDetails[i][j].parent_i = i;
    cellDetails[i][j].parent_j = j;

    /*
     Create an open list having information as-
     <f, <i, j>>
     where f = g + h,
     and i, j are the row and column index of that cell
     Note that 0 <= i <= ROW-1 & 0 <= j <= COL-1
     This open list is implemented as a set of pair of
     pair.*/
    set<pPair> openList;

    // Put the starting cell on the open list and set its
    // 'f' as 0
    openList.insert(make_pair(0.0, make_pair(i, j)));

    // We set this boolean value as false as initially
    // the destination is not reached.
    bool foundDest = false;

    while (!openList.empty()) {
        pPair p = *openList.begin();

        // Remove this vertex from the open list
        openList.erase(openList.begin());

        // Add this vertex to the closed list
        i = p.second.first;
        j = p.second.second;
        closedList[i][j] = true;

        /*
         Generating all the 8 successor of this cell

             N.W   N   N.E
               \   |   /
                \  |  /
             W----Cell----E
                  / | \
                /   |  \
             S.W    S   S.E

         Cell-->Popped Cell (i, j)
         N -->  North       (i-1, j)
         S -->  South       (i+1, j)
         E -->  East        (i, j+1)
         W -->  West           (i, j-1)
         N.E--> North-East  (i-1, j+1)
         N.W--> North-West  (i-1, j-1)
         S.E--> South-East  (i+1, j+1)
         S.W--> South-West  (i+1, j-1)*/

         // To store the 'g', 'h' and 'f' of the 8 successors
        double gNew, hNew, fNew;

        //----------- 1st Successor (North) ------------

        // Only process this cell if this is a valid one
        if (isValid(i - 1, j) == true) {
            // If the destination cell is the same as the
            // current successor
            if (isDestination(i - 1, j, dest) == true) {
                // Set the Parent of the destination cell
                cellDetails[i - 1][j].parent_i = i;
                cellDetails[i - 1][j].parent_j = j;
                printf("The destination cell is found\n");
                tracePath(cellDetails, dest);
                foundDest = true;
                return;
            }
            // If the successor is already on the closed
            // list or if it is blocked, then ignore it.
            // Else do the following
            else if (closedList[i - 1][j] == false
                && isUnBlocked(i - 1, j)
                == true) {
                gNew = cellDetails[i][j].g + 1.0;
                hNew = calculateHValue(i - 1, j, dest);
                fNew = gNew + hNew;

                // If it isn’t on the open list, add it to
                // the open list. Make the current square
                // the parent of this square. Record the
                // f, g, and h costs of the square cell
                //                OR
                // If it is on the open list already, check
                // to see if this path to that square is
                // better, using 'f' cost as the measure.
                if (cellDetails[i - 1][j].f == FLT_MAX
                    || cellDetails[i - 1][j].f > fNew) {
                    openList.insert(make_pair(
                        fNew, make_pair(i - 1, j)));

                    // Update the details of this cell
                    cellDetails[i - 1][j].f = fNew;
                    cellDetails[i - 1][j].g = gNew;
                    cellDetails[i - 1][j].h = hNew;
                    cellDetails[i - 1][j].parent_i = i;
                    cellDetails[i - 1][j].parent_j = j;
                }
            }
        }

        //----------- 2nd Successor (South) ------------

        // Only process this cell if this is a valid one
        if (isValid(i + 1, j) == true) {
            // If the destination cell is the same as the
            // current successor
            if (isDestination(i + 1, j, dest) == true) {
                // Set the Parent of the destination cell
                cellDetails[i + 1][j].parent_i = i;
                cellDetails[i + 1][j].parent_j = j;
                printf("The destination cell is found\n");
                tracePath(cellDetails, dest);
                foundDest = true;
                return;
            }
            // If the successor is already on the closed
            // list or if it is blocked, then ignore it.
            // Else do the following
            else if (closedList[i + 1][j] == false
                && isUnBlocked(i + 1, j)
                == true) {
                gNew = cellDetails[i][j].g + 1.0;
                hNew = calculateHValue(i + 1, j, dest);
                fNew = gNew + hNew;

                // If it isn’t on the open list, add it to
                // the open list. Make the current square
                // the parent of this square. Record the
                // f, g, and h costs of the square cell
                //                OR
                // If it is on the open list already, check
                // to see if this path to that square is
                // better, using 'f' cost as the measure.
                if (cellDetails[i + 1][j].f == FLT_MAX
                    || cellDetails[i + 1][j].f > fNew) {
                    openList.insert(make_pair(
                        fNew, make_pair(i + 1, j)));
                    // Update the details of this cell
                    cellDetails[i + 1][j].f = fNew;
                    cellDetails[i + 1][j].g = gNew;
                    cellDetails[i + 1][j].h = hNew;
                    cellDetails[i + 1][j].parent_i = i;
                    cellDetails[i + 1][j].parent_j = j;
                }
            }
        }

        //----------- 3rd Successor (East) ------------

        // Only process this cell if this is a valid one
        if (isValid(i, j + 1) == true) {
            // If the destination cell is the same as the
            // current successor
            if (isDestination(i, j + 1, dest) == true) {
                // Set the Parent of the destination cell
                cellDetails[i][j + 1].parent_i = i;
                cellDetails[i][j + 1].parent_j = j;
                printf("The destination cell is found\n");
                tracePath(cellDetails, dest);
                foundDest = true;
                return;
            }

            // If the successor is already on the closed
            // list or if it is blocked, then ignore it.
            // Else do the following
            else if (closedList[i][j + 1] == false
                && isUnBlocked(i, j + 1)
                == true) {
                gNew = cellDetails[i][j].g + 1.0;
                hNew = calculateHValue(i, j + 1, dest);
                fNew = gNew + hNew;

                // If it isn’t on the open list, add it to
                // the open list. Make the current square
                // the parent of this square. Record the
                // f, g, and h costs of the square cell
                //                OR
                // If it is on the open list already, check
                // to see if this path to that square is
                // better, using 'f' cost as the measure.
                if (cellDetails[i][j + 1].f == FLT_MAX
                    || cellDetails[i][j + 1].f > fNew) {
                    openList.insert(make_pair(
                        fNew, make_pair(i, j + 1)));

                    // Update the details of this cell
                    cellDetails[i][j + 1].f = fNew;
                    cellDetails[i][j + 1].g = gNew;
                    cellDetails[i][j + 1].h = hNew;
                    cellDetails[i][j + 1].parent_i = i;
                    cellDetails[i][j + 1].parent_j = j;
                }
            }
        }

        //----------- 4th Successor (West) ------------

        // Only process this cell if this is a valid one
        if (isValid(i, j - 1) == true) {
            // If the destination cell is the same as the
            // current successor
            if (isDestination(i, j - 1, dest) == true) {
                // Set the Parent of the destination cell
                cellDetails[i][j - 1].parent_i = i;
                cellDetails[i][j - 1].parent_j = j;
                printf("The destination cell is found\n");
                tracePath(cellDetails, dest);
                foundDest = true;
                return;
            }

            // If the successor is already on the closed
            // list or if it is blocked, then ignore it.
            // Else do the following
            else if (closedList[i][j - 1] == false
                && isUnBlocked(i, j - 1)
                == true) {
                gNew = cellDetails[i][j].g + 1.0;
                hNew = calculateHValue(i, j - 1, dest);
                fNew = gNew + hNew;

                // If it isn’t on the open list, add it to
                // the open list. Make the current square
                // the parent of this square. Record the
                // f, g, and h costs of the square cell
                //                OR
                // If it is on the open list already, check
                // to see if this path to that square is
                // better, using 'f' cost as the measure.
                if (cellDetails[i][j - 1].f == FLT_MAX
                    || cellDetails[i][j - 1].f > fNew) {
                    openList.insert(make_pair(
                        fNew, make_pair(i, j - 1)));

                    // Update the details of this cell
                    cellDetails[i][j - 1].f = fNew;
                    cellDetails[i][j - 1].g = gNew;
                    cellDetails[i][j - 1].h = hNew;
                    cellDetails[i][j - 1].parent_i = i;
                    cellDetails[i][j - 1].parent_j = j;
                }
            }
        }

        //----------- 5th Successor (North-East)
        //------------

        // Only process this cell if this is a valid one
        if (isValid(i - 1, j + 1) == true) {
            // If the destination cell is the same as the
            // current successor
            if (isDestination(i - 1, j + 1, dest) == true) {
                // Set the Parent of the destination cell
                cellDetails[i - 1][j + 1].parent_i = i;
                cellDetails[i - 1][j + 1].parent_j = j;
                printf("The destination cell is found\n");
                tracePath(cellDetails, dest);
                foundDest = true;
                return;
            }

            // If the successor is already on the closed
            // list or if it is blocked, then ignore it.
            // Else do the following
            else if (closedList[i - 1][j + 1] == false
                && isUnBlocked(i - 1, j + 1)
                == true) {
                gNew = cellDetails[i][j].g + 1.414;
                hNew = calculateHValue(i - 1, j + 1, dest);
                fNew = gNew + hNew;

                // If it isn’t on the open list, add it to
                // the open list. Make the current square
                // the parent of this square. Record the
                // f, g, and h costs of the square cell
                //                OR
                // If it is on the open list already, check
                // to see if this path to that square is
                // better, using 'f' cost as the measure.
                if (cellDetails[i - 1][j + 1].f == FLT_MAX
                    || cellDetails[i - 1][j + 1].f > fNew) {
                    openList.insert(make_pair(
                        fNew, make_pair(i - 1, j + 1)));

                    // Update the details of this cell
                    cellDetails[i - 1][j + 1].f = fNew;
                    cellDetails[i - 1][j + 1].g = gNew;
                    cellDetails[i - 1][j + 1].h = hNew;
                    cellDetails[i - 1][j + 1].parent_i = i;
                    cellDetails[i - 1][j + 1].parent_j = j;
                }
            }
        }

        //----------- 6th Successor (North-West)
        //------------

        // Only process this cell if this is a valid one
        if (isValid(i - 1, j - 1) == true) {
            // If the destination cell is the same as the
            // current successor
            if (isDestination(i - 1, j - 1, dest) == true) {
                // Set the Parent of the destination cell
                cellDetails[i - 1][j - 1].parent_i = i;
                cellDetails[i - 1][j - 1].parent_j = j;
                printf("The destination cell is found\n");
                tracePath(cellDetails, dest);
                foundDest = true;
                return;
            }

            // If the successor is already on the closed
            // list or if it is blocked, then ignore it.
            // Else do the following
            else if (closedList[i - 1][j - 1] == false
                && isUnBlocked(i - 1, j - 1)
                == true) {
                gNew = cellDetails[i][j].g + 1.414;
                hNew = calculateHValue(i - 1, j - 1, dest);
                fNew = gNew + hNew;

                // If it isn’t on the open list, add it to
                // the open list. Make the current square
                // the parent of this square. Record the
                // f, g, and h costs of the square cell
                //                OR
                // If it is on the open list already, check
                // to see if this path to that square is
                // better, using 'f' cost as the measure.
                if (cellDetails[i - 1][j - 1].f == FLT_MAX
                    || cellDetails[i - 1][j - 1].f > fNew) {
                    openList.insert(make_pair(
                        fNew, make_pair(i - 1, j - 1)));
                    // Update the details of this cell
                    cellDetails[i - 1][j - 1].f = fNew;
                    cellDetails[i - 1][j - 1].g = gNew;
                    cellDetails[i - 1][j - 1].h = hNew;
                    cellDetails[i - 1][j - 1].parent_i = i;
                    cellDetails[i - 1][j - 1].parent_j = j;
                }
            }
        }

        //----------- 7th Successor (South-East)
        //------------

        // Only process this cell if this is a valid one
        if (isValid(i + 1, j + 1) == true) {
            // If the destination cell is the same as the
            // current successor
            if (isDestination(i + 1, j + 1, dest) == true) {
                // Set the Parent of the destination cell
                cellDetails[i + 1][j + 1].parent_i = i;
                cellDetails[i + 1][j + 1].parent_j = j;
                printf("The destination cell is found\n");
                tracePath(cellDetails, dest);
                foundDest = true;
                return;
            }

            // If the successor is already on the closed
            // list or if it is blocked, then ignore it.
            // Else do the following
            else if (closedList[i + 1][j + 1] == false
                && isUnBlocked(i + 1, j + 1)
                == true) {
                gNew = cellDetails[i][j].g + 1.414;
                hNew = calculateHValue(i + 1, j + 1, dest);
                fNew = gNew + hNew;

                // If it isn’t on the open list, add it to
                // the open list. Make the current square
                // the parent of this square. Record the
                // f, g, and h costs of the square cell
                //                OR
                // If it is on the open list already, check
                // to see if this path to that square is
                // better, using 'f' cost as the measure.
                if (cellDetails[i + 1][j + 1].f == FLT_MAX
                    || cellDetails[i + 1][j + 1].f > fNew) {
                    openList.insert(make_pair(
                        fNew, make_pair(i + 1, j + 1)));

                    // Update the details of this cell
                    cellDetails[i + 1][j + 1].f = fNew;
                    cellDetails[i + 1][j + 1].g = gNew;
                    cellDetails[i + 1][j + 1].h = hNew;
                    cellDetails[i + 1][j + 1].parent_i = i;
                    cellDetails[i + 1][j + 1].parent_j = j;
                }
            }
        }

        //----------- 8th Successor (South-West)
        //------------

        // Only process this cell if this is a valid one
        if (isValid(i + 1, j - 1) == true) {
            // If the destination cell is the same as the
            // current successor
            if (isDestination(i + 1, j - 1, dest) == true) {
                // Set the Parent of the destination cell
                cellDetails[i + 1][j - 1].parent_i = i;
                cellDetails[i + 1][j - 1].parent_j = j;
                printf("The destination cell is found\n");
                tracePath(cellDetails, dest);
                foundDest = true;
                return;
            }

            // If the successor is already on the closed
            // list or if it is blocked, then ignore it.
            // Else do the following
            else if (closedList[i + 1][j - 1] == false
                && isUnBlocked(i + 1, j - 1)
                == true) {
                gNew = cellDetails[i][j].g + 1.414;
                hNew = calculateHValue(i + 1, j - 1, dest);
                fNew = gNew + hNew;

                // If it isn’t on the open list, add it to
                // the open list. Make the current square
                // the parent of this square. Record the
                // f, g, and h costs of the square cell
                //                OR
                // If it is on the open list already, check
                // to see if this path to that square is
                // better, using 'f' cost as the measure.
                if (cellDetails[i + 1][j - 1].f == FLT_MAX
                    || cellDetails[i + 1][j - 1].f > fNew) {
                    openList.insert(make_pair(
                        fNew, make_pair(i + 1, j - 1)));

                    // Update the details of this cell
                    cellDetails[i + 1][j - 1].f = fNew;
                    cellDetails[i + 1][j - 1].g = gNew;
                    cellDetails[i + 1][j - 1].h = hNew;
                    cellDetails[i + 1][j - 1].parent_i = i;
                    cellDetails[i + 1][j - 1].parent_j = j;
                }
            }
        }
    }

    // When the destination cell is not found and the open
    // list is empty, then we conclude that we failed to
    // reach the destination cell. This may happen when the
    // there is no way to destination cell (due to
    // blockages)
    if (foundDest == false)
        printf("Failed to find the Destination Cell\n");

    return;
}

