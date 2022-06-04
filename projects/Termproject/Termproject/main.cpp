#include <string>

#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cmath>
#include <vector>

#include <shader.h>
#include <plane.h>
#include <arcball.h>
#include <Model.h>
#include <cube.h>
#include <skybox.h> //skybox
#include <text.h> //text
#include <keyframe.h>

// sounds
#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#define SOUND_FILE_NAME "sounds/bgsound.wav"
#define SOUND_FILE_NAME_SUCCESS "sounds/success.wav"

using namespace std;

// Function Prototypes
GLFWwindow *glAllInit(const char *title);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action , int mods);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow *window, double x, double y);
unsigned int loadTexture(const char *path, bool vflip);
unsigned int loadCubemap(vector<std::string> faces);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void render();
void initKeyframes();
void updateAnimData();
void initKeyframes2();
void updateAnimData2();

// Global variables
GLFWwindow *mainWindow = NULL;
unsigned int SCR_WIDTH = 600;
unsigned int SCR_HEIGHT = 600;

Shader* lightingShader;
Shader *textureShader;

Cube* cube;
Plane *plane;
Shader* skyboxShader; //skybox
SkyBox* skybox; //skybox

Shader* modelShader;
Model* planetModel;
Model* fishModel;
Model* buddhaModel;
Model* panelModel;
Model* diverModel;

Shader* textShader; //text
Text* text; //text

glm::mat4 projection, view, model;

// for camera
glm::vec3 cameraOrigPos(0.0f, 5.0f, 30.0f);
glm::vec3 cameraPos;
glm::vec3 camTarget(0.0f, 0.0f, 0.0f);
glm::vec3 camUp(0.0f, 1.0f, 0.0f);
glm::vec3 modelPan(0.0f, 0.0f, 0.0f);

// for arcball
float arcballSpeed = 0.1f;
static Arcball camArcBall(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true);
static Arcball modelArcBall(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true);
bool arcballCamRot = true;

// for texture
unsigned int floorTexture;
unsigned int transparentTexture;
unsigned int cubemapTexture;
unsigned int containerTexture;

// vegetation locations
vector<glm::vec3> vegetation;

// for fish
unsigned int amount = 100;
glm::mat4* modelMatrices;

// for lighting
glm::vec3 lightSize(0.5f, 0.5f, 0.5f);

// for animation1
enum RenderMode { INIT, ANIM, STOP };
RenderMode renderMode;                  // current rendering mode
float beginT;                           // animation beginning time (in sec)
float timeT;                            // current time (in sec)
float animEndTime = 40.0f;               // ending time of animation (in sec)
float xTrans, yTrans, zTrans;           // current translation factors
float xAngle, yAngle, zAngle;           // current rotation factors
KeyFraming xTKF(4), yTKF(4), zTKF(4);   // translation keyframes
KeyFraming xRKF(4), yRKF(4), zRKF(4);   // rotation keyframes

// for animation2                
float beginT2;                           // animation beginning time (in sec)
float timeT2;                            // current time (in sec)
float xTrans2, yTrans2, zTrans2;           // current translation factors
float xAngle2, yAngle2, zAngle2;           // current rotation factors
KeyFraming xTKF2(4), yTKF2(4), zTKF2(4);   // translation keyframes
KeyFraming xRKF2(4), yRKF2(4), zRKF2(4);   // rotation keyframes

bool isHiding = false;
bool isLate = false;
bool isStarting = false;
bool isStop = false;

float saveT = 0.0f;


int main()
{

    mainWindow = glAllInit("FindPearl");

    // set default camera position
    cameraPos = cameraOrigPos;
  
    // basic light
    // -------------
    lightingShader = new Shader("shaders/basic_lighting.vs", "shaders/basic_lighting.fs");
    lightingShader->use();
    projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    lightingShader->setMat4("projection", projection);

    // initialize animation data
    // -------------
    initKeyframes();
    initKeyframes2();
    timeT = 0.0f;
    updateAnimData();
    updateAnimData2();
    renderMode = INIT;

    // text 
    // -------------
    textShader = new Shader("shaders/text.vs", "shaders/text.frag");
    text = new Text((char*)"fonts/TropicalAsianDemoRegular-11V0.ttf", textShader, SCR_WIDTH, SCR_HEIGHT);

    // model
    // -------------
    modelShader = new Shader("shaders/modelLoading.vs", "shaders/modelLoading.frag");
    panelModel = new Model((GLchar *)"models/panelscreen/171115_mia329_122161_Point_402_100Kfaces_OBJ.obj" );
    buddhaModel = new Model((GLchar*)"models/buddha/170511_mia337_005788_603_200Kfaces_OBJ4.obj");
    planetModel = new Model((GLchar*)"models/planet/planet.obj");
    fishModel = new Model((GLchar *)"models/jellyfish/Jellyfish_001.obj");
    diverModel = new Model((GLchar*)"models/sczbadiver/sczbadiver.obj");
    modelShader->use();
    modelShader->setMat4("projection", projection);

    // random model transformation matrices
    modelMatrices = new glm::mat4[amount];
    srand(static_cast<unsigned int>(glfwGetTime())); // initialize random seed
    float radius = 10.0;
    float offset = 2.0f;
    for (unsigned int i = 0; i < amount; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        // 1. translation
        float angle = (float)i / (float)amount * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius + displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float y = displacement * 0.4f; // keep height of asteroid field smaller compared to width of x and z
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, y, z));

        // 2. scale
        float scale = static_cast<float>((rand() % 20) / 100.0 + 0.05);
        model = glm::scale(model, glm::vec3(scale));

        // 3. rotation: add random rotation 
        float rotAngle = static_cast<float>((rand() % 360));
        model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

        modelMatrices[i] = model;
    }

    // sky
    // -------------
    vector<std::string> faces
    {
        "skybox/right.jpg",
        "skybox/left.jpg",
        "skybox/top.jpg",
        "skybox/bottom.jpg",
        "skybox/front.jpg",
        "skybox/back.jpg"
    };
    cubemapTexture = loadCubemap(faces);
    skybox = new SkyBox();
    skyboxShader = new Shader("shaders/skybox.vs", "shaders/skybox.fs");
    skyboxShader->use();
    skyboxShader->setInt("skybox", 0);
    
    // texture
    // -------------
    textureShader = new Shader("shaders/blending.vs", "shaders/blending.fs");
    textureShader->use();
    textureShader->setMat4("projection", projection);

    floorTexture = loadTexture("floor.png", false);
    transparentTexture = loadTexture("grass.png", true);
    containerTexture = loadTexture("box.bmp", false);
    textureShader->use();
    textureShader->setInt("texture1", 0);

    plane = new Plane();
    cube = new Cube();
    
    // for preventing flickering, discard very small edges in the texture
    plane->texCoords[0] = 0.1;
    plane->texCoords[1] = 0.9;
    plane->texCoords[2] = 0.1;
    plane->texCoords[3] = 0.1;
    plane->texCoords[4] = 0.9;
    plane->texCoords[5] = 0.1;
    plane->texCoords[6] = 0.9;
    plane->texCoords[7] = 0.9;
    plane->updateVBO();
    
    // setting vegetation locations
    vegetation.push_back(glm::vec3(-1.5f,  0.0f,  0.48f));
    vegetation.push_back(glm::vec3( 1.5f,  0.0f,  0.51f));
    vegetation.push_back(glm::vec3( 2.0f,  0.0f,  1.7f));
    vegetation.push_back(glm::vec3(-2.0f,  0.0f,  0.3f));
    vegetation.push_back(glm::vec3( 3.0f,  0.0f,  1.6f));
    vegetation.push_back(glm::vec3(-3.0f, 0.0f, 1.2f));

    // music
    // -------------
    PlaySound(TEXT(SOUND_FILE_NAME), NULL, SND_ASYNC | SND_LOOP);
    

    while (!glfwWindowShouldClose(mainWindow)) {

        render();
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}


// render
void render() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // texture shader
    // -------------
    view = glm::lookAt(cameraPos, camTarget, camUp);
    view = view * camArcBall.createRotationMatrix();
    textureShader->use();
    textureShader->setMat4("view", view);


    // drawing a floor
    // -------------
    glBindTexture(GL_TEXTURE_2D, floorTexture);

    model = glm::mat4(1.0);
    model = glm::translate(model, modelPan);
    model = glm::translate(model, glm::vec3(0.0f, -0.51f, 0.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(10.0f, 5.0f, 5.0f));
    textureShader->setMat4("model", model);
    plane->draw(textureShader);


    // drawing vegetation
    // -------------
    glBindTexture(GL_TEXTURE_2D, transparentTexture);
    for (GLuint i = 0; i < vegetation.size(); i++)
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, modelPan);
        model = glm::translate(model, vegetation[i]);
        textureShader->setMat4("model", model);
        plane->draw(textureShader);
    }


    // drawing a box (below pearl)
    // -------------
    glBindTexture(GL_TEXTURE_2D, containerTexture);
    model = glm::mat4(1.0);
    model = glm::translate(model, modelPan);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.3f));
    model = glm::scale(model, glm::vec3(1.8f, 1.0f, 1.0f));
    textureShader->setMat4("model", model);
    cube->draw(textureShader);


    // drawing sky
    // -------------
    glDepthFunc(GL_LEQUAL);
    skyboxShader->use();
    view = glm::mat4(glm::mat3(view)); // remove translation from the view matrix
    skyboxShader->setMat4("view", view);
    skyboxShader->setMat4("projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    skybox->draw(skyboxShader);
    glDepthFunc(GL_LESS); // set depth function back to default


    // light of pearl
    // -------------
    view = glm::lookAt(cameraPos, camTarget, camUp);
    view = view * camArcBall.createRotationMatrix();
    lightingShader->use();
    lightingShader->setMat4("view", view);
    lightingShader->setVec3("light.position", 0.0f, -2.0f, 1.0f);
    lightingShader->setVec3("viewPos", cameraPos);
    // light properties
    glm::vec3 lightColor;
    lightColor.x = sin(glfwGetTime() * 1.5f);
    lightColor.y = sin(glfwGetTime() * 1.7f);
    lightColor.z = sin(glfwGetTime() * 1.3f);
    glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // decrease the influence
    glm::vec3 ambientColor = diffuseColor * glm::vec3(0.9f); // low influence
    lightingShader->setVec3("light.ambient", ambientColor);
    lightingShader->setVec3("light.diffuse", diffuseColor);
    lightingShader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
    // material properties
    lightingShader->setVec3("material.ambient", 1.0f, 0.5f, 1.0f);
    lightingShader->setVec3("material.diffuse", 1.0f, 0.5f, 1.0f);
    lightingShader->setVec3("material.specular", 0.5f, 0.5f, 0.5f); // specular lighting doesn't have full effect on this object's material
    lightingShader->setFloat("material.shininess", 90.0f);
    // model matrix
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.0f, 2.0f, 1.0f)); //-1.0f, 1.5f, 0.5f
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    lightingShader->setMat4("model", model);


    // drawing pearl
    // -------------
    lightingShader->use();
    //glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    //view = view * camArcBall.createRotationMatrix();
    lightingShader->setMat4("view", view);
    glm::mat4 model(1.0);
    model = glm::translate(model, modelPan);
    model = glm::translate(model, glm::vec3(0.0f, 1.2f, 1.3f));
    model = model * modelArcBall.createRotationMatrix();   //modelArcball
    model = glm::scale( model, glm::vec3( 0.2f, 0.2f, 0.2f ) );
    lightingShader->setMat4("model", model);

    if (!isHiding) {
        planetModel->Draw(lightingShader);
    }


    // drawing buddha
    // -------------
    modelShader->use();
    modelShader->setMat4("view", view);
    model = glm::mat4(1.0);
    model = glm::translate(model, modelPan);
    model = glm::translate(model, glm::vec3(-3.0f, 0.0f, -3.0f));
    model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
    modelShader->setMat4("model", model);
    buddhaModel->Draw(modelShader);


    // drawing screen
    // -------------
    modelShader->use();
    modelShader->setMat4("view", view);
    model = glm::mat4(1.0);
    model = glm::translate(model, modelPan);
    model = glm::translate(model, glm::vec3(8.0f, 2.0f, 4.0f));
    model = glm::scale(model, glm::vec3(0.8f, 0.8f, 0.8f));
    modelShader->setMat4("model", model);
    panelModel->Draw(modelShader);


    // drawing fishs
    // -------------
    for (unsigned int i = 0; i < amount; i++)
    { 
        model = glm::translate(modelMatrices[i], glm::vec3(xTrans, yTrans, zTrans));
        glm::vec3 eulerAngles(glm::radians(xAngle), glm::radians(yAngle), glm::radians(zAngle));
        glm::quat q(eulerAngles);
        glm::mat4 rotMatrix = q.operator glm::mat4x4();
        model = model * rotMatrix;

        modelShader->setMat4("model", model);
        fishModel->Draw(modelShader);
    }


    // drawing diver
    modelShader->use();
    modelShader->setMat4("view", view);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(xTrans2, yTrans2, zTrans2));
    glm::vec3 eulerAngles2(glm::radians(xAngle2), glm::radians(yAngle2), glm::radians(zAngle2));
    glm::quat q(eulerAngles2);
    glm::mat4 rotMatrix = q.operator glm::mat4x4();

    model = glm::translate(model, glm::vec3(-4.0f, 2.0f, 2.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
    model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));

    model = model * rotMatrix;
    modelShader->setMat4("model", model);
    diverModel->Draw(modelShader);


    // animation1 : jelly fish
    // -------------
    float cTime = (float)glfwGetTime(); // current time
    timeT = cTime - beginT;
    updateAnimData();


    // animation2 : diver
    // -------------
    if (renderMode == ANIM) {
        float cTime2 = (float)glfwGetTime(); // current time
        timeT2 = cTime2 - beginT2;
        updateAnimData2();
    }


    // drawing texts
    // -------------
    text->RenderText("Let's catch the pearl!", 190.0f, 570.0f, 0.8f, glm::vec3(1.0, 1.0f, 0.5f));

    if (isStarting) {
        text->RenderText("Start to Move!", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
    }
    if (isStop) {
        text->RenderText("Stop!", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
    }
    if (isHiding) {
        text->RenderText("You catched the pearl", 150.0f, 280.0f, 1.0f, glm::vec3(1.0, 0.3f, 1.0f));
    }
    if (isLate) {
        text->RenderText("You didn't catch the pearl", 130.0f, 250.0f, 1.0f, glm::vec3(1.0, 0.0f, 0.0f));
    }

    glfwSwapBuffers(mainWindow);
}


GLFWwindow *glAllInit(const char *title)
{
    GLFWwindow *window;
    
    // glfw: initialize and configure
    if (!glfwInit()) {
        printf("GLFW initialisation failed!");
        glfwTerminate();
        exit(-1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    // glfw window creation
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, title, NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    // OpenGL states
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    
    // Allow modern extension features
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        cout << "GLEW initialisation failed!" << endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(-1);
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    return window;
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path, bool vflip)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    if (vflip) stbi_set_flip_vertically_on_load(true);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else {
            format = GL_RGBA;
        }
        
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


// loads a cubemap texture 
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}


void initKeyframes() {

    // x-translation keyframes
    xTKF.setKey(0, 0, -5.0);
    xTKF.setKey(1, 10, -2.0);
    xTKF.setKey(2, 20, 2.0);
    xTKF.setKey(3, animEndTime, 5.0);

    // y-translation keyframes
    yTKF.setKey(0, 0, 3.0);
    yTKF.setKey(1, 10, -2.0);
    yTKF.setKey(2, 20, 2.0);
    yTKF.setKey(3, animEndTime, -3.0);

    // z-translation keyframes
    zTKF.setKey(0, 0, 1.0);
    zTKF.setKey(1, 10, -2.0);
    zTKF.setKey(2, 20, 2.0);
    zTKF.setKey(3, animEndTime, 0.0);

    // x-rotation keyframes
    xRKF.setKey(0, 0, 0.0);
    xRKF.setKey(1, 10, 20.0);
    xRKF.setKey(2, 20, 80.0);
    xRKF.setKey(3, animEndTime, 0.0);

    // y-rotation keyframes
    yRKF.setKey(0, 0, 0.0);
    yRKF.setKey(1, 10, -30.0);
    yRKF.setKey(2, 20, 50.0);
    yRKF.setKey(3, animEndTime, 0.0);

    // z-rotation keyframes
    zRKF.setKey(0, 0, 0.0);
    zRKF.setKey(1, 10, 90.0);
    zRKF.setKey(2, 20, 180.0);
    zRKF.setKey(3, animEndTime, 200.0);
}

void updateAnimData() {
    if (timeT > animEndTime) {
        if (!isHiding) {
            isLate = true;
            isStarting = false;
            isStop = false;
        }
        renderMode = STOP;
        timeT = animEndTime;
    }
    xTrans = xTKF.getValLinear(timeT);
    yTrans = yTKF.getValLinear(timeT);
    zTrans = zTKF.getValLinear(timeT);
    xAngle = xRKF.getValLinear(timeT);
    yAngle = yRKF.getValLinear(timeT);
    zAngle = zRKF.getValLinear(timeT);
}

void initKeyframes2() {

    // x-translation keyframes
    xTKF2.setKey(0, 0, -5.0);
    xTKF2.setKey(1, 10, -3.0);
    xTKF2.setKey(2, 20, 0.0);
    xTKF2.setKey(3, animEndTime, 1);

    // y-translation keyframes
    yTKF2.setKey(0, 0, 3.0);
    yTKF2.setKey(1, 10, 1.0);
    yTKF2.setKey(2, 20, -3.0);
    yTKF2.setKey(3, animEndTime, -3.0);

    // z-translation keyframes
    zTKF2.setKey(0, 0, 0.0);
    zTKF2.setKey(1, 10, 0.5);
    zTKF2.setKey(2, 20, -0.5);
    zTKF2.setKey(3, animEndTime, -0.5);

    // x-rotation keyframes
    xRKF2.setKey(0, 0, 5.0);
    xRKF2.setKey(1, 10, -5.0);
    xRKF2.setKey(2, 20, 0.0);
    xRKF2.setKey(3, animEndTime, 0.0);

    // y-rotation keyframes
    yRKF2.setKey(0, 0, 5.0);
    yRKF2.setKey(1, 10, -5.0);
    yRKF2.setKey(2, 20, 0.0);
    yRKF2.setKey(3, animEndTime, 0.0);

    // z-rotation keyframes
    zRKF2.setKey(0, 0, -5.0);
    zRKF2.setKey(1, 10, 5.0);
    zRKF2.setKey(2, 20, 0.0);
    zRKF2.setKey(3, animEndTime, 0.0);
}

void updateAnimData2() {
    if (timeT2 > 15.0f) {
        isHiding = true;
        isStarting = false;
        isStop = false;
        PlaySound(TEXT(SOUND_FILE_NAME_SUCCESS), NULL, SND_ASYNC);
        renderMode = STOP;
    }
    xTrans2 = xTKF2.getValLinear(timeT2);
    yTrans2 = yTKF2.getValLinear(timeT2);
    zTrans2 = zTKF2.getValLinear(timeT2);
    xAngle2 = xRKF2.getValLinear(timeT2);
    yAngle2 = yRKF2.getValLinear(timeT2);
    zAngle2 = zRKF2.getValLinear(timeT2);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    projection = glm::perspective(glm::radians(45.0f),
                                  (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    textureShader->use();
    textureShader->setMat4("projection", projection);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        camArcBall.init(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true);
        modelArcBall.init(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true);
        cameraPos = cameraOrigPos;
        modelPan[0] = modelPan[1] = modelPan[2] = 0.0f;
    }
    else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        arcballCamRot = !arcballCamRot;
        if (arcballCamRot) {
            cout << "ARCBALL: Camera rotation mode" << endl;
        }
        else {
            cout << "ARCBALL: Model  rotation mode" << endl;
        }
    }

    else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        if (renderMode == INIT) {
            renderMode = ANIM;
            isStarting = true;
            beginT2 = glfwGetTime();
        }
        else if (renderMode == STOP) {
            if (timeT2 == animEndTime) renderMode = INIT;
            else {
                renderMode = ANIM;
                isStop = false;
                isStarting = true;
                beginT2 = beginT2 + (glfwGetTime() - saveT - beginT2);
            }
        }
        else if (renderMode == ANIM) {
            renderMode = STOP;
            isStop = true;
            isStarting = false;
            saveT = timeT2;
        }

        if (renderMode == INIT) {
            timeT2 = 0.0;
            updateAnimData2();
        }
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (arcballCamRot)
        camArcBall.mouseButtonCallback(window, button, action, mods);
    else
        modelArcBall.mouseButtonCallback(window, button, action, mods);
}

void cursor_position_callback(GLFWwindow *window, double x, double y) {
    if (arcballCamRot)
        camArcBall.cursorCallback(window, x, y);
    else
        modelArcBall.cursorCallback(window, x, y);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    cameraPos[2] -= (yoffset * 0.5);
}

