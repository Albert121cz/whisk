#include "shaders.hpp"


Shader::Shader(const char* shaderFile, const GLenum type) : shaderType(type)
{
    initialized = false;
    std::string shaderString = openFile(shaderFile);

    if (shaderString.empty())
    {
        #ifdef DEBUG
            std::cout << "Shader failed to load: " << shaderFile << std::endl;
        #endif /* DEBUG */
        return;
    }

    #ifdef DEBUG
        std::cout << "Shader loaded: " << shaderFile << std::endl;
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

            std::cout << "Shader compilation failed: " << shaderFile << "\n"
                << message << std::endl;
        }
        else
        {
            std::cout << "Shader compiled: " << shaderFile << std::endl;
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


// the whole function is sourced from:
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


ShaderManager::ShaderManager()
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
    
    // used to make the regular expression
    // https://cpprocks.com/files/c++11-regex-cheatsheet.pdf
    const std::regex fileExtensionRegex("\\..{4}$");

    // regular expression is used to recognize shader type from the extension
    std::regex_search(fileString, fileExtension, fileExtensionRegex);

    if (fileExtension[0] == ".vert")
    {
        vertexShaders.push_back(std::make_unique<Shader>
            (file, GL_VERTEX_SHADER));
        if (!vertexShaders.back()->getInitialized())
            vertexShaders.pop_back();
    }

    else if (fileExtension[0] == ".frag")
    {
        fragmentShaders.push_back(std::make_unique<Shader>
            (file, GL_FRAGMENT_SHADER));
        if (!fragmentShaders.back()->getInitialized())
            fragmentShaders.pop_back();
    }

    else
    {
        #ifdef DEBUG
            std::cout << "Unsupported file extension: " << file << std::endl;
        #endif /* DEBUG */
    }
}


bool ShaderManager::linkProgram()
{
    for(auto it = vertexShaders.begin(); it != vertexShaders.end(); it++)
        glAttachShader(ID, (*it)->getID());

    for(auto it = fragmentShaders.begin(); it != fragmentShaders.end(); it++)
        glAttachShader(ID, (*it)->getID());

    // all attached shaders are linked together into a shader program
    glLinkProgram(ID);

    GLint linkStatus;
    glGetProgramiv(ID, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE)
    {
        #ifdef DEBUG
            GLsizei logLength = 0;
            GLchar message[1024];
            glGetProgramInfoLog(ID, 1024, &logLength, message);

            std::cout << "Shader program linking failed:\n" << message
                << std::endl;
        #endif /* DEBUG */
        return false;
    }
    #ifdef DEBUG
        std::cout << "Shader program linked" << std::endl;
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
