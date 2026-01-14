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
