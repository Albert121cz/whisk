#include "shaders.hpp"


Shader::Shader(GraphicsManager* parent, const char* shaderFile,
    const GLenum type)
    : parentManager(parent), shaderType(type)
{
    initialized = false;
    std::string shaderString = openFile(shaderFile);

    if (shaderString.empty())
    {
        #ifdef DEBUG
            parentManager->sendToLog("Shader failed to load: " +
                std::string(shaderFile));
        #endif /* DEBUG */
        return;
    }

    #ifdef DEBUG
        parentManager->sendToLog("Shader loaded: " +
            std::string(shaderFile));
    #endif /* DEBUG */

    const char* shaderSource = shaderString.c_str();

    ID = glCreateShader(shaderType);

    initialized = true;

    // send shader code to OpenGL
    glShaderSource(ID, 1, &shaderSource, NULL);

    glCompileShader(ID);

    #ifdef DEBUG
        GLint compileStatus;
        glGetShaderiv(ID, GL_COMPILE_STATUS, &compileStatus);
        if (compileStatus != GL_TRUE)
        {
            GLsizei logLength = 0;
            GLchar message[1024];
            glGetShaderInfoLog(ID, 1024, &logLength, message);

            parentManager->sendToLog("Shader compilation failed: " + 
                std::string(shaderFile) + "\n" + std::string(message));
        }
        else
        {
            parentManager->sendToLog("Shader compiled: " +
                std::string(shaderFile));
        }
    #endif /* DEBUG */
}


Shader::~Shader()
{
    if (initialized)
        glDeleteShader(ID);
}


GLuint Shader::getID() const
{
    return ID;
}


bool Shader::getInitialized() const
{
    return initialized;
}


// https://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
std::string Shader::openFile(const char *filename)
{
    std::ifstream in(filename, std::ios::binary);

    if (!in)
        return "";

    std::string contents;

    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    
    return(contents);
}


ShaderManager::ShaderManager(GraphicsManager* parent) : parentManager(parent)
{
    ID = glCreateProgram();
}


ShaderManager::~ShaderManager()
{
    glDeleteProgram(ID);
}


void ShaderManager::addShader(const char* file)
{
    std::string fileString(file);
    std::smatch fileExtension;
    
    // https://cpprocks.com/files/c++11-regex-cheatsheet.pdf
    const std::regex fileExtensionRegex("\\..{4}$");

    std::regex_search(fileString, fileExtension, fileExtensionRegex);

    if (fileExtension[0] == ".vert")
    {
        vertexShaders.push_back(std::make_unique<Shader>
            (parentManager, file, GL_VERTEX_SHADER));
        if (!vertexShaders.back()->getInitialized())
            vertexShaders.pop_back();
    }

    else if (fileExtension[0] == ".frag")
    {
        fragmentShaders.push_back(std::make_unique<Shader>
            (parentManager, file, GL_FRAGMENT_SHADER));
        if (!fragmentShaders.back()->getInitialized())
            fragmentShaders.pop_back();
    }

    else
    {
        #ifdef DEBUG
            parentManager->sendToLog("Unsupported file extension: " +
                std::string(file));
        #endif /* DEBUG */
    }
}


bool ShaderManager::linkProgram()
{
    for(auto it = vertexShaders.begin(); it != vertexShaders.end(); it++)
        glAttachShader(ID, (*it)->getID());

    for(auto it = fragmentShaders.begin(); it != fragmentShaders.end(); it++)
        glAttachShader(ID, (*it)->getID());

    glLinkProgram(ID);

    GLint linkStatus;
    glGetProgramiv(ID, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE)
    {
        #ifdef DEBUG
            GLsizei logLength = 0;
            GLchar message[1024];
            glGetProgramInfoLog(ID, 1024, &logLength, message);

            parentManager->sendToLog("Shader program linking failed:\n" +
                std::string(message));
        #endif /* DEBUG */
        return false;
    }
    #ifdef DEBUG
        parentManager->sendToLog("Shader program linked");
    #endif /* DEBUG */
    return true;
}


void ShaderManager::useProgram()
{
    glUseProgram(ID);
}

GLuint ShaderManager::getID() const
{
    return ID;
}
