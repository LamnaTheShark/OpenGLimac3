#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>

#include <cmath>

#include "glew/glew.h"

#include "GLFW/glfw3.h"
#include "stb/stb_image.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include "glm/glm.hpp"
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4, glm::ivec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "glm/gtc/type_ptr.hpp" // glm::value_ptr

//#pragma comment(lib, \"assimp.lib\")

#include <assimp/Importer.hpp> 
#include <assimp/scene.h> 
#include <assimp/postprocess.h> 

using namespace std ;
#include <vector>


#ifndef DEBUG_PRINT
#define DEBUG_PRINT 1
#endif

#if DEBUG_PRINT == 0
#define debug_print(FORMAT, ...) ((void)0)
#else
#ifdef _MSC_VER
#define debug_print(FORMAT, ...) \
    fprintf(stderr, "%s() in %s, line %i: " FORMAT "\n", \
        __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#else
#define debug_print(FORMAT, ...) \
    fprintf(stderr, "%s() in %s, line %i: " FORMAT "\n", \
        __func__, __FILE__, __LINE__, __VA_ARGS__)
#endif
#endif

// Font buffers
extern const unsigned char DroidSans_ttf[];
extern const unsigned int DroidSans_ttf_len;    

// Shader utils
int check_link_error(GLuint program);
int check_compile_error(GLuint shader, const char ** sourceBuffer);
int check_link_error(GLuint program);
GLuint compile_shader(GLenum shaderType, const char * sourceBuffer, int bufferSize);
GLuint compile_shader_from_file(GLenum shaderType, const char * fileName);

// OpenGL utils
bool checkError(const char* title);


class ShaderProgram{
public:
    //Shader shaderParams;
    GLchar* Vertpath;
    GLchar* Fragpath;
    //GLuint shaderProg;

    GLuint vertShaderId; //= compile_shader_from_file(GL_VERTEX_SHADER, "shader/classic/heart.vert");
    GLuint fragShaderId; // = compile_shader_from_file(GL_FRAGMENT_SHADER, "shader/classic/heart.frag");
    GLuint progName; // = glCreateProgram();

    GLuint mvpLocation; // = glGetUniformLocation(programObjectCube, "projection");
    GLuint mvLocation; // = glGetUniformLocation(programObjectCube, "view");
    //GLuint skyboxLocation; // = glGetUniformLocation(programObjectCube, "skybox");

    //mvpLocation = glGetUniformLocation(classicHeart, "MVP");
    //mvLocation = glGetUniformLocation(classicHeart, "MV");
    GLuint timeLocation; // = glGetUniformLocation(classicHeart, "Time");
    GLuint diffuseLocation; // = glGetUniformLocation(classicHeart, "Diffuse");
    GLuint specLocation; // = glGetUniformLocation(classicHeart, "Specular");
    GLuint lightLocation; // = glGetUniformLocation(classicHeart, "Light");
    //GLuint reflectMapLocation; // = glGetUniformLocation(classicHeart, "ReflectMap");
    GLuint specularPowerLocation; // = glGetUniformLocation(classicHeart, "SpecularPower");
    GLuint instanceCountLocation; // = glGetUniformLocation(classicHeart, "InstanceCount");

    ShaderProgram(GLchar* Vertpath, GLchar* Fragpath)//, Shader shaderParams) //, GLuint shaderProg)
    {
        //this->shaderParams = shaderParams;
        this->Vertpath = Vertpath;
        this->Fragpath = Fragpath;
        //this->shaderProg = shaderProg;
        //Shader shaderParams;

        this->createShader();
        //shaderProg = this->progName;
        cout << "LOADED::SHADER:: " << this->progName << endl;
    }

private:
    void createShader(){

        this->vertShaderId = compile_shader_from_file(GL_VERTEX_SHADER, this->Vertpath);
        this->fragShaderId = compile_shader_from_file(GL_FRAGMENT_SHADER, this->Fragpath);
        this->progName = glCreateProgram();

        glAttachShader(this->progName, this->vertShaderId);
        glAttachShader(this->progName, this->fragShaderId);
        glLinkProgram(this->progName);
        
            if (check_link_error(this->progName) < 0){
                exit(1);
            }
                

        this->mvpLocation = glGetUniformLocation(this->progName, "projection");
        this->mvLocation = glGetUniformLocation(this->progName, "view");
        //GLuint skyboxLocation = glGetUniformLocation(programObjectCube, "skybox");

        this->mvpLocation = glGetUniformLocation(this->progName, "MVP");
        this->mvLocation = glGetUniformLocation(this->progName, "MV");
        this->timeLocation = glGetUniformLocation(this->progName, "Time");
        this->diffuseLocation = glGetUniformLocation(this->progName, "Diffuse");
        this->specLocation = glGetUniformLocation(this->progName, "Specular");
        this->lightLocation = glGetUniformLocation(this->progName, "Light");
        //GLuint reflectMapLocation = glGetUniformLocation(classicHeart, "ReflectMap");
        this->specularPowerLocation = glGetUniformLocation(this->progName, "SpecularPower");
        this->instanceCountLocation = glGetUniformLocation(this->progName, "InstanceCount");
           if (!checkError("Uniforms ")){
                exit(1);
            }
                
    }
};

//ASSIMP Mesh & Model implementation
//Derived from online tutorial
//Modified for current use & optimisation
struct Vertex {
    // Position
    glm::vec3 Position;
    // Normal
    glm::vec3 Normal;
    // TexCoords
    glm::vec2 TexCoords;
};

struct Texture {
    GLuint id;
    string type;
    aiString path;
};

class Mesh {
public:
    /*  Mesh Data  */
    vector<Vertex> vertices;
    vector<GLuint> indices;
    //vector<Texture> textures;

    /*  Functions  */
    // Constructor
    Mesh(vector<Vertex> vertices, vector<GLuint> indices) //, vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        //this->textures = textures;

        // Now that we have all the required data, set the vertex buffers and its attribute pointers.
        this->setupMesh();
    }

    void DrawModel(){ //Fonction de dessin d'un mesh

        glBindVertexArray(this->VAO);
        glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, (void*)0);
    }



private:
    /*  Render data  */
    GLuint VAO, VBO, EBO, VBO2, VBO3, VBO4;

    /*  Functions    */
    // Initializes all the buffer objects/arrays
    void setupMesh()
    {
        // Create buffers/arrays
        
        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &this->VBO);
        glGenBuffers(1, &this->EBO);

        glBindVertexArray(this->VAO);
        // Load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);  

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

        // Set the vertex attribute pointers
        // Vertex Positions
        glEnableVertexAttribArray(0);   
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
        // Vertex Normals
        glEnableVertexAttribArray(1);   
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));
        // Vertex Texture Coords
        glEnableVertexAttribArray(2);   
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);
        

    }
};

class Model 
{
public:
    Model(GLchar* path)
    {
        this->loadModel(path);
    }

    void DrawAll(){
        for(GLuint i = 0; i < this->meshes.size(); i++){
            //cout << "ERROR::ASSIMP:: " << this->meshes.size() << endl;
            this->meshes[i].DrawModel();
        }
    }
    
private:
    /*  Model Data  */
    vector<Mesh> meshes;
    string directory;

    /*  Functions   */
    // Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string path)
    {
        // Read file via ASSIMP
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
        // Check for errors
        if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // Retrieve the directory path of the filepath
        this->directory = path.substr(0, path.find_last_of('/'));

        // Process ASSIMP's root node recursively
        this->processNode(scene->mRootNode, scene);
    }

    // Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene)
    {
        // Process each mesh located at the current node
        for(GLuint i = 0; i < node->mNumMeshes; i++)
        {
            // The node object only contains indices to index the actual objects in the scene. 
            // The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; 
            this->meshes.push_back(this->processMesh(mesh, scene));         
        }
        // After we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(GLuint i = 0; i < node->mNumChildren; i++)
        {
            this->processNode(node->mChildren[i], scene);
        }

    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        // Data to fill
        vector<Vertex> vertices;
        vector<GLuint> indices;
        //vector<Texture> textures;

        // Walk through each of the mesh's vertices
        for(GLuint i = 0; i < mesh->mNumVertices; i++)
        {
            
            Vertex vertex;
            glm::vec3 vector; 
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // Normals
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
            // Texture Coordinates
            if(mesh->mTextureCoords[0]) 
            {
                glm::vec2 vec;
               
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            vertices.push_back(vertex);
        }

        for(GLuint i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];

            for(GLuint j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        
        // Return a mesh object created from the extracted mesh data

        return Mesh(vertices, indices);
    }


};

GLuint loadCubemap(vector<const GLchar*> faces)
{ //Create a CubeMap
    GLuint textureID;
    glGenTextures(1, &textureID);
    glActiveTexture(GL_TEXTURE0);
    int x;
    int y;
    int comp;

    int width,height;
    unsigned char* image;
    
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    for(GLuint i = 0; i < faces.size(); i++)
    {
        image = stbi_load(faces[i], &width, &height, 0, 3);
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
            GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
        );
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);


    fprintf(stderr, "Cubemap %dx%d:%d\n", x, y, comp);
    checkError("Cubemap Texture Initialization");

    return textureID;
}


void drawSpecific(Model mesh, ShaderProgram shader, glm::mat4 transform, glm::mat4 mv, glm::mat4 mvp, double t, int instanceCount, float specularPower){
    //Draw a specific Mesh using a chosen Shader
    glUseProgram(shader.progName);

    glProgramUniformMatrix4fv(shader.progName, shader.mvpLocation, 1, 0, glm::value_ptr(transform));

    glProgramUniformMatrix4fv(shader.progName, shader.mvLocation, 1, 0, glm::value_ptr(mv));
    glProgramUniform1i(shader.progName, shader.instanceCountLocation, (int) instanceCount);
    glProgramUniform1f(shader.progName, shader.specularPowerLocation, specularPower); //30.0f
    glProgramUniform1f(shader.progName, shader.timeLocation, t);

    mesh.DrawAll();

}

void drawVao(GLuint vao, ShaderProgram shader, glm::mat4 transform, glm::mat4 mv, glm::mat4 mvp, double t, int instanceCount, float specularPower, GLuint triCount){
    //Draw a specific VAO using a chosen Shader
    glUseProgram(shader.progName);

    glProgramUniform1i(shader.progName, shader.diffuseLocation, 2);
    glProgramUniform1i(shader.progName, shader.specLocation, 3);

    glProgramUniformMatrix4fv(shader.progName, shader.mvpLocation, 1, 0, glm::value_ptr(transform));

    glProgramUniformMatrix4fv(shader.progName, shader.mvLocation, 1, 0, glm::value_ptr(mv));
    glProgramUniform1i(shader.progName, shader.instanceCountLocation, (int) instanceCount);
    glProgramUniform1f(shader.progName, shader.specularPowerLocation, specularPower); //30.0f
    glProgramUniform1f(shader.progName, shader.timeLocation, t);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, triCount, GL_UNSIGNED_INT, (void*)0);
}




GLuint generateAttachmentTexture(GLboolean depth, GLboolean stencil, int width, int height)
{ //auto creation of FrameBuffer Texture

    GLenum attachment_type;
    if(!depth && !stencil)
        attachment_type = GL_RGB;
    else if(depth && !stencil)
        attachment_type = GL_DEPTH_COMPONENT;
    else if(!depth && stencil)
        attachment_type = GL_STENCIL_INDEX;

  
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    if(!depth && !stencil)
        glTexImage2D(GL_TEXTURE_2D, 0, attachment_type, width, height, 0, attachment_type, GL_UNSIGNED_BYTE, NULL);
    else 
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}




struct Camera
{
    float radius;
    float theta;
    float phi;
    glm::vec3 o;
    glm::vec3 eye;
    glm::vec3 up;
};
void camera_defaults(Camera & c);
void camera_zoom(Camera & c, float factor);
void camera_turn(Camera & c, float phi, float theta);
void camera_pan(Camera & c, float x, float y);

struct GUIStates
{
    bool panLock;
    bool turnLock;
    bool zoomLock;
    int lockPositionX;
    int lockPositionY;
    int camera;
    double time;
    bool playing;
    static const float MOUSE_PAN_SPEED;
    static const float MOUSE_ZOOM_SPEED;
    static const float MOUSE_TURN_SPEED;
};
const float GUIStates::MOUSE_PAN_SPEED = 0.001f;
const float GUIStates::MOUSE_ZOOM_SPEED = 0.05f;
const float GUIStates::MOUSE_TURN_SPEED = 0.005f;
void init_gui_states(GUIStates & guiStates);




int main( int argc, char **argv )
{
    

    int width = 1280, height= 720; 
    float widthf = (float) width, heightf = (float) height;
    double t;
    float fps = 0.f;

    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        exit( EXIT_FAILURE );
    }
    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
    glfwWindowHint(GLFW_DECORATED, GL_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    int const DPI = 2; // For retina screens only
#else
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    int const DPI = 1;
# endif

    // Open a window and create its OpenGL context
    GLFWwindow * window = glfwCreateWindow(width/DPI, height/DPI, "aogl", 0, 0);
    if( ! window )
    {
        fprintf( stderr, "Failed to open GLFW window\n" );
        glfwTerminate();
        exit( EXIT_FAILURE );
    }
    glfwMakeContextCurrent(window);

    // Init glew
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
          /* Problem: glewInit failed, something is seriously wrong. */
          fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
          exit( EXIT_FAILURE );
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode( window, GLFW_STICKY_KEYS, GL_TRUE );

    // Enable vertical sync (on cards that support it)
    glfwSwapInterval( 1 );
    GLenum glerr = GL_NO_ERROR;
    glerr = glGetError();

    ImGui_ImplGlfwGL3_Init(window, true);

    // Init viewer structures
    Camera camera;
    camera_defaults(camera);
    GUIStates guiStates;
    init_gui_states(guiStates);
    int instanceCount = 1;
    int pointLightCount = 10;
    int directionalLightCount = 1;
    int spotLightCount = 0;
    float speed = 0.5;

    // Load images and upload textures
    GLuint textures[6];
    glGenTextures(6, textures);
    int x;
    int y;
    int comp;

    unsigned char * diffuse = stbi_load("textures/pattern1/diffuse.tga", &x, &y, &comp, 3);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, diffuse);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    fprintf(stderr, "Diffuse %dx%d:%d\n", x, y, comp);

    unsigned char * spec = stbi_load("textures/pattern1/specular.tga", &x, &y, &comp, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, x, y, 0, GL_RED, GL_UNSIGNED_BYTE, spec);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    fprintf(stderr, "Spec %dx%d:%d\n", x, y, comp);


    unsigned char * diffFloor = stbi_load("textures/pattern2/diffuse.tga", &x, &y, &comp, 3);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, diffFloor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    fprintf(stderr, "diffFloor %dx%d:%d\n", x, y, comp);

    unsigned char * specFloor = stbi_load("textures/pattern2/specular.tga", &x, &y, &comp, 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, textures[3]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, specFloor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    fprintf(stderr, "SpecFloor %dx%d:%d\n", x, y, comp);

    unsigned char * alphaFloor = stbi_load("textures/pattern2/height.tga", &x, &y, &comp, 3);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, textures[4]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, alphaFloor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    fprintf(stderr, "alphaFloor %dx%d:%d\n", x, y, comp);

    unsigned char * reflectMap = stbi_load("textures/map.jpg", &x, &y, &comp, 3);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, textures[5]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, reflectMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    fprintf(stderr, "reflectMap %dx%d:%d\n", x, y, comp);
    checkError("Texture Initialization");

    vector<const GLchar*> faces;
    faces.push_back("textures/skybox/right.jpg");
    faces.push_back("textures/skybox/left.jpg");
    faces.push_back("textures/skybox/top.jpg");
    faces.push_back("textures/skybox/bottom.jpg");
    faces.push_back("textures/skybox/back.jpg");
    faces.push_back("textures/skybox/front.jpg");
    GLuint cubemapTexture = loadCubemap(faces);  //Create CubeMap

        // Try to load and compile cubemap shaders
    //Kepts as "classic" Shader despite advanced Shader Creation Functions, because of the CubeMap Uniform Variable
    GLuint vertShaderIdCube = compile_shader_from_file(GL_VERTEX_SHADER, "shaders/cube/cube.vert");
    GLuint fragShaderIdCube = compile_shader_from_file(GL_FRAGMENT_SHADER, "shaders/cube/cube.frag");
    GLuint programObjectCube = glCreateProgram();
    glAttachShader(programObjectCube, vertShaderIdCube);
    glAttachShader(programObjectCube, fragShaderIdCube);
    glLinkProgram(programObjectCube);
    if (check_link_error(programObjectCube) < 0)
        exit(1);

    GLuint mvpLocation = glGetUniformLocation(programObjectCube, "projection");
    GLuint mvLocation = glGetUniformLocation(programObjectCube, "view");
    GLuint skyboxLocation = glGetUniformLocation(programObjectCube, "skybox");



    ShaderProgram classic("shaders/classic/heart.vert","shaders/classic/heart.frag");
    ShaderProgram classicGlow("shaders/classic/glow.vert","shaders/classic/glow.frag");
    ShaderProgram mirror("shaders/mirror/heart.vert","shaders/mirror/heart.frag");
    ShaderProgram mirrorGlow("shaders/mirror/glow.vert","shaders/mirror/glow.frag");
    ShaderProgram reflexS("shaders/reflect/reflex.vert","shaders/reflect/reflex.frag");
    //Shaders Creations

    // Load geometry
    int cube_triangleCount = 12;
    int cube_triangleList[] = {0, 1, 2, 2, 1, 3, 4, 5, 6, 6, 5, 7, 8, 9, 10, 10, 9, 11, 12, 13, 14, 14, 13, 15, 16, 17, 18, 19, 17, 20, 21, 22, 23, 24, 25, 26, };
    float cube_uvs[] = {0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f,  1.f, 0.f,  1.f, 1.f,  0.f, 1.f,  1.f, 1.f,  0.f, 0.f, 0.f, 0.f, 1.f, 1.f,  1.f, 0.f,  };
    float cube_vertices[] = {-0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5 };
    float cube_normals[] = {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, };
    int plane_triangleCount = 2;
    int plane_triangleList[] = {0, 1, 2, 2, 1, 3}; 
    float plane_uvs[] = {0.f, 0.f, 0.f, 50.f, 50.f, 0.f, 50.f, 50.f};
    float plane_vertices[] = {-50.0, -1.0, 50.0, 50.0, -1.0, 50.0, -50.0, -1.0, -50.0, 50.0, -1.0, -50.0};
    float plane_normals[] = {0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0};

    //blit quads
    int   quad_triangleCount = 2;
    int   quad_triangleList[] = {0, 1, 2, 2, 1, 3}; 
    float quad_vertices[] =  {-1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0};



    // Vertex Array Object
    GLuint vao[3];
    glGenVertexArrays(3, vao);

    // Vertex Buffer Objects
    GLuint vbo[10];
    glGenBuffers(10, vbo);

    // Cube
    glBindVertexArray(vao[0]);
    // Bind indices and upload data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_triangleList), cube_triangleList, GL_STATIC_DRAW);
    // Bind vertices and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
    // Bind normals and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_normals), cube_normals, GL_STATIC_DRAW);
    // Bind uv coords and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_uvs), cube_uvs, GL_STATIC_DRAW);

    // Plane
    glBindVertexArray(vao[1]);
    // Bind indices and upload data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[4]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(plane_triangleList), plane_triangleList, GL_STATIC_DRAW);
    // Bind vertices and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), plane_vertices, GL_STATIC_DRAW);
    // Bind normals and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_normals), plane_normals, GL_STATIC_DRAW);
    // Bind uv coords and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_uvs), plane_uvs, GL_STATIC_DRAW);

       GLfloat skyboxVertices[] = {
        // Positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
  
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
  
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
   
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
  
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
  
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

        // Quad
    //GLuint quad_vao;
    // Quad
    glBindVertexArray(vao[2]);
    // Bind indices and upload data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[8]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_triangleList), quad_triangleList, GL_STATIC_DRAW);
    // Bind vertices and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[9]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    // Setup skybox VAO
    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);

    //ASSIMP OPEN


    Model plot("mesh/plot.obj");
    Model heart("mesh/heart.obj");
    Model glow("mesh/glow.obj");
    Model out("mesh/out.obj");

    // Generate Shader Storage Objects
    GLuint ssbo[6];
    glGenBuffers(6, ssbo);


    //Targeting implementation of reflection blur

    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);  
    // Create a color attachment texture
    GLuint textureColorbuffer = generateAttachmentTexture(false, false, width, height);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    // Create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // Use a single renderbuffer object for both a depth AND stencil buffer.
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // Now actually attach it
    // Now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /*ShaderProgram screen("shaders/screen.vert", "shaders/screen.frag");
    GLuint screenTexture = glGetUniformLocation(screen.progName, "screenTexture");
    glProgramUniform1i(screen.progName, screenTexture, textureColorbuffer);
    */
    ShaderProgram blit("shaders/blit.vert", "shaders/blit.frag");
    GLuint blitTexture = glGetUniformLocation(blit.progName, "Texture");
    glProgramUniform1i(blit.progName, blitTexture, textureColorbuffer);
    

// _______________________________________________________END OF BLUR
    checkError("Buffer Init");

    // Viewport 
    glViewport( 0, 0, width, height  );

    do
    {
        t = glfwGetTime() * speed;

        // Mouse states
        int leftButton = glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_LEFT );
        int rightButton = glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT );
        int middleButton = glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_MIDDLE );

        if( leftButton == GLFW_PRESS )
            guiStates.turnLock = true;
        else
            guiStates.turnLock = false;

        if( rightButton == GLFW_PRESS )
            guiStates.zoomLock = true;
        else
            guiStates.zoomLock = false;

        if( middleButton == GLFW_PRESS )
            guiStates.panLock = true;
        else
            guiStates.panLock = false;

        // Camera movements
        int altPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
        if (!altPressed && (leftButton == GLFW_PRESS || rightButton == GLFW_PRESS || middleButton == GLFW_PRESS))
        {
            double x; double y;
            glfwGetCursorPos(window, &x, &y);
            guiStates.lockPositionX = x;
            guiStates.lockPositionY = y;
        }
        if (altPressed == GLFW_PRESS)
        {
            double mousex; double mousey;
            glfwGetCursorPos(window, &mousex, &mousey);
            int diffLockPositionX = mousex - guiStates.lockPositionX;
            int diffLockPositionY = mousey - guiStates.lockPositionY;
            if (guiStates.zoomLock)
            {
                float zoomDir = 0.0;
                if (diffLockPositionX > 0)
                    zoomDir = -1.f;
                else if (diffLockPositionX < 0 )
                    zoomDir = 1.f;
                camera_zoom(camera, zoomDir * GUIStates::MOUSE_ZOOM_SPEED);
            }
            else if (guiStates.turnLock)
            {
                camera_turn(camera, diffLockPositionY * GUIStates::MOUSE_TURN_SPEED,
                            diffLockPositionX * GUIStates::MOUSE_TURN_SPEED);

            }
            else if (guiStates.panLock)
            {
                camera_pan(camera, diffLockPositionX * GUIStates::MOUSE_PAN_SPEED,
                            diffLockPositionY * GUIStates::MOUSE_PAN_SPEED);
            }
            guiStates.lockPositionX = mousex;
            guiStates.lockPositionY = mousey;
        }





        // Default states
        glEnable(GL_DEPTH_TEST);

        // Clear the front buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 

        // Get camera matrices
        glm::mat4 projection = glm::perspective(45.0f, widthf / heightf, 0.1f, 100.f); 
        glm::mat4 worldToView = glm::lookAt(camera.eye, camera.o, camera.up);
        glm::mat4 objectToWorld;
        glm::mat4 mv = worldToView * objectToWorld;
        glm::mat4 mvp = projection * mv;


        //ATTEMPTING SKYBOX

 /*       glDepthMask(GL_FALSE);
        glUseProgram(programObjectCube);     

        glm::mat4 Cubeview = worldToView ; 
        glm::mat4 Cubeprojection = glm::perspective(0.01f, (float)(widthf/heightf), 1.1f, 1000.0f);
         //glm::translate(mvp, glm::vec3 (0.0f, 0.0f, 0.0f) );
        //CubeMatrix = glm::perspective(1.0f, (float)(1280/720), 0.1f, 100.0f);
        // glm::translate(mvp, glm::vec3 (0.0f, 4.0f, 0.0f) );
        //CubeMatrix = glm::scale(CubeMatrix, glm::vec3 (100.0f, 100.0f, 100.0f) );
        //CubeMatrix = glm::rotate(CubeMatrix, 3.11415f/2.0f, glm::vec3 (1.0f, 0.0f, 0.0f) );
        //discMatrix = glm::rotate(discMatrix, 3.11415f/2.0f, glm::vec3 (0.0f, 1.0f, 0.0f) );
        //glm::rotate(Model, angle_in_degrees, glm::vec3(x, y, z)); // where x, y, z is axis of rotation (e.g. 0 1 0)
        //glProgramUniformMatrix4fv(programObjectCube, mvpLocation, 1, 0, glm::value_ptr(CubeMatrix));

        glUniformMatrix4fv(glGetUniformLocation(programObjectCube, "view"), 1, GL_FALSE, glm::value_ptr(Cubeview));
        glUniformMatrix4fv(glGetUniformLocation(programObjectCube, "projection"), 1, GL_FALSE, glm::value_ptr(Cubeprojection));
        // skybox cube
 
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(programObjectCube, "skybox"), 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

        glDrawArrays(GL_TRIANGLES, 0, 36); 
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);

        //END SKYBOX ATTEMPT
*/


        struct PointLight
        {
            glm::vec3 position;
            int padding;
            glm::vec3 color;
            float intensity;
        };

        struct DirectionalLight
        {
            glm::vec3 position;
            int padding;
            glm::vec3 color;
            float intensity;
        };

        struct SpotLight
        {
            glm::vec3 position;
            float angle;
            glm::vec3 direction;
            float penumbraAngle;
            glm::vec3 color;
            float intensity;
        };


        int pointLightBufferSize = sizeof(PointLight) * pointLightCount + sizeof(int) * 4;
        int directionalLightBufferSize = sizeof(DirectionalLight) * directionalLightCount + sizeof(int) * 4;
        int spotLightBufferSize = sizeof(SpotLight) * spotLightCount + sizeof(int) * 4;


        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[0]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, pointLightBufferSize, 0, GL_DYNAMIC_COPY);
        void * lightBuffer = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
        ((int*) lightBuffer)[0] = pointLightCount;
        for (int i = 0; i < pointLightCount; ++i) {
            PointLight p = { glm::vec3( worldToView * glm::vec4((pointLightCount*cosf(t*i)) , 1.0, (pointLightCount*sinf(t*i)) , 1.0)), 0,  
                             glm::vec3(1.0f, 1.0f, 1.0f),  
                             4.0f};
            ((PointLight*) ((int*) lightBuffer + 4))[i] = p;
        }
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[1]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, directionalLightBufferSize, 0, GL_DYNAMIC_COPY);
        lightBuffer = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
        ((int*) lightBuffer)[0] = directionalLightCount;
        for (int i = 0; i < directionalLightCount; ++i) {
            DirectionalLight directionalLight = { glm::vec3( worldToView * glm::vec4(-1.0, -1.0, 0.0, 0.0)), 0,  
                                                  glm::vec3(1.0f, 1.0f, 1.0f),  
                                                  0.3f};
            ((DirectionalLight*) ((int*) lightBuffer + 4))[i] = directionalLight;
        }
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[2]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, spotLightBufferSize, 0, GL_DYNAMIC_COPY);
        lightBuffer = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
        ((int*) lightBuffer)[0] = spotLightCount;
        for (int i = 0; i < spotLightCount; ++i)  {
            SpotLight spotLight = { glm::vec3( worldToView * glm::vec4((spotLightCount*sinf(t))  * cosf(t*i), 1.f + sinf(t * i), fabsf(spotLightCount*cosf(t)) * sinf(t*i), 1.0)), 45.f + 20.f * cos(t + i), 
                                    glm::vec3( worldToView * glm::vec4(sinf(t*10.0+i), -1.0, 0.0, 0.0)), 60.f + 20.f * cos(t + i),  
                                    glm::vec3(fabsf(cos(t+i*2.f)), 1.-fabsf(sinf(t+i)) , 0.5f + 0.5f-fabsf(cosf(t+i))),  1.0};
            ((SpotLight*) ((int*) lightBuffer + 4))[i] = spotLight;
        }
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


       

        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, ssbo[0], 0, pointLightBufferSize);
        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, ssbo[1], 0, directionalLightBufferSize);
        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, ssbo[2], 0, spotLightBufferSize);



        //Generating mirror lights

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[3]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, pointLightBufferSize, 0, GL_DYNAMIC_COPY);
        lightBuffer = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
        ((int*) lightBuffer)[0] = pointLightCount;
        for (int i = 0; i < pointLightCount; ++i) {
            PointLight p = { glm::vec3( worldToView * glm::vec4((pointLightCount*cosf(t*i)) , -1.0, (pointLightCount*sinf(t*i)) , 1.0)), 0,  
                             glm::vec3(1.0f, 1.0f, 1.0f),  
                             4.0f};
            ((PointLight*) ((int*) lightBuffer + 4))[i] = p;
        }
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[4]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, directionalLightBufferSize, 0, GL_DYNAMIC_COPY);
        lightBuffer = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
        ((int*) lightBuffer)[0] = directionalLightCount;
        for (int i = 0; i < directionalLightCount; ++i) {
            DirectionalLight directionalLight = { glm::vec3( worldToView * glm::vec4(1.0, 1.0, 0.0, 0.0)), 0,  
                                                  glm::vec3(1.0f, 1.0f, 1.0f),  
                                                  0.3f};
            ((DirectionalLight*) ((int*) lightBuffer + 4))[i] = directionalLight;
        }
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[5]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, spotLightBufferSize, 0, GL_DYNAMIC_COPY);
        lightBuffer = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
        ((int*) lightBuffer)[0] = spotLightCount;
        for (int i = 0; i < spotLightCount; ++i)  {
            SpotLight spotLight = { glm::vec3( worldToView * glm::vec4((spotLightCount*sinf(t))  * cosf(t*i), 1.f + sinf(t * i), fabsf(spotLightCount*cosf(t)) * sinf(t*i), 1.0)), 45.f + 20.f * cos(t + i), 
                                    glm::vec3( worldToView * glm::vec4(sinf(t*10.0+i), -1.0, 0.0, 0.0)), 60.f + 20.f * cos(t + i),  
                                    glm::vec3(fabsf(cos(t+i*2.f)), 1.-fabsf(sinf(t+i)) , 0.5f + 0.5f-fabsf(cosf(t+i))),  1.0};
            ((SpotLight*) ((int*) lightBuffer + 4))[i] = spotLight;
        }
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


       

        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 3, ssbo[3], 0, pointLightBufferSize);
        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 4, ssbo[4], 0, directionalLightBufferSize);
        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 5, ssbo[5], 0, spotLightBufferSize);

        

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glm::mat4 discMatrix;


        //________________________________Rendering Mirror Disc

        //glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT); 
        glEnable(GL_DEPTH_TEST);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures[1]);

        //glBindFramebuffer(GL_FRAMEBUFFER, 0);

        discMatrix = glm::translate(mvp, glm::vec3 (0.0f, -6.0f, 0.0f) );
        discMatrix = glm::scale(discMatrix, glm::vec3 (0.01f, -0.01f, 0.01f) );
        discMatrix = glm::rotate(discMatrix, 3.11415f/2.0f, glm::vec3 (1.0f, 0.0f, 0.0f) );
        //Translations, rotations and scale for the disc

        drawSpecific(plot, mirror, discMatrix, mv, mvp, t, instanceCount, 30.0f);
        drawSpecific(heart, mirror, discMatrix, mv, mvp, t, instanceCount, 30.0f);
        drawSpecific(glow, mirrorGlow, discMatrix, mv, mvp, t, instanceCount, 30.0f);
        drawSpecific(out, mirrorGlow, discMatrix, mv, mvp, t, instanceCount, 30.0f);

        glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
        //glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
        //glClear(GL_COLOR_BUFFER_BIT);
        //glDisable(GL_DEPTH_TEST);
 

        //_______________________End Rendering Mirror Disc


        


        //screenShader.Use(); 
        /*glUseProgram(blit.progName); 
        
        glProgramUniformMatrix4fv(blit.progName, blit.mvLocation, 1, 0, glm::value_ptr(mv));
        glProgramUniformMatrix4fv(blit.progName, blit.mvpLocation, 1, 0, glm::value_ptr(mvp));
        //glProgramUniform1i(blit.progName, blitTexture, textureColorbuffer);
        glBindVertexArray(vao[2]);
        glDisable(GL_DEPTH_TEST);
        //glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);

        glViewport( 0, 0, width/2, height/2  );

        glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);

        glBindVertexArray(0); 

        glViewport( 0, 0, width, height );
        glEnable(GL_DEPTH_TEST);

        glUseProgram(classic.progName);
        glProgramUniform1i(classic.progName, classic.diffuseLocation, textureColorbuffer);

        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glBindVertexArray(vao[1]);
        glDrawElementsInstanced(GL_TRIANGLES, cube_triangleCount * 3, GL_UNSIGNED_INT, (void*)0, (int) instanceCount);
*/

        //________________________Rendering Floor

        // Select textures
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textures[2]);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, textures[3]);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, textures[4]);

        drawVao(vao[1], reflexS, mvp, mv, mvp, 0.0f, instanceCount, 30.0f, plane_triangleCount * 3);

        //________________________________Rendering Disc
        //  Select Second Textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures[1]);

        discMatrix = glm::translate(mvp, glm::vec3 (0.0f, 4.0f, 0.0f) );
        discMatrix = glm::scale(discMatrix, glm::vec3 (0.01f, 0.01f, 0.01f) );
        discMatrix = glm::rotate(discMatrix, 3.11415f/2.0f, glm::vec3 (1.0f, 0.0f, 0.0f) );
        glProgramUniformMatrix4fv(classic.progName, classic.mvpLocation, 1, 0, glm::value_ptr(discMatrix));


        drawSpecific(plot, classic, discMatrix, mv, mvp, t, instanceCount, 30.0f);
        drawSpecific(heart, classic, discMatrix, mv, mvp, t, instanceCount, 30.0f);
        drawSpecific(glow, classicGlow, discMatrix, mv, mvp, t, instanceCount, 30.0f);
        drawSpecific(out, classicGlow, discMatrix, mv, mvp, t, instanceCount, 30.0f);

        //_______________________End Rendering Disc

 //_______________________________________________________________________________________________________________________________________________________________________________________



        // Draw UI
        ImGui_ImplGlfwGL3_NewFrame();
        ImGui::SetNextWindowSize(ImVec2(200,100), ImGuiSetCond_FirstUseEver);
        ImGui::Begin("aogl");
        ImGui::SliderFloat("Speed", &speed, 0.01, 1.0);
        //ImGui::SliderInt("Point Lights", &pointLightCount, 0, 1024);
        //ImGui::SliderInt("Directional Lights", &directionalLightCount, 0, 1024);
        //ImGui::SliderInt("Spot Lights", &spotLightCount, 0, 1024);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
        
        ImGui::Render();
        // Check for errors
        checkError("End loop");

        glfwSwapBuffers(window);
        glfwPollEvents();

        double newTime = glfwGetTime();
        fps = 1.f/ (newTime - t);
    } // Check if the ESC key was pressed
    while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GLFW_PRESS );

    // Close OpenGL window and terminate GLFW
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();

    exit( EXIT_SUCCESS );
}

// No windows implementation of strsep
char * strsep_custom(char **stringp, const char *delim)
{
    register char *s;
    register const char *spanp;
    register int c, sc;
    char *tok;
    if ((s = *stringp) == NULL)
        return (NULL);
    for (tok = s; ; ) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                *stringp = s;
                return (tok);
            }
        } while (sc != 0);
    }
    return 0;
}

int check_compile_error(GLuint shader, const char ** sourceBuffer)
{
    // Get error log size and print it eventually
    int logLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 1)
    {
        char * log = new char[logLength];
        glGetShaderInfoLog(shader, logLength, &logLength, log);
        char *token, *string;
        string = strdup(sourceBuffer[0]);
        int lc = 0;
        while ((token = strsep_custom(&string, "\n")) != NULL) {
           printf("%3d : %s\n", lc, token);
           ++lc;
        }
        fprintf(stderr, "Compile : %s", log);
        delete[] log;
    }
    // If an error happend quit
    int status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
        return -1;     
    return 0;
}

int check_link_error(GLuint program)
{
    // Get link error log size and print it eventually
    int logLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 1)
    {
        char * log = new char[logLength];
        glGetProgramInfoLog(program, logLength, &logLength, log);
        fprintf(stderr, "Link : %s \n", log);
        delete[] log;
    }
    int status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);        
    if (status == GL_FALSE)
        return -1;
    return 0;
}

GLuint compile_shader(GLenum shaderType, const char * sourceBuffer, int bufferSize)
{
    GLuint shaderObject = glCreateShader(shaderType);
    const char * sc[1] = { sourceBuffer };
    glShaderSource(shaderObject, 
                   1, 
                   sc,
                   NULL);
    glCompileShader(shaderObject);
    check_compile_error(shaderObject, sc);
    return shaderObject;
}

GLuint compile_shader_from_file(GLenum shaderType, const char * path)
{
    FILE * shaderFileDesc = fopen( path, "rb" );
    if (!shaderFileDesc)
        return 0;
    fseek ( shaderFileDesc , 0 , SEEK_END );
    long fileSize = ftell ( shaderFileDesc );
    rewind ( shaderFileDesc );
    char * buffer = new char[fileSize + 1];
    fread( buffer, 1, fileSize, shaderFileDesc );
    buffer[fileSize] = '\0';
    GLuint shaderObject = compile_shader(shaderType, buffer, fileSize );
    delete[] buffer;
    return shaderObject;
}


bool checkError(const char* title)
{
    int error;
    if((error = glGetError()) != GL_NO_ERROR)
    {
        std::string errorString;
        switch(error)
        {
        case GL_INVALID_ENUM:
            errorString = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            errorString = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            errorString = "GL_INVALID_OPERATION";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            errorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            errorString = "GL_OUT_OF_MEMORY";
            break;
        default:
            errorString = "UNKNOWN";
            break;
        }
        fprintf(stdout, "OpenGL Error(%s): %s\n", errorString.c_str(), title);
    }
    return error == GL_NO_ERROR;
}

void camera_compute(Camera & c)
{
    c.eye.x = cos(c.theta) * sin(c.phi) * c.radius + c.o.x;   
    c.eye.y = cos(c.phi) * c.radius + c.o.y ;
    c.eye.z = sin(c.theta) * sin(c.phi) * c.radius + c.o.z;   
    c.up = glm::vec3(0.f, c.phi < M_PI ?1.f:-1.f, 0.f);
}

void camera_defaults(Camera & c)
{
    c.phi = 3.14/2.f;
    c.theta = 3.14/2.f;
    c.radius = 10.f;
    camera_compute(c);
}

void camera_zoom(Camera & c, float factor)
{
    c.radius += factor * c.radius ;
    if (c.radius < 0.1)
    {
        c.radius = 10.f;
        c.o = c.eye + glm::normalize(c.o - c.eye) * c.radius;
    }
    camera_compute(c);
}

void camera_turn(Camera & c, float phi, float theta)
{
    c.theta += 1.f * theta;
    c.phi   -= 1.f * phi;
    if (c.phi >= (2 * M_PI) - 0.1 )
        c.phi = 0.00001;
    else if (c.phi <= 0 )
        c.phi = 2 * M_PI - 0.1;
    camera_compute(c);
}

void camera_pan(Camera & c, float x, float y)
{
    glm::vec3 up(0.f, c.phi < M_PI ?1.f:-1.f, 0.f);
    glm::vec3 fwd = glm::normalize(c.o - c.eye);
    glm::vec3 side = glm::normalize(glm::cross(fwd, up));
    c.up = glm::normalize(glm::cross(side, fwd));
    c.o[0] += up[0] * y * c.radius * 2;
    c.o[1] += up[1] * y * c.radius * 2;
    c.o[2] += up[2] * y * c.radius * 2;
    c.o[0] -= side[0] * x * c.radius * 2;
    c.o[1] -= side[1] * x * c.radius * 2;
    c.o[2] -= side[2] * x * c.radius * 2;       
    camera_compute(c);
}

void init_gui_states(GUIStates & guiStates)
{
    guiStates.panLock = false;
    guiStates.turnLock = false;
    guiStates.zoomLock = false;
    guiStates.lockPositionX = 0;
    guiStates.lockPositionY = 0;
    guiStates.camera = 0;
    guiStates.time = 0.0;
    guiStates.playing = false;
}

