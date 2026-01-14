# 3016-CW2

# Video Link 
# Gameplay description 
The gameplay is relatively simple, as the project is mostly focused on the scene itself rather than what you can do in the scene. However it does still contain some notable gameplay elements.
A key element of this was players ability to move around the scene using key inputs. These are done with a WASD control scheme, where w moves forwards, S moves backwards, A moves left and D moves right. This Control scheme was chosen as it is common in many games of this nature meaning it would be quick for players to pick up and understand 

# Dependencies used 
A variety of dependencies were used to complete this project, all repositories referenced are located in side the repository within the library folder within the project. Details about each dependency can be seen below 
## Open gl
Open gl was the core dependency the entire project was built around, basically the entire project would not be possible without the inclusion of the openGL library. 
In short, all the visuals come from OpenGL, and many of the other dependencies mentioned  use this one as a base(GLFW,GLAD). A key example of where this was absolutely vital to the code is seen in the render loop itself where the ‘glDawElements’ function was called. This function was used to draw the geometry of different objects in the scene and without it, essentially nothing of note would have been displayed. 

An example can be seen below
![Open GL Example](MDImages/OpenGLUseExample.png)
## GLM
GLM (Open gl mathematics) is a math library, which allowed for use of a variety of required mathematical features. While some of the features included with GLM could be coded manually without it (e.g 3 indiviual floats,or a length 3 array being used instead of a vec3), the entire scene would have been significantly harder to code, both in terms of the amount of code required and in terms of ease of understanding what the mathemtatics are doing.
Some key sections that GLM was used for were setting positions of models, setting the camera positions /moving the camera, and any other functions that require positioning/repositioning. Below is the code used to set the position of the central objects which is almost entirely made up of GLM originated code
![GLM Example](MDImages/GLMUseExample.png)
## ASSIMP/Learn Open GL
Assimp was used for a very simple but important reason, it enabled the program to retrieve/load a model from a file. Without this, it would not have been possible to load the models needed to display the intended scene. However, assimp cannot complete the full process for displaying a model, as it is unable to construct a model after it has been loaded. For this task the Learn Open Gl model construction libraries were used. It will take the loaded model data from the intended model file and construct it to be a model that can be displayed 
This can be seen in the code, when models are loaded, essentially ASSIMP is loading the obj file and then learning open gl is converting it to usable open gl data.
An example of this can be seen below where both models and shaders are loaded
![Assimp and Learn open gl Example](MDImages/ModelsAndShadersLoadExample.png)
## Glad 
While open GLs libraries are very effective, they cannot be used effectively without the help of a tool to help them be loaded. Glad is used for this purpose, as it manages the function pointers needed in OpenGL.
While GLAD is only seen once in the code (when it is initially loaded) it is actually used all throughout the code, as without it , the required function pointers would not be possible to load and thus the open gl library would be essentially useless 
## Fast Noise Lite
Fast noise lite was used to generate the terrain used in the scene. This was very important to the scene as without it, the scene would either be a set of floating models or a model for the terrain itself would need to be made. Terrain was set up by first specifying the type of noise the terrain should use. In this case perlin noise was used to allow for random terrain to be generated while still looking like terrain that could realistically exist. With the terrain noise specified, the terrain vertices are generated, aswell as setting any biome noise to allow for a varied look in the terrain. 
With terrain now set up it is now rendered alongside everything else in the render loop
Below is an example of this being used, where the nosie type is specified.
![Fast noise lite Example](MDImages/FNLUseExample.png)

# Game programming patterns
## Game Loop
The core of the scene is built using the game loop programming pattern, essentially having everything rendered repeatedly inside a while loop which continues until the scene is exited/closed. Every iteration of the loop, the program will check for any user inputs, in the case of this scene, it checks for W,A,S,D key clicks as mouse movement. 
When it detects a key input, the relevant changes are made to data in the scene, with WASD being used for movement and the mouse being used to change the direction of the camera. Then on the next frame/loop iteration, the scene being displayed will be re rendered to reflect the changes .
![Render loop](MDImages/RenderLoopCondition.png)
## Dirty flag
While the game loop method is very effective for drawing and rendering the scene, it does contain some drawbacks. Notably in regards to performance. This can cause an issue when the scene starts to get more complex, as rendering many objects every single frame will inevitably cause the scene to slow down. Therefore the game incorparetes a boolean flag, where expensice items in the scene are only rendered if something occurs to justify it (e.g a mouse movement or wasd click)
Below is the check done in the render loop and an example of where the flag is updated in the input detection function
![Dirty flag](MDImages/DirtyFlagPattern.png)
![Dirty flag catch](MDImages/DirtyFlagExample.png)
## Command pattern
The scene also contains a command pattern. In the game loop, 2 functions are called. One will check for any key inputs,and the other will check for mouse inputs. The code will check for what was inputted and adjust the game state accordingly, with WASD moving the position of the player and the mouse being used to move the direction the camera is looking
# Sample Screens
## Exception handling
To ensure the user was able to see as much of the scene as possible. Movement was implemented, which allowed both moving position and changing the way the camera is looking. 
To make sure the user did not simply go through the ground to an empty scene below, a check was added to make sure the camera's height position could not go lower than the terrain at the position the current user was at. Whenever a user would move the camera, the scene would check that their position was not low enough to be going through terrain and if they had in fact gone below the current position's terrain, the camera's Y positioning would be set to be above it.
![Height check](MDImages/TerrainHeightCheck.png)
This came with a key exception to handle as if the user went out of bounds of the terrain, the code would attempt to check the terrains current position, and when it couldnt find it, a crash would occur. Therefore a check had to be added

(Note: this function was created with the assistance of AI, for more details see the AI statement later in this file)
![Height get function](MDImages/GetHeightFunction.png) 
Essentially, the cameras x and y position were converted into what tile the user is currently in, and if it was more than the maximum or less than 0, a 0 value would be returned, preventing the code from causing an error when not within the bounds of the terrain
## Initialisation 
There were a few instances within the code where something had to be initialised before it could be used (e.g features from libraries or assets). In these cases, a check was done to ensure nothing went wrong during this initialisation that would cause an error. For these cases, a check was done, and if it failed then the code would exit cleanly. If it did not exit cleanly and instead tried to continue, a full on crash would occur instead. 

Below are a few examples 
GLFW window initilasation: 
![GLFW initialisation](MDImages/GLFWInit.png) 
GLad initialisation: 
![Glad initialisation](MDImages/GladInit.png) 
Texture asset loading:
![texture initialisation](MDImages/TextureInit.png) 

# Extra things of note 
## Signature
Placed within the scenel,there are 2 objects,one being a H and one being a W. These act as a signature for myself, being my initials, for more details on these objects, please see the set up section below
![Initials objects](MDImages/Initials.png) 
## Setting up objects 
As mentioned in the above signature section, within the scene there are 2 objects, one being a H and one being a W. Unlile the trees and other objects seen around the scene, these were created by hard coding their vertices and indices in to the desired shapes.
To assist with the setup of these objects, rough diagrams were created. These diagrams were not completely final, but were helpful for visualising the coordinates needed for the indices/vertices.
A few changes did occur between the rough images and the final version, for exampple the H shaped object had some positioning changed around, thinning down the side parts. But overall these were still very important to the initial visualisation of positioning and drawing using the indices
These diagrams can be seen below or within the repository
### Vertices diagrams 
![Vertices Set up](MDImages/VerticesSetup.png) 
### Indices diagrams
![Indices Set up](MDImages/IndicesSetup.png) 




