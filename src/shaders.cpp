#include "shaders.hpp"


Shader::Shader(GraphicsManager* parent, 
            const char* shaderFile, const int type)
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

    const char* shaderSource = shaderString.c_str();

    ID = glCreateShader(shaderType);

    initialized = true;

    // second parameter sets how many strings we want to use
    // last parameter sets the length of the strings - NULL is set if
    // the strings are \0 terminated
    glShaderSource(ID, 1, &shaderSource, NULL);

    glCompileShader(ID);
    parentManager->oglErrorCheck(SHADER_CREATE);
}


// https://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
std::string Shader::openFile(const char *filename)
{
    std::ifstream in(filename, std::ios::in | std::ios::binary);
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