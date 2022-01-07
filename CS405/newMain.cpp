#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "camera.h"
#include "entity.h"

#include <windows.h>
#include <mmsystem.h>
#include <mciapi.h>
//these two headers are already included in the <Windows.h> header
#pragma comment(lib, "Winmm.lib")

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

struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
void computeMap();
void compMap();
void gravity();
bool checkFinish();
void RenderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color, unsigned int VBO, unsigned int VAO);

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
//----FUNCTION PROTOTYPES---------------------------------------------
void ResetGrid();
int XYToIndex(int x, int y);
int IsInBounds(int x, int y);
void Visit(int x, int y);
void PrintGrid();
/////////////////////////////////////////////////////////////////////

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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader lightingShader("colors.vert", "colors.frag");
    Shader lightCubeShader("light_cube.vert", "light_cube.frag");
    Shader ourShader("vertex.vert", "frag.frag");

    Shader textShader("text.vert", "text.frag");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    textShader.use();
    glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Load Shader
    Model ourModel("backpack.obj");

    Model ourModel2("scene.gltf");
    

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

    // Maze Generation
    srand(time(0));
    ResetGrid();
    Visit(1, 1);
    compMap();
    PrintGrid();
    //computeMap();
    /*****************/

    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f,  0.2f,  2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3(0.0f,  5.0f, -3.0f)
    };

    // FreeType
    // --------
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    // find path to font
    std::string font_name("arial.ttf");
    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        return -1;
    }

    // load font as face
    FT_Face face;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

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

    //text VAO
    unsigned int textVAO, textVBO;
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

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
    
    PlaySound(L"song.wav", NULL, SND_ASYNC);
    
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {

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

        RenderText(textShader, "This is sample text", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f), textVBO, textVAO);
        RenderText(textShader, "(C) LearnOpenGL.com", 540.0f, 570.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f), textVBO, textVAO);

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
        glm::mat4 model2 = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);
        lightingShader.setMat4("model", model2);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 10.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down
        lightingShader.setMat4("model", model);
        ourModel.Draw(lightingShader);

        model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model2 = glm::scale(model2, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        lightingShader.setMat4("model", model2);
        ourModel2.Draw(lightingShader);

        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        // render containers
        //glBindVertexArray(cubeVAO);
        //for (unsigned int i = 0; i < 1; i++)
        //{
        //    // calculate the model matrix for each object and pass it to shader before drawing
        //    glm::mat4 model = glm::mat4(1.0f);
        //    model = glm::translate(model, glm::vec3(0.0f, -10.0f, 0.0f));
        //    lightingShader.setMat4("model", model);

        //    glDrawArrays(GL_TRIANGLES, 0, 36);
        //}


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
            objects.clear();
            lightPos.clear();
            compMap();
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

// render line of text
// -------------------
void RenderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color, unsigned int VBO, unsigned int VAO)
{
    // activate corresponding render state	
    shader.use();
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}