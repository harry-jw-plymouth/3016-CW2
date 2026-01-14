//STD
#include <iostream>

//GLAD
#include <glad/glad.h>

//GLM
#include "glm/ext/vector_float3.hpp"
#include <glm/gtc/type_ptr.hpp> //Access to the value_ptr

//ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//LEARNOPENGL
#include <learnopengl/shader_m.h>
#include <learnopengl/model.h>

//GENERAL
#include "Header.h"

#include "FastNoiseLite.h"

using namespace std;
using namespace glm;


using namespace std;
using namespace glm;

int windowWidth;
int windowHeight;

//VAO vertex attribute positions in correspondence to vertex attribute type
enum VAO_IDs { Triangles, Indices, Colours, Textures, NumVAOs = 2 };
//VAOs
GLuint TerrainVAOs[NumVAOs];
GLuint ObjectVAOS[NumVAOs];
GLuint SecondObjectVAOs[NumVAOs];
//Buffer types
enum Buffer_IDs { ArrayBuffer, NumBuffers = 4 };
//Buffer objects
GLuint TerrainBuffers[NumBuffers];
GLuint ObjectBuffers[NumBuffers];
GLuint SecondObjectBuffers[NumBuffers];

//Shader program
GLuint program;

int HighestIndex;

//Transformations
//Relative position within world space
vec3 cameraPosition = vec3(0.0f, 5.0f, 0.0f);
//The direction of travel
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);
//Up position within world space
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);

//Camera sideways rotation
float cameraYaw = -90.0f;
//Camera vertical rotation
float cameraPitch = 0.0f;
//Determines if first entry of mouse into window
bool mouseFirstEntry = true;
//Positions of camera from given last frame
float cameraLastXPos = 800.0f / 2.0f;
float cameraLastYPos = 600.0f / 2.0f;


float deltaTime = 0.0f;
//Last value of time change
float lastFrame = 0.0f;

//Start Pos for terrain (both x and y)
float drawingStartPosition = 4.0f;

//Size of each section connected by vertices
float TileSize = 0.0625f;

#define RENDER_DISTANCE 256 //Render width of map
#define MAP_SIZE RENDER_DISTANCE * RENDER_DISTANCE //Size of map in x & z space

//Amount of chunks across one dimension
const int squaresRow = RENDER_DISTANCE - 1;
//Two triangles per square to form a 1x1 chunk
const int trianglesPerSquare = 2;

//Amount of triangles on map
const int TrianglesGrid = squaresRow * squaresRow * trianglesPerSquare;
GLfloat(*terrainVertices)[6] = new GLfloat[MAP_SIZE][6];
//Generation of height map indices - ALLOCATED ON HEAP TO AVOID STACK OVERFLOW
GLuint(*terrainIndices)[3] = new GLuint[TrianglesGrid][3];

//flag for updatingwindow
bool UpdateNeeded = true;

//Tallest point of terrain 
vec3 TerrainTallestPointCoords;

//list of scattered trees coordinates 
const int  NumberOfTrees = 50;
vec3 TreesPositions[NumberOfTrees];
int indexesToPlaceTrees[NumberOfTrees];

//list of scattered rock model placement positions
const int  NumberOfRocks = 100;
vec3 RocksPositions[NumberOfRocks];

//Postion for butterfly
mat4 ButterflyModel = mat4(1.0f);
vec3 ButterflyCurrentPos(0.0f, 5.0f, 0.0f);

//W shaped object vertices
float SecondObjectVertices[] = {
    //Front
    //left outer section
    -1.0f,1.0f,0.0f,   0.0f,1.0f, //0 top left
    -0.75f,1.0f, 0.0f,  1.0f,1.0f, //1 top right
    -0.25,0.0f,  0.0f,      1.0f, 0.0f,  //2 bottom right
    -0.5f,-1.0f, 0.0f,   0.0f,0.0f,  //3 bottom left

    //left inner section
     -0.25f,0.0f,0.0f, 0.0f,1.0f, //4 top left
    -0.0f,0.25f, 0.0f,  1.0f,1.0f, //5 top right
    -0.0f,-0.25f,0.0f,      1.0f, 0.0f,  //6 bottom right
    -0.5f,-1.0f, 0.0f,   0.0f,0.0f,  //7 bottom left

    //Right Inner section
    0.0f,0.25f,0.0f,  0.0f,1.0f, //8 top left
    0.25f,0.0f, 0.0f,  1.0f,1.0f, //9 top right
     0.5f,-1.0f, 0.0f,      1.0f, 0.0f,  //10 bottom right
     0.0f,-0.25f,  0.0f,   0.0f,0.0f,  //11 bottom left

     //Right Outer section
     0.75f,1.0f, 0.0f, 0.0f,1.0f, //12 top left
     1.0f,1.0f, 0.0f,  1.0f,1.0f, //13 top right
      0.5f,-1.0f,0.0f,    1.0f, 0.0f,  //14 bottom right
      0.25f,0.0f, 0.0f,   0.0f,0.0f,  //15 bottom left
      //back
     //left outer section
     -1.0f,1.0f,1.0f,   0.0f,1.0f, //16 top left
     -0.75f,1.0f, 1.0f,  1.0f,1.0f, //17 top right
     -0.25,0.0f,  1.0f,      1.0f, 0.0f,  //18 bottom right
     -0.5f,-1.0f, 1.0f,   0.0f,0.0f,  //19 bottom left

     //left inner section
      -0.25f,0.0f,1.0f, 0.0f,1.0f, //20 top left
     -0.0f,0.25f, 1.0f,  1.0f,1.0f, //21 top right
     -0.0f,-0.25f,1.0f,      1.0f, 0.0f,  //22 bottom right
     -0.5f,-1.0f, 1.0f,   0.0f,0.0f,  //23 bottom left

     //Right Inner section
     0.0f,0.25f,1.0f,  0.0f,1.0f, //24 top left
     0.25f,0.0f, 1.0f,  1.0f,1.0f, //25 top right
      0.5f,-1.0f, 1.0f,      1.0f, 0.0f,  //26 bottom right
      0.0f,-0.25f, 1.0f,   0.0f,0.0f,  //27 bottom left

      //Right Outer section
      0.75f,1.0f, 1.0f, 0.0f,1.0f, //28 top left
      1.0f,1.0f, 1.0f,  1.0f,1.0f, //29 top right
       0.5f,-1.0f,1.0f,    1.0f, 0.0f,  //30 bottom right
       0.25f,0.0f, 1.0f,   0.0f,0.0f,  //31 bottom left


};
//W shaped object indices
unsigned int SecondObjectIndices[] = {
    //front
    0,1,2  , 0,2,3,  //left outer section 
    4,7,6 ,  4,5,6,  // left inner section 
    8,11,9,  11,14,9,  // right inner section
    9,10,13,  9,12,13,  //Right Outer section
    //back
    16,17,18  , 16,18,19,  //left outer section 
    20,23,22 ,  20,21,22,  // left inner section 
    24,27,25,  27,30,25,  // right inner section
    25,26,29,  25,28,29,  //Right Outer section
    //sides
     0,1,16,  1,16,17,  //top left
     12,13,28,  13,28,29,//top right
     0,3,16,  16,19,3,//left outer side
     1,4, 17,   17,18,4,   //left inner side
     4,5,20,  20,21,5,// left center top
     8,9,24,   24,25,9,//right center top
     9,12, 31,   28,31,12 ,  //right inner side
     13,10,29,   29,30, 10,//right outer side
     10,11,30,  11,26,27,//Bottom right
     7,6,22,  22,23,7 //bottom left


};

//H Shaped object vertices
float ObjectVertices[] = {
    // Left section                        Indexes and positions in corresponding sqaure
    -1.0f,  1.0f, 0.0f,     0.0f, 1.0f,  //0 top left
    -0.5f,  1.0f, 0.0f,     1.0f, 1.0f,  //1  top right  
    -0.5f, -1.0f, 0.0f,     1.0f, 0.0f,  //2  bottom right
    -1.0f, -1.0f, 0.0f,     0.0f, 0.0f,  //3   bottom left

    // Right section
     0.5f,  1.0f, 0.0f,     0.0f, 1.0f,  //4
    1.0f,  1.0f, 0.0f,  1.0f, 1.0f,      //5
    1.0f, -1.0f, 0.0f,  1.0f, 0.0f,      //6
    0.5f, -1.0f, 0.0f,  0.0f, 0.0f,      //7

    // Middle section
    -0.5f,  0.25f, 0.0f,     0.0f, 1.0f, //8
    0.5f,  0.25f, 0.0f,  1.0f, 1.0f,     //9
   0.5f, -0.25f, 0.0f,     1.0f, 0.0f,   //10
    -0.5f, -0.25f, 0.0f,     0.0f, 0.0f,  //11

    // Back
   // Left section
   -1.0f,  1.0f, 1.0f,     0.0f, 1.0f,   //12
   -0.5f,  1.0f, 1.0f,     1.0f, 1.0f,   //13
   -0.5f, -1.0f, 1.0f,     1.0f, 0.0f,   //14
   -1.0f, -1.0f, 1.0f,     0.0f, 0.0f,   //15

   // Right section
    0.5f,  1.0f, 1.0f,     0.0f, 1.0f,   //16
   1.0f,  1.0f, 1.0f,  1.0f, 1.0f,       //17
   1.0f, -1.0f, 1.0f,  1.0f, 0.0f,      //18
   0.5f, -1.0f, 1.0f,  0.0f, 0.0f,      //19

   // Middle section
   -0.5f,  0.25f, 1.0f,     0.0f, 1.0f,   //20
   0.5f,  0.25f, 1.0f,  1.0f, 1.0f,       //21
  0.5f, -0.25f, 1.0f,     1.0f, 0.0f,     //22
   -0.5f, -0.25f, 1.0f,     0.0f, 0.0f,   //23

};
//H Shaped object indices
unsigned int ObjectIndices[] = {
    // Left section
     0, 1, 2,  2, 3, 0,
     // Right section
      4, 5, 6,  6, 7, 4,
      // Middle section
       8, 9,10, 10,11, 8,
       //Back
         //left section
      12,15,14, 14,13,12,
      //Right Section
      16,19,18, 18,17,16,
      //Back section
      20,23,22, 22,21,20,
      //Sides
       //Left Side
         0,3,15, 15,12,0,  //left/outer
        1,13,14, 14,2,1,   //right/inner
        0,1,12, 12,13,1,  // top  
        2,3,14,  14,15,3, //bottom


        //Right Side
        4,7,19, 19,16,4, //left/inner
        5,17,18, 18,6,5, //right/outer
        4,5,16, 16,17,5, //top 
        6,7,18, 18, 19, 7, //bottom

        //Middle Side
        11,10,22, 22,23,11,//bottom 
        8,20,21, 21,9,8 //top
};

//
const unsigned int totalIndexCount = sizeof(ObjectIndices) / sizeof(ObjectIndices[0]);
const unsigned int totalIndexCountForSecondObject = sizeof(SecondObjectIndices) / sizeof(SecondObjectIndices[0]);

// mat4 for H and W object shapes 
mat4 ObjectTransformModel;
mat4 SecondObjectTransformModel;
unsigned int texture;


static int SetUpObject() {
    //First object
    glGenVertexArrays(NumVAOs, ObjectVAOS);
    glBindVertexArray(ObjectVAOS[0]);

    glGenBuffers(NumBuffers, ObjectBuffers);

    glBindBuffer(GL_ARRAY_BUFFER, ObjectBuffers[Triangles]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ObjectVertices), ObjectVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ObjectBuffers[Indices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ObjectIndices), ObjectIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //Textures
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //Unbinding
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //postioned to map center
    ObjectTransformModel = mat4(1.0f);
    //   SecondObjectTransformModel = translate(SecondObjectTransformModel, vec3(-0.25, 5, 0.0));
    ObjectTransformModel = translate(ObjectTransformModel, TerrainTallestPointCoords);
 //   ObjectTransformModel = translate(ObjectTransformModel, vec3(-0.7, 0.10, 0.7));
   ObjectTransformModel = translate(ObjectTransformModel, vec3(-0.7, 1.0, 0.7));
    //  SecondObjectTransformModel = scale(SecondObjectTransformModel, vec3(0.5, 0.5, 0.5));
    ObjectTransformModel = rotate(ObjectTransformModel, radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
    ObjectTransformModel = scale(ObjectTransformModel, vec3(0.2, 0.2, 0.2));


    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    //Selects x axis (S) of texture bound to GL_TEXTURE_2D & sets to repeat beyond normalised coordinates
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //Selects y axis (T) equivalently
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);



    //Second object
    glGenVertexArrays(NumVAOs, SecondObjectVAOs);
    glBindVertexArray(SecondObjectVAOs[0]);

    glGenBuffers(NumBuffers, SecondObjectBuffers);

    glBindBuffer(GL_ARRAY_BUFFER, SecondObjectBuffers[Triangles]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SecondObjectVertices), SecondObjectVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SecondObjectBuffers[Indices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(SecondObjectIndices), SecondObjectIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //Textures
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //Unbinding
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //Postion model to center of map
    SecondObjectTransformModel = mat4(1.0f);
    SecondObjectTransformModel = translate(SecondObjectTransformModel, TerrainTallestPointCoords);
   // SecondObjectTransformModel = translate(SecondObjectTransformModel, vec3(-0.3, 0.10, 0.3));
    SecondObjectTransformModel = translate(SecondObjectTransformModel, vec3(-0.3, 1.0, 0.3));
    SecondObjectTransformModel = rotate(SecondObjectTransformModel, radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
    SecondObjectTransformModel = scale(SecondObjectTransformModel, vec3(0.2, 0.2, 0.2));

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    //Selects x axis (S) of texture bound to GL_TEXTURE_2D & sets to repeat beyond normalised coordinates
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //Selects y axis (T) equivalently
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


    //texture
    int width, height, colourChannels;

    //Get texture from media
    unsigned char* data = stbi_load("media/woodPlanks.jpg", &width, &height, &colourChannels, 0);
    if (data) //If successful
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else //If unsuccessful
    {
        cout << "Failed to load texture.\n";
        return -1;
    }
    stbi_image_free(data);
}

//Pulled directly from labs
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    //Resizes window based on contemporary width & height values
    glViewport(0, 0, width, height);
}
//Pulled directly from labs
void Mouse_CallBack(GLFWwindow* window, double xpos, double ypos) {
    //Initially no last positions, so sets last positions to current positions
    if (mouseFirstEntry)
    {
        cameraLastXPos = (float)xpos;
        cameraLastYPos = (float)ypos;
        mouseFirstEntry = false;
    }

    //Sets values for change in position since last frame to current frame
    float xOffset = (float)xpos - cameraLastXPos;
    float yOffset = cameraLastYPos - (float)ypos;

    //Sets last positions to current positions for next frame
    cameraLastXPos = (float)xpos;
    cameraLastYPos = (float)ypos;

    //Moderates the change in position based on sensitivity value
    const float sensitivity = 0.025f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    //Adjusts yaw & pitch values against changes in positions
    cameraYaw += xOffset;
    cameraPitch += yOffset;
    //flag to update window in loop
    if (cameraPitch != 0 || cameraYaw != 0) {
        UpdateNeeded = true;
    }

    //Prevents turning up & down beyond 90 degrees to look backwards
    if (cameraPitch > 89.0f)
    {
        cameraPitch = 89.0f;
    }
    else if (cameraPitch < -89.0f)
    {
        cameraPitch = -89.0f;
    }

    //Modification of direction vector based on mouse turning
    vec3 direction;
    direction.x = cos(radians(cameraYaw)) * cos(radians(cameraPitch));
    direction.y = sin(radians(cameraPitch));
    direction.z = sin(radians(cameraYaw)) * cos(radians(cameraPitch));
    cameraFront = normalize(direction);
}
float GetHeightOfTerrainAtCurrentPos() {
    //return height of terrain at cameras current position
    // Used to ensure user does not go under terrain
    int CurrentX = (int)((drawingStartPosition - cameraPosition.x) / TileSize);
    int CurrentZ = (int)((drawingStartPosition - cameraPosition.z) / TileSize);

    //Check camera is within bounds of terrain, otherwise crash will occur
    if (CurrentX < 0 || CurrentX >= RENDER_DISTANCE || CurrentZ >= RENDER_DISTANCE || CurrentZ < 0) {
        return 0;
    }
    return terrainVertices[CurrentZ * RENDER_DISTANCE + CurrentX][1];

}
//check user position to ensure they have not gone throught the ground
void CheckForCollision() {
    float CurrentPosTerrainHeight = GetHeightOfTerrainAtCurrentPos();
    if (cameraPosition.y < CurrentPosTerrainHeight + 0.25) {
        cameraPosition.y = CurrentPosTerrainHeight + 0.25;
    }
}
//Pulled directly from labs, but with addition of call for collision check function and boolean flag for render loop
void ProcessUserInput(GLFWwindow* WindowIn) {
    if (glfwGetKey(WindowIn, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(WindowIn, true);
    }
    //Extent to which to move in one instance
    const float movementSpeed = 2.0f * deltaTime;
    //WASD controls
    if (glfwGetKey(WindowIn, GLFW_KEY_W) == GLFW_PRESS)
    {
        cameraPosition += movementSpeed * cameraFront;
        UpdateNeeded = true;
    }
    if (glfwGetKey(WindowIn, GLFW_KEY_S) == GLFW_PRESS)
    {
        cameraPosition -= movementSpeed * cameraFront;
        UpdateNeeded = true;
    }
    if (glfwGetKey(WindowIn, GLFW_KEY_A) == GLFW_PRESS)
    {
        cameraPosition -= normalize(cross(cameraFront, cameraUp)) * movementSpeed;
        UpdateNeeded = true;
    }
    if (glfwGetKey(WindowIn, GLFW_KEY_D) == GLFW_PRESS)
    {
        cameraPosition += normalize(cross(cameraFront, cameraUp)) * movementSpeed;
        UpdateNeeded = true;
    }
    //Function called to ensure camera doesnt go beneath terrain
    CheckForCollision();
    // cout << "camers pos: " << cameraPosition.x << ", " << cameraPosition.y << cameraPosition.z << "\n";
}
void SetPosForModels() {
    int Current = 0;
    int Variance;
    int FinalIndex = 0;
    int IndexTemp;
    // loop through every tree to be drawn on terrain
    for (int i = 0; i < NumberOfTrees; i++) {
        // indexes dynamically set based on number of trees required/declared earlier in code
       // indexes chosen are equally spaced apart initially
       // essentially i multiplied The Number of vertices in the terrain divided by the number of trees
       // Variance is then added to the index to ensure trees are added in a semi realistic pattern
       // variance is random, but limited so trees are still spread apart properly
        FinalIndex = (i * (HighestIndex / NumberOfTrees)) + rand() % (HighestIndex / NumberOfTrees);

        //Check Variance doesnt make the index higher than the length of the array it is indexing
        if (HighestIndex - 1 < FinalIndex) {
            FinalIndex = HighestIndex;
        }

        //set tree model position to the poistion stored at the vertices of the index
        TreesPositions[i].x = terrainVertices[FinalIndex][0];
        TreesPositions[i].y = terrainVertices[FinalIndex][1];
        TreesPositions[i].z = terrainVertices[FinalIndex][2];
    }
    //loop through every rock to be drawn on the terrain
    for (int i = 0; i < NumberOfRocks; i++) {
        //get index with variance
        // same method as trees
        FinalIndex = (i * (HighestIndex / NumberOfRocks)) + rand() % (HighestIndex / NumberOfRocks);

        //Check Variance doesnt make the index too high
        if (HighestIndex - 1 < FinalIndex) {
            FinalIndex = HighestIndex;
        }

        //set rock position to position of position of indexed vertices
        RocksPositions[i].x = terrainVertices[FinalIndex][0];
        RocksPositions[i].y = terrainVertices[FinalIndex][1] + 0.01;
        RocksPositions[i].z = terrainVertices[FinalIndex][2];
    }
}
//mostly taken from labs but with some adjustments 
void SetUpTerrain() {
    //Biome noise
    FastNoiseLite TerrainNoise;
    TerrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    TerrainNoise.SetFrequency(0.05f);
    int TerrainSeed = rand() % 100;
    TerrainNoise.SetSeed(TerrainSeed);

    //Biome noise
    FastNoiseLite BiomeNoise;
    BiomeNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    BiomeNoise.SetFrequency(0.05f);
    int biomeSeed = rand() % 100;
    TerrainNoise.SetSeed(biomeSeed);


    float centerX = RENDER_DISTANCE * 0.5f; float centerY = RENDER_DISTANCE * 0.5f;

    //Positions to start drawing from (centered around origin)
    float columnVerticesOffset = drawingStartPosition;
    float rowVerticesOffset = drawingStartPosition;

    //Highest point of terrain updated in loop
    float TallestTerrainPos = -200;
    int TallestPosIndex = 0;
    //Center 
    float CenterPos = RENDER_DISTANCE * 0.5f;

    //Terrain vertice index
    int i = 0;
    //Using x & y nested for loop in order to apply noise 2-dimensionally
    for (int y = 0; y < RENDER_DISTANCE; y++)
    {
        for (int x = 0; x < RENDER_DISTANCE; x++)
        {
            //Setting of height from 2D noise value at respective x & y coordinate
            float XOffset = x - CenterPos;
            float YOffset = y - CenterPos;
            //Uses pythagorean theorem to calculate true distance
            float Distance = sqrt(XOffset * XOffset + YOffset * YOffset);
            float maxDist = sqrt(CenterPos * CenterPos + CenterPos * CenterPos);

            //uses fall off to create the slope of the hill get lower the further from center the position is
            float FallOff = 1.0f - (Distance / maxDist); 
            FallOff =std::max(FallOff, 0.0f);
            float noise = TerrainNoise.GetNoise((float)x, (float)y);
            terrainVertices[i][1] = FallOff * 5.0f + noise * 0.5f;

            float biomeValue = BiomeNoise.GetNoise((float)x, (float)y);

            if (biomeValue <= -0.75f) //Plains
            {
                //set biome to green colour
                terrainVertices[i][3] = 0.45f;
                terrainVertices[i][4] = 0.75f;
                terrainVertices[i][5] = 0.35f;
            }
            else //Deeper forest
            {
                //set biome to darker green colour
                terrainVertices[i][3] = 0.10f;
                terrainVertices[i][4] = 0.45f;
                terrainVertices[i][5] = 0.20f;
            }
            //check to see if new height is taller than current tallest
            // used ton detemine where to place central objects 
            if (terrainVertices[i][1] > TallestTerrainPos) {
                TallestTerrainPos = terrainVertices[i][1];
                TallestPosIndex = i;
            }
            i++;
        }
    }

    int RowIndex = 0;
    for (int i = 0; i < MAP_SIZE; i++)
    {
        //Generation of x & z vertices for horizontal plane
        terrainVertices[i][0] = columnVerticesOffset;
        terrainVertices[i][2] = rowVerticesOffset;


        //Shifts x position across for next triangle along grid
        columnVerticesOffset = columnVerticesOffset + -TileSize;

        //Indexing of each chunk within row
        RowIndex++;
        //True when all triangles of the current row have been generated
        if (RowIndex == RENDER_DISTANCE)
        {
            //Resets for next row of triangles
            RowIndex = 0;
            //Resets x position for next row of triangles
            columnVerticesOffset = drawingStartPosition;
            //Shifts z position
            rowVerticesOffset = rowVerticesOffset + -TileSize;
        }
    }

    //Set tallest coordinates to cooridnates of vertice of highest index 
    HighestIndex = i;
    TerrainTallestPointCoords.x = terrainVertices[TallestPosIndex][0];
    TerrainTallestPointCoords.y = terrainVertices[TallestPosIndex][1];
    TerrainTallestPointCoords.z = terrainVertices[TallestPosIndex][2];

    //Positions to start mapping indices from
    int ColumnIndicesOffset = 0;
    int RowIndicesOffset = 0;

    //Loop for setting terrain Indices
    RowIndex = 0;
    for (int i = 0; i < TrianglesGrid - 1; i += 2)
    {
        terrainIndices[i][0] = ColumnIndicesOffset + RowIndicesOffset; //top left
        terrainIndices[i][1] = RENDER_DISTANCE + ColumnIndicesOffset + RowIndicesOffset; //bottom left
        terrainIndices[i][2] = 1 + ColumnIndicesOffset + RowIndicesOffset; //top right

        terrainIndices[i + 1][0] = 1 + ColumnIndicesOffset + RowIndicesOffset; //top right
        terrainIndices[i + 1][1] = RENDER_DISTANCE + ColumnIndicesOffset + RowIndicesOffset; //bottom left
        terrainIndices[i + 1][2] = 1 + RENDER_DISTANCE + ColumnIndicesOffset + RowIndicesOffset; //bottom right

        //Shifts x position across for next chunk along grid
        ColumnIndicesOffset = ColumnIndicesOffset + 1;

        //Indexing of each chunk within row
        RowIndex++;

        //True when all chunks of the current row have been generated
        if (RowIndex == squaresRow)
        {
            //Resets for next row of chunks
            RowIndex = 0;
            //Resets x position for next row of chunks
            ColumnIndicesOffset = 0;
            //Shifts z position
            RowIndicesOffset = RowIndicesOffset + RENDER_DISTANCE;
        }
    }

    //Sets index of VAO
    glGenVertexArrays(NumVAOs, TerrainVAOs);
    //Binds VAO to a buffer
    glBindVertexArray(TerrainVAOs[0]);
    //Sets indexes of all required buffer objects
    glGenBuffers(NumBuffers, TerrainBuffers);

    //Binds vertex object to array buffer
    glBindBuffer(GL_ARRAY_BUFFER, TerrainBuffers[Triangles]);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * MAP_SIZE * 6, terrainVertices, GL_STATIC_DRAW);

    //Binding & allocation for indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TerrainBuffers[Indices]);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * TrianglesGrid * 3, terrainIndices, GL_STATIC_DRAW);

    //Allocation & indexing of vertex attribute memory for vertex shader
    //Positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //Colours
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //Unbinding
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Get Coordinates for trees and other repeated objects to place around
    SetPosForModels();
}
void MoveButterFly() {
    float Speed = 0.1f;
    ButterflyCurrentPos.x += Speed * deltaTime;
    ButterflyModel = mat4(1.0f);
    ButterflyModel = translate(ButterflyModel, ButterflyCurrentPos);
    ButterflyModel = scale(ButterflyModel, vec3(0.25f));
    //ButterflyModel = translate(ButterflyModel, vec3(0.0f, 0.0f, Speed*deltaTime));

}
int main()
{
    //Initialisation of GLFW
    glfwInit();

    //Sizes for window
    windowWidth = 1280;
    windowHeight = 720;

    //Initialisation of 'GLFWwindow' object
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "CW2 Scene", NULL, NULL);

    //Checks if window has been successfully instantiated
    if (window == NULL)
    {
        std::cout << "GLFW Window did not instantiate\n";
        glfwTerminate();
        return -1;
    }
    //Sets cursor to automatically bind to window & hides cursor pointer
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //Binds OpenGL to window
    glfwMakeContextCurrent(window);

    //Initialisation of GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "GLAD failed to initialise\n";
        return -1;
    }

    //Loading of shaders
    Shader Shaders("shaders/vertexShader.vert", "shaders/fragmentShader.frag");
    Shader ObjectShader("shaders/ObjectVertexShader.vert", "shaders/ObjectFragmentShader.frag");
    Shader TerrainShader("shaders/TerrainVertexShader.vert", "shaders/TerrainFragmentShader.frag");

    //Loading Of Models
    Model Rock("media/rock/Rock07-Base.obj");;
    Model Butterfly("media/Bird/_butterfly.obj");
    Model Tree("media/Tree/GenTree-103_AE3D_03122023-F1.obj");
    // Only use centre tree when required, causes slow down
   //  Model CenterTree("media/CenterTree/MainTree.obj");
    Shaders.use();

    //Sets the viewport size within the window to match the window size of 1280x720
    glViewport(0, 0, 1280, 720);

    //Sets the framebuffer_size_callback() function as the callback for the window resizing event
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //Sets the mouse_callback() function as the callback for the mouse movement event
    glfwSetCursorPosCallback(window, Mouse_CallBack);

    SetUpTerrain();

    //Model matrices
    mat4 model = mat4(1.0f);
    mat4 ScatteredModel = mat4(1.0f);
    mat4 ScatteredRockModel = mat4(1.0f);
    mat4 MainTreeModel = mat4(1.0f);

    //Looking straight forward
    model = rotate(model, radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
    ScatteredModel = rotate(ScatteredModel, radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
    //Elevation to look upon terrain
    model = translate(model, TerrainTallestPointCoords);
    MainTreeModel = translate(MainTreeModel, TerrainTallestPointCoords);

    //Set Butterfly to start position
    ButterflyModel = translate(ButterflyModel, vec3(0.0f, 5.0f, 0.0f));
    ButterflyModel = scale(ButterflyModel, vec3(0.0025, 0.0025, 0.0025));


    //Model for terrain
    mat4 TerrainModel = mat4(1.0f);

    //Projection matrix
    mat4 projection = perspective(radians(45.0f), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);

    //Scaling to zoom in
    model = scale(model, vec3(0.025f, 0.025f, 0.025f));
    MainTreeModel = scale(MainTreeModel, vec3(0.05f));

    //Function sets up vertices and indices for the H and W objects 
    SetUpObject();

    //mvps to pass to shaders
    mat4 mvp;
    mat4 Objectmvp;
    glEnable(GL_DEPTH_TEST);

    //Render loop
    while (glfwWindowShouldClose(window) == false)
    {
        //Time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //Input
        ProcessUserInput(window);

        //Move model each frame
        MoveButterFly();
        //Draw Butterfly
        mat4 view;
        mvp = projection * view * ButterflyModel;
        Shaders.setMat4("mvpIn", mvp);
        Butterfly.Draw(Shaders);

        //Rendering
        glClearColor(0.25f, 0.0f, 1.0f, 1.0f); //Colour to display 
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);

        UpdateNeeded = false;

        //Transformations
        view = lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp); //Sets the position of the viewer, the movement direction in relation to it & the world up direction
        mvp = projection * view * TerrainModel;

        //Set shader for terrain
        TerrainShader.use();
        TerrainShader.setMat4("mvpIn", mvp); //Setting of uniform with Shader class

        //Render terrain
        glBindVertexArray(TerrainVAOs[0]);
        glDrawElements(GL_TRIANGLES, TrianglesGrid * 3, GL_UNSIGNED_INT, 0);

        //Draw H Object
        ObjectShader.use();
        ObjectShader.setInt("textureIn", 0);
        Objectmvp = projection * view * ObjectTransformModel;
        ObjectShader.setMat4("transformIn", Objectmvp); //Setting of uniform with Shader class

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glBindVertexArray(ObjectVAOS[0]);
        // glBindTexture(GL_TEXTURE_2D, texture);

        glDrawElements(GL_TRIANGLES, totalIndexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);


        //Draw W Second object
        Objectmvp = projection * view * SecondObjectTransformModel;
        ObjectShader.setMat4("transformIn", Objectmvp); //Setting of uniform with Shader class

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glBindVertexArray(SecondObjectVAOs[0]);

        glDrawElements(GL_TRIANGLES, totalIndexCountForSecondObject, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        //Drawing models

        //draw main tree at tallest point
        mvp = projection * view * MainTreeModel;
        Shaders.setMat4("mvpIn", mvp);
        Tree.Draw(Shaders);

        // Main tree (slows preformance)
      //  mvp = projection * view * MainTreeModel;
      //  Shaders.setMat4("mvpIn", mvp);
      //  CenterTree.Draw(Shaders);

        for (int i = 0; i < NumberOfTrees; i++) {
            mat4 ScatteredModel = mat4(1.0f);
            ScatteredModel = translate(ScatteredModel, TreesPositions[i]);
            ScatteredModel = scale(ScatteredModel, vec3(0.025f, 0.025f, 0.025f));

            mvp = projection * view * ScatteredModel;
            Shaders.setMat4("mvpIn", mvp);

            Tree.Draw(Shaders);

        }

        //Check if update needed
        //Only updates if necessary(e.g on user input)
        if (UpdateNeeded) {
            Shaders.use();
            //Draw scattered trees
           
            //Draw scattered rocks
            for (int i = 0; i < NumberOfRocks; i++) {
                mat4 ScatteredRockModel = mat4(1.0f);
                ScatteredRockModel = translate(ScatteredRockModel, RocksPositions[i]);
                ScatteredRockModel = scale(ScatteredRockModel, vec3(0.0005f, 0.0005f, 0.0005f));
                mvp = projection * view * ScatteredRockModel;
                Shaders.setMat4("mvpIn", mvp);
                Rock.Draw(Shaders);

            }
        }

        //Check for OpenGL errors
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cout << "OpenGL Error: " << err << endl;
        }

        //Refreshing
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete[] terrainVertices;
    delete[] terrainIndices;

    //Safely terminates GLFW
    glfwTerminate();

    return 0;
}