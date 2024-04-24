#include <array>
#include <iostream>
#include <fstream>

#include "shader.h"

namespace fs = std::filesystem;

/**
 * @brief Constructor for Shader class.
 *
 * @param vertex_shader_path Path to the vertex shader file.
 * @param fragment_shader_path Path to the fragment shader file.
 */
Shader::Shader( 
    const std::filesystem::path& vertex_shader_path, 
    const std::filesystem::path& fragment_shader_path)
{
    m_shader_id = glCreateProgram();

    if (m_shader_id == 0) {
        throw std::runtime_error("glCreateProgram() FAIL");
    }

    x_add_shader( vertex_shader_path, GL_VERTEX_SHADER);
    x_add_shader( fragment_shader_path, GL_FRAGMENT_SHADER);

    glLinkProgram(m_shader_id);

    GLint success{}; 
    glGetProgramiv(m_shader_id, GL_LINK_STATUS, &success);

    if (!success) {
        auto error_message = std::array<char, 512>{};
        glGetProgramInfoLog(m_shader_id, 512, nullptr, error_message.data());
        std::cout << "Error linking shader program: " << error_message.data() << "\n";
    }
}

/**
 * @brief Destructor for Shader class.
 */
Shader::~Shader()
{
    x_reset();
}

/**
 * @brief Reset function for Shader class.
 */
void Shader::x_reset()
{
    if (m_shader_id != 0) {
        glDeleteProgram(m_shader_id);
        m_shader_id = 0;
    }

    return;
}

/**
 * @brief Use shader function for Shader class.
 */
void Shader::use_shader()
{
    glUseProgram(m_shader_id);
}



/**
 * @brief Add shader function for Shader class.
 *
 * @param shader_path Path to the shader file.
 * @param shader_type Shader type (e.g., GL_VERTEX_SHADER).
 */
void Shader::x_add_shader( 
    const std::filesystem::path& shader_path,
    GLenum shader_type)
{
    auto shader_string = x_read_shader_file(shader_path);
    

    const auto code = std::array<const GLchar*, 1>{ shader_string.c_str() };
    const auto code_length = std::array<GLint, 1>{static_cast<GLint>(shader_string.size()) };

    const auto shader = glCreateShader(shader_type);

    glShaderSource(shader, 1, code.data(), code_length.data());
    glCompileShader(shader);

    GLint success{};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        auto error_message = std::array<char, 512>{};

        glGetShaderInfoLog(shader, 512, nullptr, error_message.data());
        std::cout << "Error compiling shader: " << error_message.data() << "\n";
        std::cout << "Shader location: " << shader_path << "\n";
    }

    glAttachShader(m_shader_id, shader);

    return;
}

/**
 * @brief Read shader file function for Shader class.
 *
 * @param file_path Path to the shader file.
 * @return The content of the shader file as a string.
 */
auto Shader::x_read_shader_file(
    const std::filesystem::path& file_path)-> std::string
{
    fs::path shader_path(file_path);

    if (!fs::exists(shader_path) || !fs::is_regular_file(shader_path)) { 
        throw std::runtime_error("Failed to locate shader file: " + shader_path.string());
        return "";
    }

    std::ifstream shader_file(file_path);

    if (!shader_file.is_open()) {
        //std::cout << "Failed to open shader file: " << file_path << "\n";
        throw std::runtime_error("Failed to open shader file: " + file_path.string());
        return ""; 
    }

    std::string code;
    std::string line;

    while (std::getline(shader_file, line)) {
        code += line + '\n';
    }

    shader_file.close();
    return code;
}


// Setters for various shader uniforms...

void Shader::set_int(const std::string& name, int value) const 
{
    glUniform1i( glGetUniformLocation(m_shader_id, name.c_str()), value);
}

void Shader::set_float(const std::string& name, float value) const
{
    glUniform1f( glGetUniformLocation(m_shader_id, name.c_str()), value);
}

void Shader::set_double(const std::string& name, double value) const
{
    glUniform1dv( glGetUniformLocation(m_shader_id, name.c_str()), 1, &value);
}
 
void Shader::set_dvec2(const std::string& name, const glm::dvec2& value) const
{
    glUniform2dv(glGetUniformLocation(m_shader_id, name.c_str()), 1, &value[0]);
}
 
void Shader::set_vec2(const std::string& name, const glm::vec2& value) const
{
    glUniform2fv(glGetUniformLocation(m_shader_id, name.c_str()), 1, &value[0]);
}

void Shader::set_vec2(const std::string& name, float x, float y) const
{
    glUniform2f(glGetUniformLocation(m_shader_id, name.c_str()), x, y);
}

 
void Shader::set_vec3(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(m_shader_id, name.c_str()), 1, &value[0]);
}

void Shader::set_vec3(const std::string& name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(m_shader_id, name.c_str()), x, y, z);
}

void Shader::set_vec4(const std::string& name, const glm::vec4& vec) const
{
    glUniform4f(glGetUniformLocation(m_shader_id, name.c_str()), vec.x, vec.y, vec.z, vec.w);
}

void Shader::set_mat4(const std::string& name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(m_shader_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

