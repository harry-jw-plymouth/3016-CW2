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
GLuint VAOs[NumVAOs];

//Buffer types
enum Buffer_IDs { ArrayBuffer, NumBuffers = 4 };
//Buffer objects
GLuint Buffers[NumBuffers];

//Shader program
GLuint program;


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

float Tile_Size = 0.0625f;

#define RENDER_DISTANCE 256 //Render width of map
#define MAP_SIZE RENDER_DISTANCE * RENDER_DISTANCE //Size of map in x & z space

//Amount of chunks across one dimension
const int squaresRow = RENDER_DISTANCE - 1;
//Two triangles per square to form a 1x1 chunk
const int trianglesPerSquare = 2;

//Amount of triangles on map
const int trianglesGrid = squaresRow * squaresRow * trianglesPerSquare;
GLfloat(*terrainVertices)[6] = new GLfloat[MAP_SIZE][6];
//Generation of height map indices - ALLOCATED ON HEAP TO AVOID STACK OVERFLOW
GLuint(*terrainIndices)[3] = new GLuint[trianglesGrid][3];

//flag for updatingwindow
bool UpdateNeeded = true;

//Tallest point of terrain 
vec3 TerrainTallestPointCoords;

//list of scattered trees coordinates 
const int  NumberOfTrees = 10;
vec3 TreesPositions[NumberOfTrees];
int IndexesToPlaceTrees[NumberOfTrees] = { 1,200,300,400,500,600,700,800,900,10 };

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    //Resizes window based on contemporary width & height values
    glViewport(0, 0, width, height);
}
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
    int CurrentX = (int)((drawingStartPosition - cameraPosition.x) / Tile_Size);
    int CurrentZ = (int)((drawingStartPosition - cameraPosition.z) / Tile_Size);
    return terrainVertices[CurrentZ * RENDER_DISTANCE + CurrentX][1];
    
}
void CheckForCollision() {
    float CurrentPosTerrainHeight = GetHeightOfTerrainAtCurrentPos();
    if (cameraPosition.y < CurrentPosTerrainHeight + 0.25) {
        cameraPosition.y = CurrentPosTerrainHeight + 0.25;
    }
}
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
    CheckForCollision();

   // cout << "camers pos: " << cameraPosition.x << ", " << cameraPosition.y << cameraPosition.z << "\n";
}

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

    int rowIndex = 0;
    for (int i = 0; i < MAP_SIZE; i++)
    {
        //Generation of x & z vertices for horizontal plane
        terrainVertices[i][0] = columnVerticesOffset;
      //  terrainVertices[i][1] = 0.0f;
        terrainVertices[i][2] = rowVerticesOffset;

        //Colour
        terrainVertices[i][3] = 9.0f;
        terrainVertices[i][4] = 121.75f;
        terrainVertices[i][5] = 105.25f;

        //Shifts x position across for next triangle along grid
        columnVerticesOffset = columnVerticesOffset + -Tile_Size;

        //Indexing of each chunk within row
        rowIndex++;
        //True when all triangles of the current row have been generated
        if (rowIndex == RENDER_DISTANCE)
        {
            //Resets for next row of triangles
            rowIndex = 0;
            //Resets x position for next row of triangles
            columnVerticesOffset = drawingStartPosition;
            //Shifts z position
            rowVerticesOffset = rowVerticesOffset + -Tile_Size;
        }
    }
    //Terrain vertice index
    int i = 0;

    //Highest point of terrain updated in loop
    float TallestTerrainPos = -200;
    int TallestPosIndex = 0;
    //Using x & y nested for loop in order to apply noise 2-dimensionally
    for (int y = 0; y < RENDER_DISTANCE; y++)
    {
        for (int x = 0; x < RENDER_DISTANCE; x++)
        {
            //Setting of height from 2D noise value at respective x & y coordinate
            //terrainVertices[i][1] = TerrainNoise.GetNoise((float)x, (float)y);
            float centerX = RENDER_DISTANCE * 0.5f;
            float centerY = RENDER_DISTANCE * 0.5f;
            float dx = x - centerX;
            float dy = y - centerY; 
            float dist = sqrt(dx * dx + dy * dy);
            float maxDist = sqrt(centerX * centerX + centerY * centerY);
            float falloff = 1.0f - (dist / maxDist); falloff = std::max(falloff, 0.0f); 
            float noise = TerrainNoise.GetNoise((float)x, (float)y); 
            terrainVertices[i][1] = falloff * 5.0f + noise * 0.5f;

            float biomeValue = BiomeNoise.GetNoise((float)x, (float)y);

            if (biomeValue <= -0.75f) //Plains
            {
                terrainVertices[i][3] = 0.45f;
                terrainVertices[i][4] = 0.75f;
                terrainVertices[i][5] = 0.35f;
            }
            else //Deeper forest
            {
                terrainVertices[i][3] = 0.10f;
                terrainVertices[i][4] = 0.45f;
                terrainVertices[i][5] = 0.20f;
            }
            //check to see if new height is taller than current tallest
            if (terrainVertices[i][1] > TallestTerrainPos) {
                TallestTerrainPos = terrainVertices[i][1];
                TallestPosIndex = i;
            }

            i++;

 
        }

    }
    TerrainTallestPointCoords.x = terrainVertices[TallestPosIndex][0];
    TerrainTallestPointCoords.y = terrainVertices[TallestPosIndex][1];
    TerrainTallestPointCoords.z = terrainVertices[TallestPosIndex][2];
    

    //Positions to start mapping indices from
    int columnIndicesOffset = 0;
    int rowIndicesOffset = 0;

    //Generation of map indices in the form of chunks (1x1 right angle triangle squares)
    rowIndex = 0;
    for (int i = 0; i < trianglesGrid - 1; i += 2)
    {
        terrainIndices[i][0] = columnIndicesOffset + rowIndicesOffset; //top left
        terrainIndices[i][1] = RENDER_DISTANCE + columnIndicesOffset + rowIndicesOffset; //bottom left
        terrainIndices[i][2] = 1 + columnIndicesOffset + rowIndicesOffset; //top right

        terrainIndices[i + 1][0] = 1 + columnIndicesOffset + rowIndicesOffset; //top right
        terrainIndices[i + 1][1] = RENDER_DISTANCE + columnIndicesOffset + rowIndicesOffset; //bottom left
        terrainIndices[i + 1][2] = 1 + RENDER_DISTANCE + columnIndicesOffset + rowIndicesOffset; //bottom right

        //Shifts x position across for next chunk along grid
        columnIndicesOffset = columnIndicesOffset + 1;

        //Indexing of each chunk within row
        rowIndex++;

        //True when all chunks of the current row have been generated
        if (rowIndex == squaresRow)
        {
            //Resets for next row of chunks
            rowIndex = 0;
            //Resets x position for next row of chunks
            columnIndicesOffset = 0;
            //Shifts z position
            rowIndicesOffset = rowIndicesOffset + RENDER_DISTANCE;
        }
    }

    //Sets index of VAO
    glGenVertexArrays(NumVAOs, VAOs);
    //Binds VAO to a buffer
    glBindVertexArray(VAOs[0]);
    //Sets indexes of all required buffer objects
    glGenBuffers(NumBuffers, Buffers);

    //Binds vertex object to array buffer
    glBindBuffer(GL_ARRAY_BUFFER, Buffers[Triangles]);
    //Allocates buffer memory for the vertices of the 'Triangles' buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * MAP_SIZE * 6, terrainVertices, GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(terrainVertices), terrainVertices, GL_STATIC_DRAW);

    //Binding & allocation for indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[Indices]);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(terrainIndices), terrainIndices, GL_STATIC_DRAW);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * trianglesGrid * 3, terrainIndices, GL_STATIC_DRAW);

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
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //Get Coordinates for trees to place around
    for (int i = 0; i < NumberOfTrees; i++) {
        TreesPositions[i].x = terrainVertices[IndexesToPlaceTrees[i]][0];
        TreesPositions[i].y = terrainVertices[IndexesToPlaceTrees[i]][1];
        TreesPositions[i].z = terrainVertices[IndexesToPlaceTrees[i]][2];
    }


}
int main()
{
   
    //Initialisation of GLFW
    glfwInit();

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
    Shader TerrainShader("shaders/TerrainVertexShader.vert", "shaders/TerrainFragmentShader.frag");
    Model Rock("media/rock/Rock07-Base.obj");
    Model Tree("media/Tree/GenTree-103_AE3D_03122023-F1.obj");
    Shaders.use();

    //Sets the viewport size within the window to match the window size of 1280x720
    glViewport(0, 0, 1280, 720);

    //Sets the framebuffer_size_callback() function as the callback for the window resizing event
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //Sets the mouse_callback() function as the callback for the mouse movement event
    glfwSetCursorPosCallback(window, Mouse_CallBack);

    // progress
    SetUpTerrain();

    //Model matrix
    mat4 model = mat4(1.0f);
    mat4 ScatteredModel = mat4(1.0f);
   
    //Looking straight forward
    model = rotate(model, radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
    ScatteredModel = rotate(ScatteredModel, radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
    //Elevation to look upon terrain
    model = translate(model, TerrainTallestPointCoords);

    //Model for terrain
    mat4 TerrainModel = mat4(1.0f);
    
    //Projection matrix
    mat4 projection = perspective(radians(45.0f), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);


    //Scaling to zoom in
    model = scale(model, vec3(0.025f, 0.025f, 0.025f));
    
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

        if (UpdateNeeded) {
            UpdateNeeded = false;

            //Rendering
            glClearColor(0.25f, 0.0f, 1.0f, 1.0f); //Colour to display on cleared window
            glClear(GL_COLOR_BUFFER_BIT); //Clears the colour buffer
            glClear(GL_DEPTH_BUFFER_BIT); //Might need

            glEnable(GL_CULL_FACE); //Discards all back-facing triangles

            //Transformations
            mat4 view = lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp); //Sets the position of the viewer, the movement direction in relation to it & the world up direction
            mat4 mvp = projection * view * TerrainModel;

            TerrainShader.use();
            TerrainShader.setMat4("mvpIn", mvp); //Setting of uniform with Shader class

            //Render terrain
            glBindVertexArray(VAOs[0]);
            glDrawElements(GL_TRIANGLES, trianglesGrid * 3, GL_UNSIGNED_INT, 0);

            //Drawing models
            Shaders.use();
            //draw main tree at tallest point
            mvp = projection * view * model;
            Shaders.setMat4("mvpIn", mvp);
            Tree.Draw(Shaders);

            //Draw scattered trees
            for (int i = 0; i < NumberOfTrees; i++) {
                mat4 ScatteredModel = mat4(1.0f);
                ScatteredModel = translate(ScatteredModel, TreesPositions[i]);
                ScatteredModel = scale(ScatteredModel, vec3(0.025f, 0.025f, 0.025f));

                mat4 mvp = projection * view * ScatteredModel;
                Shaders.setMat4("mvpIn", mvp);

                Tree.Draw(Shaders);
                
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