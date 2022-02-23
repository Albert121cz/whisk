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
            std::ostringstream messageStream;
            messageStream << "Shader failed to load: " << shaderFile;
            parentManager->sendToLog(messageStream.str());
        #endif /* DEBUG */
        return;
    }
    else
    {
        #ifdef DEBUG
            std::ostringstream messageStream;
            messageStream << "Shader loaded: " << shaderFile;
            parentManager->sendToLog(messageStream.str());
        #endif /* DEBUG */
    }

    const char* shaderSource = shaderString.c_str();

    ID = glCreateShader(shaderType);

    initialized = true;

    // second parameter sets how many strings we want to use
    // last parameter sets the length of the strings - NULL is set if
    // the strings are \0 terminated
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

            std::ostringstream messageStream;
            messageStream << "Shader compilation failed: " << shaderFile 
                            << "\n" << message;
            parentManager->sendToLog(messageStream.str());
        }
        else
        {
            std::ostringstream messageStream;
            messageStream << "Shader compiled: " << shaderFile;
            parentManager->sendToLog(messageStream.str());
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
    if (in)
    {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return(contents);
    }
    return {};
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
    std::smatch extension;
    
    // https://cpprocks.com/files/c++11-regex-cheatsheet.pdf
    const std::regex extensionRegex("\\..{4}$");

    std::regex_search(fileString, extension, extensionRegex);

    if (extension[0] == ".vert")
    {
        vertexShaders.push_back(std::make_unique<Shader>
            (parentManager, file, GL_VERTEX_SHADER));
        if (!vertexShaders.back()->getInitialized())
            vertexShaders.pop_back();
    }

    else if (extension[0] == ".frag")
    {
        fragmentShaders.push_back(std::make_unique<Shader>
            (parentManager, file, GL_FRAGMENT_SHADER));
        if (!fragmentShaders.back()->getInitialized())
            fragmentShaders.pop_back();
    }

    else
    {
        #ifdef DEBUG
            std::ostringstream messageStream;
            messageStream << "Unsupported file extension: " << file;
            parentManager->sendToLog(messageStream.str());
        #endif /* DEBUG */
    }
}


void ShaderManager::linkProgram()
{
    for(auto it = vertexShaders.begin(); it != vertexShaders.end(); it++)
        glAttachShader(ID, (*it)->getID());

    for(auto it = fragmentShaders.begin(); it != fragmentShaders.end(); it++)
        glAttachShader(ID, (*it)->getID());

    glLinkProgram(ID);

    #ifdef DEBUG
        GLint linkStatus;
        glGetProgramiv(ID, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE)
        {
            GLsizei logLength = 0;
            GLchar message[1024];
            glGetProgramInfoLog(ID, 1024, &logLength, message);

            std::ostringstream messageStream;
            messageStream << "Shader program linking failed:\n" << message;
            parentManager->sendToLog(messageStream.str());
        }
        else
        {
            parentManager->sendToLog(std::string("Shader program linked"));
        }
    #endif /* DEBUG */
}


void ShaderManager::useProgram()
{
    glUseProgram(ID);
}

GLuint ShaderManager::getID() const
{
    return ID;
}
