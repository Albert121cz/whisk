#include "shaders.hpp"


Shader::Shader(GraphicsManager* parent, 
            const char* shaderFile, const GLenum type)
            : shaderType(type), parentManager(parent)
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
        parentManager->oglErrorCheck(SHADER_CREATE);
    #endif /* DEBUG */
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